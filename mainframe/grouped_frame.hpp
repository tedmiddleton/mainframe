//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_grouped_frame_h
#define INCLUDED_mainframe_grouped_frame_h

#include "mainframe/frame.hpp"
#include <map>

namespace mf
{

template<size_t... Inds>
struct index_defn
{};

template<typename IndexDefn, typename... Ts>
class grouped_frame;

namespace detail {

// Does the index def contain that column index?
// index_defn_contains<3, index_defn<0, 2, 1>>::value == false
// index_defn_contains<1, index_defn<0, 2, 1>>::value == true
template<size_t ColNum, typename IndexDefn>
struct index_defn_contains : std::false_type
{};

template<size_t ColNum, size_t... Inds>
struct index_defn_contains<ColNum, index_defn<ColNum, Inds...>> : std::true_type
{};

template<size_t ColNum, size_t Ind, size_t... Inds>
struct index_defn_contains<ColNum, index_defn<Ind, Inds...>> : index_defn_contains<ColNum, index_defn<Inds...>>
{};

// Get the ColNum'th index
// index_defn_deref<3, index_defn<32, 12, 11, 34>>::value == 34
// index_defn_deref<1, index_defn<32, 12, 11, 34>>::value == 12
template<size_t ColNum, typename IndexDefn>
struct index_defn_deref;

template<size_t ColNum, size_t Ind, size_t... Inds>
struct index_defn_deref<ColNum, index_defn<Ind, Inds...>>
{
    static const size_t value = index_defn_deref<ColNum-1, index_defn<Inds...>>::value;
};

template<size_t Ind, size_t... Inds>
struct index_defn_deref<0, index_defn<Ind, Inds...>>
{
    static const size_t value = Ind;
};

// Make a Tpl with the index defn's columns/items
// index_defn_select<index_defn<0, 4>, tuple<char, short, int, long>>::type == tuple<char, long>
// index_defn_select<index_defn<2, 0>, frame<year_month, double, float>>::type == frame<float, year_month>
template<typename IndexDefn, typename Tpl>
struct index_defn_select;

template<size_t Ind, size_t... Inds, template<typename...> typename Tpl, typename... Ts>
struct index_defn_select<index_defn<Ind, Inds...>, Tpl<Ts...>>
{
    using ind_type = typename pack_element<Ind, Ts...>::type;
    using remaining = typename index_defn_select<index_defn<Inds...>, Tpl<Ts...>>::type;
    using type = typename prepend<ind_type, remaining>::type;
};

template<size_t Ind, template<typename...> typename Tpl, typename... Ts>
struct index_defn_select<index_defn<Ind>, Tpl<Ts...>>
{
    using ind_type = typename pack_element<Ind, Ts...>::type;
    using type = Tpl<ind_type>;
};

// Prepend an index to an index defn
// index_defn_prepend<6, index_defn<0, 1, 2>>::type == index_defn<6, 0, 1, 2>
template<size_t Ind, typename IndexDefn>
struct index_defn_prepend;

template<size_t Ind, size_t... Inds>
struct index_defn_prepend<Ind, index_defn<Inds...>>
{
    using type = index_defn<Ind, Inds...>;
};

template<size_t CurrInd, size_t Num, typename IndexDefn>
struct index_defn_invert_impl
{
    using remaining = typename index_defn_invert_impl<CurrInd+1, Num, IndexDefn>::type;
    using type = typename std::conditional<
        index_defn_contains<CurrInd, IndexDefn>::value,
        remaining,
        typename index_defn_prepend<CurrInd, remaining>::type>::type;
};

template<size_t CurrInd, typename IndexDefn>
struct index_defn_invert_impl<CurrInd, CurrInd, IndexDefn>
{
    using type = index_defn<>;
};

// Invert the index definition for a Tpl
// index_defn_invert<index_defn<0, 3>, frame<char, short, int, long>>::type == index_defn<1, 2>
// index_defn_invert<index_defn<3, 0>, frame<char, short, int, long>>::type == index_defn<1, 2>
template<typename IndexDefn, typename Tpl>
struct index_defn_invert;

template<typename IndexDefn, template <typename...> typename Tpl, typename... Ts>
struct index_defn_invert<IndexDefn, Tpl<Ts...>>
{
    using type = typename index_defn_invert_impl<0, sizeof...(Ts), IndexDefn>::type;
};


// A frame with only the indexed columns in it.
// eg indexed_frame<index_defn<4, 0>, frame<short, double, double, int, year_month>>::type
// == frame<year_month, short>
// This is used to build the frame index
template<typename IndexDefn, typename Frame>
struct get_index_frame;

template<size_t Ind, size_t... Inds, template <typename...> typename Frame, typename... Ts>
struct get_index_frame<index_defn<Ind, Inds...>, Frame<Ts...>>
{
    using ind_type = typename pack_element<Ind, Ts...>::type;
    using remaining = typename get_index_frame<index_defn<Inds...>, Frame<Ts...>>::type;
    using type = typename prepend<ind_type, remaining>::type;

