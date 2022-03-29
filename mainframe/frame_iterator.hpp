
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_frame_iterator_h
#define INCLUDED_mainframe_frame_iterator_h

#include <iterator>
#include "mainframe/series.hpp"

namespace mf
{

template< typename ... Ts >
struct frame_iterator;

template< typename ... Ts >
using frame_row = frame_iterator< Ts... >;

template< typename ... Ts >
struct frame_iterator
{
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = ptrdiff_t;

    frame_iterator( std::tuple<typename series<Ts>::iterator ...>& its )
        : m_ptrs( its )
    {}

    frame_iterator( std::tuple<series<Ts>...>& cols, int offset )
    {
        init_impl<0, Ts...>( cols, offset );
    }

    template< size_t Ind, typename U, typename ... Us >
    void init_impl( std::tuple<series<Ts>...>& cols, int offset )
    {
        series<U>& s = std::get< Ind >( cols );
        auto begin = s.begin();
        std::get< Ind >( m_ptrs ) = begin + offset;
        if constexpr ( sizeof...( Us ) > 0 ) {
            init_impl< Ind+1, Us... >( cols, offset );
        }
    }

    template< typename ... Us >
    frame_iterator< Ts..., Us... > combine( 
        const frame_iterator< Us... >& other )
    {
        auto ptr = tuple_cat( m_ptrs, other.m_ptrs );
        frame_iterator< Ts..., Us... > out{ ptr };
        return out;
    }

    template< size_t Ind >
    typename series_vector<typename std::tuple_element< Ind, std::tuple< Ts ... > >::type>::iterator column_iterator()
    {
        return std::get< Ind >( m_ptrs );
    }

    frame_iterator<Ts...> operator+( size_t n )
    {
        frame_iterator<Ts...> out{ *this };
        out += n;
        return out;
    }

    frame_iterator<Ts...> operator-( size_t n )
    {
        frame_iterator<Ts...> out{ *this };
        out -= n;
        return out;
    }

    void operator++()
    {
        operator+=( 1 );
    }

    void operator--()
    {
        operator-=( 1 );
    }

    void operator+=( size_t n )
    {
        inc< 0, Ts... >( n );
    }

    void operator-=( size_t n )
    {
        dec< 0, Ts... >( n );
    }

    template< size_t Ind, typename U, typename ... Us >
    void inc( size_t n )
    {
        std::get< Ind >( m_ptrs ) += n;
        if constexpr ( sizeof...( Us ) > 0 ) {
            inc< Ind+1, Us... >( n );
        }
    }

    template< size_t Ind, typename U, typename ... Us >
    void dec( size_t n )
    {
        std::get< Ind >( m_ptrs ) -= n;
        if constexpr ( sizeof...( Us ) > 0 ) {
            dec< Ind+1, Us... >( n );
        }
    }

    template< size_t Ind >
    typename std::tuple_element< Ind, std::tuple< Ts ... > >::type& get()
    {
        auto ptr = std::get< Ind >( m_ptrs );
        return *ptr;
    }

    template< size_t Ind, typename T >
    typename std::tuple_element< Ind, std::tuple< Ts ... > >::type& get()
    {
        auto ptr = std::get< Ind >( m_ptrs );
        return *ptr;
    }

    template< size_t Ind, typename T, typename U, typename ... Us >
    T& get_impl()
    {
        if constexpr ( sizeof...( Us ) > 0 ) {
        }
        else {
        }
    }



    frame_row< Ts... >& operator*()
    {
        return *this;
    }

    template< typename ... Us >
    friend bool operator==( const frame_iterator< Us... >& l, const frame_iterator< Us... >& r );
    template< typename ... Us >                                                          
    friend bool operator!=( const frame_iterator< Us... >& l, const frame_iterator< Us... >& r );

private:
    template< typename ... Us >
    friend struct frame_iterator;

    std::tuple< typename series<Ts>::iterator... > m_ptrs;
};

template< typename ... Us >
bool operator==( const frame_iterator< Us... >& l, const frame_iterator< Us... >& r )
{
    return l.m_ptrs == r.m_ptrs;
}

template< typename ... Us >                                                          
bool operator!=( const frame_iterator< Us... >& l, const frame_iterator< Us... >& r )
{
    return l.m_ptrs != r.m_ptrs;
}

} // namespace mf

#endif // INCLUDED_mainframe_frame_iterator_h

