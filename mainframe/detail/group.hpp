//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_detail_group_h
#define INCLUDED_mainframe_detail_group_h

#include <map>

#include "mainframe/frame.hpp"
#include "mainframe/detail/frame_indexer.hpp"

namespace mf
{

template<typename IndexDefn, typename... Ts>
class group;

}

namespace mf::detail
{

// A frame derived from another frame and two index definitions.
//   result_frame<
//     index_defn<4, 0>,
//     index_defn<2, 3>,
//     frame<short, double, float, int, year_month>>::type
//   == frame<year_month, short, float, int>
// This is used to execute grouped functions like max() - the functions specify columns,
// which is what the second index_defn is for. The first index_defn is for the
// indexed/group'd-by columns. So for example, if we had
//   frame<year, month, double, int, float, bool> f;
//   auto gf = f.groupby( _1, _5 ); // month and bool
//   auto mx = gf.max( _2, _4 );
//  ...inside the max() function we will need
//   auto rf = result_frame<index_defn<1, 5>, index_defn<2, 4>, frame<Ts...>>::get( m_frame );
// and rf will be of type
//  frame<month, bool, double, float>
template<typename IndexDefn1, typename IndexDefn2, typename Frame>
struct get_result_frame;

template<size_t... Inds1, size_t... Inds2, typename Frame>
struct get_result_frame<index_defn<Inds1...>, index_defn<Inds2...>, Frame>
{
    using indexed_cols1 = typename get_index_frame<index_defn<Inds1...>, Frame>::type;
    using indexed_cols2 = typename get_index_frame<index_defn<Inds2...>, Frame>::type;
    using type          = typename combine_frames<indexed_cols1, indexed_cols2>::type;

    static type
    op(Frame& f)
    {
        return f.columns(columnindex<Inds1>{}..., columnindex<Inds2>{}...);
    }

    static const type
    op(const Frame& f)
    {
        return f.columns(columnindex<Inds1>{}..., columnindex<Inds2>{}...);
    }
};

template<size_t Ind>
struct sum_op : std::integral_constant<size_t, Ind>
{};

template<size_t Ind>
struct min_op : std::integral_constant<size_t, Ind>
{};

template<size_t Ind>
struct max_op : std::integral_constant<size_t, Ind>
{};

template<size_t Ind>
struct mean_op : std::integral_constant<size_t, Ind>
{};

template<size_t Ind>
struct stddev_op : std::integral_constant<size_t, Ind>
{};

struct count_op
{};

template<typename Frame, typename... Ops>
struct get_result_columns_from_args;

template<typename... Ts, template<size_t> typename Op, size_t Ind, typename... Ops>
struct get_result_columns_from_args<frame<Ts...>, Op<Ind>, Ops...>
{
    using ind_type          = typename detail::pack_element<Ind, Ts...>::type;
    using remaining_columns = typename get_result_columns_from_args<frame<Ts...>, Ops...>::type;
    using type              = typename detail::prepend<series<ind_type>, remaining_columns>::type;
};

template<typename... Ts, typename... Ops>
struct get_result_columns_from_args<frame<Ts...>, count_op, Ops...>
{
    using remaining_columns = typename get_result_columns_from_args<frame<Ts...>, Ops...>::type;
    using type              = typename detail::prepend<series<size_t>, remaining_columns>::type;
};

template<typename... Ts, template<size_t> typename Op, size_t Ind>
struct get_result_columns_from_args<frame<Ts...>, Op<Ind>>
{
    using ind_type = typename detail::pack_element<Ind, Ts...>::type;
    using type     = std::tuple<series<ind_type>>;
};

template<typename... Ts>
struct get_result_columns_from_args<frame<Ts...>, count_op>
{
    using type = std::tuple<series<size_t>>;
};

template<typename Frame, typename... Ops>
struct get_frame_from_args;

template<typename... Ts, template<size_t> typename Op, size_t Ind, typename... Ops>
struct get_frame_from_args<frame<Ts...>, Op<Ind>, Ops...>
{
    using ind_type             = typename detail::pack_element<Ind, Ts...>::type;
    using remaining_frame_type = typename get_frame_from_args<frame<Ts...>, Ops...>::type;
    using type                 = typename detail::prepend<ind_type, remaining_frame_type>::type;
};

template<typename... Ts, typename... Ops>
struct get_frame_from_args<frame<Ts...>, count_op, Ops...>
{
    using remaining_frame_type = typename get_frame_from_args<frame<Ts...>, Ops...>::type;
    using type                 = typename detail::prepend<size_t, remaining_frame_type>::type;
};

template<typename... Ts, template<size_t> typename Op, size_t Ind>
struct get_frame_from_args<frame<Ts...>, Op<Ind>>
{
    using ind_type = typename detail::pack_element<Ind, Ts...>::type;
    using type     = frame<ind_type>;
};

template<typename... Ts>
struct get_frame_from_args<frame<Ts...>, count_op>
{
    using type = frame<size_t>;
};

template<size_t QColInd, size_t ArgInd, typename OpsTpl>
struct is_colind_at_argind;

template<size_t QColInd, size_t ArgInd, typename Op, typename... Ops>
struct is_colind_at_argind<QColInd, ArgInd, std::tuple<Op, Ops...>>
{
    static const bool value = is_colind_at_argind<QColInd, ArgInd - 1, std::tuple<Ops...>>::value;
};

template<size_t QColInd, template<size_t> typename Op, size_t ColInd, typename... Ops>
struct is_colind_at_argind<QColInd, 0, std::tuple<Op<ColInd>, Ops...>> : std::false_type
{};

template<size_t QColInd, template<size_t> typename Op, typename... Ops>
struct is_colind_at_argind<QColInd, 0, std::tuple<Op<QColInd>, Ops...>> : std::true_type
{};

template<size_t QColInd, typename... Ops>
struct is_colind_at_argind<QColInd, 0, std::tuple<count_op, Ops...>> : std::false_type
{};

template<typename Frame, typename IndexDefn, typename... Ops>
struct get_aggregate_frame;

template<typename... Ts, size_t... Inds, typename... Ops>
struct get_aggregate_frame<frame<Ts...>, index_defn<Inds...>, Ops...>
{
    using index_frame = typename get_index_frame<index_defn<Inds...>, frame<Ts...>>::type;
    using args_frame  = typename get_frame_from_args<frame<Ts...>, Ops...>::type;
    using type        = typename detail::combine_frames<index_frame, args_frame>::type;
};

} // namespace mf::detail

#endif // INCLUDED_mainframe_detail_group_h