    static type op( Frame<Ts...>& f )
    {
        return f.columns(columnindex<Ind>{}, columnindex<Inds>{}...);
    }

    static const type op( const Frame<Ts...>& f )
    {
        return f.columns(columnindex<Ind>{}, columnindex<Inds>{}...);
    }
};

template<size_t Ind, template <typename...> typename Frame, typename... Ts>
struct get_index_frame<index_defn<Ind>, Frame<Ts...>>
{
    using ind_type = typename pack_element<Ind, Ts...>::type;
    using type = frame<ind_type>;
};

// A frame with all of the indexed columns removed
// eg unindexed_frame<index_defn<4, 0>, frame<short, double, double, int, year_month>>::type
// is frame<double, double, int>
template<typename IndexDefn, typename Frame>
using get_unindex_frame = get_index_frame<typename index_defn_invert<IndexDefn, Frame>::type, Frame>;

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
    using type = typename join_frames<indexed_cols1, indexed_cols2>::type;

    static type op( Frame& f )
    {
        return f.columns(columnindex<Inds1>{}..., columnindex<Inds2>{}...);
    }

    static const type op( const Frame& f )
    {
        return f.columns(columnindex<Inds1>{}..., columnindex<Inds2>{}...);
    }
};

} // namespace detail

template<size_t... GroupInds, typename... Ts>
class grouped_frame<index_defn<GroupInds...>, Ts...>
{
    // Pulling types out of namespaces for convenience
    template<size_t Ind, typename... Us>
    using pack_element = detail::pack_element<Ind, Us...>;
    template<typename T, typename U>
    using join_frames = detail::join_frames<T, U>;

    // The index definition for this grouped_frame 
    using group_index_defn = index_defn<GroupInds...>;

    // The index frame is a frame made of the indexed columns
    using get_index_frame = 
        detail::get_index_frame<group_index_defn, frame<Ts...>>;
    using index_frame = typename get_index_frame::type;

    // The result frame for any given function invocation has the index columns 
    // but also the grouped function columns
    template<typename IndexDefn>
    using get_result_frame =
        detail::get_result_frame<group_index_defn, IndexDefn, frame<Ts...>>;
    template<typename IndexDefn>
    using result_frame = typename get_result_frame<IndexDefn>::type;

    using key_type = typename index_frame::row_type; // frame_row
    using map_value_type = std::vector<size_t>;
    using map_type = std::map<key_type, map_value_type>;

public:
    grouped_frame(frame<Ts...> f)
        : m_frame(f)
    {}

    template<size_t... Inds>
    result_frame<index_defn<Inds...>>
    min(columnindex<Inds>... ci) const
    {
        return group_op<numeric_max, row_min>( "min", ci... ); 
    }

    template<size_t... Inds>
    result_frame<index_defn<Inds...>>
    max(columnindex<Inds>... ci) const
    {
        return group_op<numeric_min, row_max>( "max", ci... ); 
    }

    typename join_frames<index_frame, frame<int>>::type
    count() const
    {
        build_index();

        auto f1 = get_index_frame::op( m_frame );
        auto outframe = f1.template new_series<int>( "count" );
        outframe.clear();

        auto it = m_idx.cbegin();
        auto end = m_idx.cend();
        for ( ; it != end; ++it  ) {
            const key_type& kt = it->first;
            const auto& rows = it->second;
            auto count = rows.size();
            outframe.push_back( kt, count );
        }

        return outframe;
    }

    template<size_t... Inds>
    result_frame<index_defn<Inds...>>
    sum(columnindex<Inds>... ci) const
    {
        return group_op<numeric_zero, row_sum>( "sum", ci... ); 
    }

private:
    template<size_t Ind, typename IndexDefn, size_t Num>
    void
    augment_column_names(std::string opstr, std::array<std::string, Num>& names) const
    {
        if constexpr (detail::index_defn_contains<Ind, IndexDefn>::value) {
            std::string& name = names[Ind];
            std::stringstream ss;
            ss << opstr << "(" << name << ")";
            name = ss.str();
        }
        if constexpr (Ind+1 < Num) {
            augment_column_names<Ind+1, IndexDefn>(opstr, names);
        }
    }

