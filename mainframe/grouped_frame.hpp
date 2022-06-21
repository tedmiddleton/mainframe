
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_grouped_frame_h
#define INCLUDED_mainframe_grouped_frame_h

#include "mainframe/frame.hpp"

namespace mf
{

template< size_t ... Idx >
struct group_index
{
    //group_index( size_t indices... )
    //{
    //    init_impl<0>( indices... );
    //}

    //template< size_t Ind=0 >
    //void init_impl( size_t idx, size_t remaining... )
    //{
    //    m_indices[ Ind ] = idx;
    //    if constexpr ( Ind+1 < sizeof...( Idx ) ) {
    //        init_impl<Ind+1>( remaining... );
    //    }
    //}

    //std::array< size_t, sizeof...( Idx ) > m_indices;
};

//template< template <size_t ...> class GroupIdx, typename ... Ts >
template< class GroupIdx, typename ... Ts >
class grouped_frame;

template< size_t ... Idx, typename ... Ts >
class grouped_frame< group_index<Idx...>, Ts... >
{
public:
    grouped_frame( frame< Ts... > f )
        : m_frame( f )
    {}

private:
    frame< Ts... > m_frame;
};

template< size_t ... Idx, typename ... Ts >
grouped_frame<group_index<Idx...>, Ts...> groupby( frame< Ts... > f, columnindex<Idx>... )
{
    return grouped_frame<group_index<Idx...>, Ts...>{ f };
}


} // namespace mf

#endif // INCLUDED_mainframe_grouped_frame_h
