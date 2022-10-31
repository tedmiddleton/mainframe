//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_group_h
#define INCLUDED_mainframe_group_h

#include <unordered_map>

#include "mainframe/frame.hpp"
#include "mainframe/detail/group.hpp"
#include "mainframe/detail/frame_indexer.hpp"

namespace mf
{

namespace agg
{
template<size_t Ind>
detail::sum_op<Ind>
sum(columnindex<Ind>)
{
    return detail::sum_op<Ind>{};
}

template<size_t Ind>
detail::min_op<Ind>
min(columnindex<Ind>)
{
    return detail::min_op<Ind>{};
}

template<size_t Ind>
detail::max_op<Ind>
max(columnindex<Ind>)
{
    return detail::max_op<Ind>{};
}

template<size_t Ind>
detail::mean_op<Ind>
mean(columnindex<Ind>)
{
    return detail::mean_op<Ind>{};
}

template<size_t Ind>
detail::stddev_op<Ind>
stddev(columnindex<Ind>)
{
    return detail::stddev_op<Ind>{};
}

detail::count_op inline count()
{
    return detail::count_op{};
}

} // namespace agg

///
/// Intermediate class for GROUP BY aggregate operations
///
/// group is a container for a class frame along with a column index. It
/// can be created explicitly, but it makes more sense to create one with
/// frame::group_by().
///
template<size_t... GroupInds, typename... Ts>
class group<index_defn<GroupInds...>, Ts...>
    : public frame_indexer<index_defn<GroupInds...>, Ts...>
{
    // The index definition for this group
    using group_index_defn = index_defn<GroupInds...>;

    // The index frame is a frame made of the indexed columns
    using get_index_frame = detail::get_index_frame<group_index_defn, frame<Ts...>>;
    using index_frame     = typename get_index_frame::type;

    // The result frame for any given function invocation has the index columns
    // but also the grouped function columns
    template<typename IndexDefn>
    using get_result_frame = detail::get_result_frame<group_index_defn, IndexDefn, frame<Ts...>>;
    template<typename IndexDefn>
    using result_frame = typename get_result_frame<IndexDefn>::type;

    using key_type       = typename index_frame::const_value_type; // _row_proxy
    using map_value_type = std::vector<size_t>;
    using map_type       = std::unordered_map<key_type, map_value_type>;

    template<typename... Ops>
    using get_aggregate_frame = detail::get_aggregate_frame<frame<Ts...>, group_index_defn, Ops...>;

public:
    group(frame<Ts...> f)
        : frame_indexer<index_defn<GroupInds...>, Ts...>(f)
    {}

    template<typename... Ops>
    typename get_aggregate_frame<Ops...>::type
    aggregate(Ops...) const
    {
        this->build_index();

        typename detail::get_result_columns_from_args<frame<Ts...>, Ops...>::type result_columns;
        rename_result_columns_args<0, Ops...>(result_columns);
        auto ifr = get_index_frame::op(this->m_frame);
        ifr.clear(); // this is mostly to get the column names

        for (const auto& [key, rowinds] : this->m_idx) {

            // key is a _row_proxy into the original frame
            // rowinds is a vector<size_t> of row indices into the original frame

            // now we iterate the frame columns, performing all operations
            aggregate_column<0, Ops...>(rowinds, result_columns);
            aggregate_count<Ops...>(rowinds, result_columns);
            ifr.push_back(key);
        }

        auto result = add_result_series<0>(ifr, result_columns);
        return result;
    }

private:
    template<size_t ColInd>
    std::string
    get_op_name(detail::sum_op<ColInd>) const
    {
        return "sum";
    }

    template<size_t ColInd>
    std::string
    get_op_name(detail::min_op<ColInd>) const
    {
        return "min";
    }

    template<size_t ColInd>
    std::string
    get_op_name(detail::max_op<ColInd>) const
    {
        return "max";
    }

    template<size_t ColInd>
    std::string
    get_op_name(detail::mean_op<ColInd>) const
    {
        return "mean";
    }

    template<size_t ColInd>
    std::string
    get_op_name(detail::stddev_op<ColInd>) const
    {
        return "stddev";
    }

    template<size_t ArgInd, typename... Ops, typename... Us>
    void
    rename_result_columns_args(std::tuple<series<Us>...>& result_columns) const
    {
        rename_result_columns_args_cols<0, ArgInd, Ops...>(result_columns);
        rename_result_columns_args_count<ArgInd, Ops...>(result_columns);
        if constexpr (ArgInd + 1 < sizeof...(Ops)) {
            rename_result_columns_args<ArgInd + 1, Ops...>(result_columns);
        }
    }

    template<size_t ArgInd, typename... Ops, typename... Us>
    void
    rename_result_columns_args_count(std::tuple<series<Us>...>& result_columns) const
    {
        using Op = typename detail::pack_element<ArgInd, Ops...>::type;
        if constexpr (std::is_same<Op, detail::count_op>::value) {
            auto& s = std::get<ArgInd>(result_columns);
            s.set_name("count(*)");
        }
    }

    template<size_t ColInd, size_t ArgInd, typename... Ops, typename... Us>
    void
    rename_result_columns_args_cols(std::tuple<series<Us>...>& result_columns) const
    {
        using OpsTup = std::tuple<Ops...>;
        if constexpr (detail::is_colind_at_argind<ColInd, ArgInd, OpsTup>::value) {
            columnindex<ColInd> ci;
            auto name = this->m_frame.column_name(ci);
            auto& s   = std::get<ArgInd>(result_columns);
            using Op  = typename detail::pack_element<ArgInd, Ops...>::type;
            Op op;
            std::stringstream ss;
            ss << get_op_name(op) << "( " << name << " )";
            s.set_name(ss.str());
        }
        if constexpr (ColInd + 1 < sizeof...(Ts)) {
            rename_result_columns_args_cols<ColInd + 1, ArgInd, Ops...>(result_columns);
        }
    }

    template<typename... Ops, typename... Us>
    void
    aggregate_count(
        const std::vector<size_t>& rowinds, std::tuple<series<Us>...>& result_columns) const
    {
        aggregate_count_arg<0, Ops...>(rowinds, result_columns);
    }

    template<size_t ArgInd, typename... Ops, typename... Us>
    void
    aggregate_count_arg(
        const std::vector<size_t>& rowinds, std::tuple<series<Us>...>& result_columns) const
    {
        using Op = typename detail::pack_element<ArgInd, Ops...>::type;
        if constexpr (std::is_same<Op, detail::count_op>::value) {
            auto& s = std::get<ArgInd>(result_columns);
            s.push_back(rowinds.size());
        }
        if constexpr (ArgInd + 1 < sizeof...(Ops)) {
            aggregate_count_arg<ArgInd + 1, Ops...>(rowinds, result_columns);
        }
    }

    template<size_t ColInd, typename... Ops, typename... Us>
    void
    aggregate_column(
        const std::vector<size_t>& rowinds, std::tuple<series<Us>...>& result_columns) const
    {
        aggregate_column_arg<ColInd, 0, Ops...>(rowinds, result_columns);
        if constexpr (ColInd + 1 < sizeof...(Ts)) {
            aggregate_column<ColInd + 1, Ops...>(rowinds, result_columns);
        }
    }

    template<size_t ColInd, size_t ArgInd, typename... Ops, typename... Us>
    void
    aggregate_column_arg(
        const std::vector<size_t>& rowinds, std::tuple<series<Us>...>& result_columns) const
    {
        (void)result_columns;
        using OpsTup = std::tuple<Ops...>;
        if constexpr (detail::is_colind_at_argind<ColInd, ArgInd, OpsTup>::value) {

            using Op = typename detail::pack_element<ArgInd, Ops...>::type;
            Op op;
            auto& result_column = std::get<ArgInd>(result_columns);
            aggregate_column_arg_op(op, rowinds, result_column);
        }
        if constexpr (ArgInd + 1 < sizeof...(Us)) {
            aggregate_column_arg<ColInd, ArgInd + 1, Ops...>(rowinds, result_columns);
        }
    }

    template<size_t ColInd, typename T>
    void
    aggregate_column_arg_op(
        detail::sum_op<ColInd>, const std::vector<size_t>& rowinds, series<T>& result_column) const
    {
        T temp = static_cast<T>(0);
        columnindex<ColInd> ci;
        for (size_t rowind : rowinds) {
            const auto& row = this->m_frame.row(rowind);
            const T& t      = row.at(ci);
            temp += t;
        }
        result_column.push_back(temp);
    }

    template<size_t ColInd, typename T>
    void
    aggregate_column_arg_op(
        detail::max_op<ColInd>, const std::vector<size_t>& rowinds, series<T>& result_column) const
    {
        using std::max;
        using std::numeric_limits;
        T temp = numeric_limits<T>::lowest();
        columnindex<ColInd> ci;
        for (size_t rowind : rowinds) {
            const auto& row = this->m_frame.row(rowind);
            const T& t      = row.at(ci);
            temp            = max(temp, t);
        }
        result_column.push_back(temp);
    }

    template<size_t ColInd, typename T>
    void
    aggregate_column_arg_op(
        detail::min_op<ColInd>, const std::vector<size_t>& rowinds, series<T>& result_column) const
    {
        using std::min;
        using std::numeric_limits;
        T temp = numeric_limits<T>::max();
        columnindex<ColInd> ci;
        for (size_t rowind : rowinds) {
            const auto& row = this->m_frame.row(rowind);
            const T& t      = row.at(ci);
            temp            = min(temp, t);
        }
        result_column.push_back(temp);
    }

    template<size_t ColInd, typename T>
    void
    aggregate_column_arg_op(
        detail::mean_op<ColInd>, const std::vector<size_t>& rowinds, series<T>& result_column) const
    {
        T temp = static_cast<T>(0);
        columnindex<ColInd> ci;
        for (size_t rowind : rowinds) {
            const auto& row = this->m_frame.row(rowind);
            const T& t      = row.at(ci);
            temp += t;
        }
        temp /= rowinds.size();
        result_column.push_back(temp);
    }

    template<size_t ColInd, typename T>
    void
    aggregate_column_arg_op(detail::stddev_op<ColInd>, const std::vector<size_t>& rowinds,
        series<T>& result_column) const
    {
        T sum = static_cast<T>(0);
        columnindex<ColInd> ci;
        for (size_t rowind : rowinds) {
            const auto& row = this->m_frame.row(rowind);
            const T& t      = row.at(ci);
            sum += t;
        }
        T mean = sum / rowinds.size();

        T sqdist = static_cast<T>(0);
        for (size_t rowind : rowinds) {
            const auto& row = this->m_frame.row(rowind);
            const T& t      = row.at(ci);
            T dist          = t - mean;
            sqdist += (dist * dist);
        }

        T stddev = static_cast<T>(sqrt(sqdist / rowinds.size()));
        result_column.push_back(stddev);
    }

    template<size_t Ind, typename... Us>
    uframe
    add_result_series(
        typename get_index_frame::type ifr, std::tuple<series<Us>...>& result_columns) const
    {
        uframe ufr = ifr;
        std::vector<useries> us;
        vectorize_result_columns<0>(result_columns, us);
        for (auto s : us) {
            ufr.add_column(s);
        }
        return ufr;
    }

    template<size_t Ind, typename... Us>
    void
    vectorize_result_columns(
        std::tuple<series<Us>...>& result_columns, std::vector<useries>& us) const
    {
        us.push_back(std::get<Ind>(result_columns));
        if constexpr (Ind + 1 < sizeof...(Us)) {
            vectorize_result_columns<Ind + 1>(result_columns, us);
        }
    }
};


} // namespace mf

#endif // INCLUDED_mainframe_group_h