    template<typename T>
    struct numeric_zero
    {
        T
        operator()()
        {
            return static_cast<T>(0);
        }
    };

    template<typename T>
    struct numeric_max
    {
        T
        operator()()
        {
            using std::numeric_limits;
            return numeric_limits<T>::max(); 
        }
    };

    template<typename T>
    struct numeric_min
    {
        T
        operator()()
        {
            using std::numeric_limits;
            return numeric_limits<T>::min(); 
        }
    };

    template<typename T>
    struct row_sum
    {
        T
        operator()( const T& t1, const T& t2 )
        {
            return t1 + t2; 
        }
    };

    template<typename T>
    struct row_min
    {
        T
        operator()( const T& t1, const T& t2 )
        {
            using std::min;
            return min(t1, t2); 
        }
    };

    template<typename T>
    struct row_max
    {
        T
        operator()( const T& t1, const T& t2 )
        {
            using std::max;
            return max(t1, t2); 
        }
    };

    template<size_t Ind=0, template<typename>typename Op, typename... Us>
    void
    group_row_init(frame_row<Us...>& fr) const
    {
        using T = typename detail::pack_element<Ind, Us...>::type;

        // Don't do anything to the index columns!
        if constexpr (!detail::index_defn_contains<Ind, group_index_defn>::value) {
            columnindex<Ind> ci;
            fr.at( ci ) = Op<T>()();
        }

        if constexpr (Ind+1 < sizeof...(Us)) {
            group_row_init<Ind+1, Op>(fr);
        }
    }

    template<size_t Ind=0, template<typename>typename Op, bool IsConst, typename... Us>
    void
    group_row_op(const _row_proxy<IsConst, Us...>& rp, frame_row<Us...>& fr) const
    {
        using T = typename detail::pack_element<Ind, Us...>::type;

        // Don't do anything to the index columns!
        if constexpr (!detail::index_defn_contains<Ind, group_index_defn>::value) {
            columnindex<Ind> ci;
            fr.at( ci ) = Op<T>()(fr.at( ci ), rp.at( ci ));
        }

        if constexpr (Ind+1 < sizeof...(Us)) {
            group_row_op<Ind+1, Op>(rp, fr);
        }
    }

    template<template<typename>typename Init, template<typename>typename RowOp, size_t... Inds>
    result_frame<index_defn<Inds...>>
    group_op(std::string name, columnindex<Inds>...) const
    {
        build_index();
        auto inframe = get_result_frame<index_defn<Inds...>>::op( m_frame );
        auto outframe = inframe;
        outframe.clear();
        auto names = outframe.column_names();
        augment_column_names<0, index_defn<Inds...>>(name, names);
        outframe.set_column_names(names);

        auto it = m_idx.cbegin();
        auto end = m_idx.cend();
        for ( ; it != end; ++it  ) {
            const map_value_type& rows = it->second;
            auto rit = rows.cbegin();
            auto rend = rows.cend();
            // frame_row
            typename result_frame<index_defn<Inds...>>::row_type oprow;
            oprow = inframe[ *rit ];
            group_row_init<0, Init>( oprow );
            for ( ; rit != rend; ++rit ) {
                const auto& row = inframe[ *rit ];
                group_row_op<0, RowOp>( row, oprow );
            }
            outframe.push_back( oprow );
        }

        return outframe;
    }

    void
    build_index() const
    {
        if (m_idx.size() != 0) {
            return;
        }

        const index_frame ifr{ get_index_frame::op( m_frame ) };

        for ( size_t i = 0; i < ifr.size(); ++i ) {
            auto row = ifr[i];
            key_type key{ row };
            auto findit = m_idx.find( key );
            if ( findit == m_idx.end() ) {
                map_value_type arr;
                arr.push_back(i);
                m_idx[key] = arr;
            }
            else {
                findit->second.push_back(i);
            }
        }
    }
 
    void 
    debug_index() const
    {
        std::cout << "Index:\n";
        for ( auto& [key, val] : m_idx ) {
            std::cout << "key " << key << ": ";
            std::cout << "[ ";
            int i = 0;
            for ( auto& ind : val ) {
                if (i++ != 0) { std::cout << ", "; }
                std::cout << ind;
            }
            std::cout << " ]\n";
        }
    }

    mutable map_type m_idx;
    frame<Ts...> m_frame;
};

template<size_t... Idx, typename... Ts>
grouped_frame<index_defn<Idx...>, Ts...>
groupby(frame<Ts...> f, columnindex<Idx>...)
{
    return grouped_frame<index_defn<Idx...>, Ts...>{ f };
}


} // namespace mf

#endif // INCLUDED_mainframe_grouped_frame_h
