
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

template< template<typename> class Iter, typename ... Ts >
class _frame_iterator;

template< template<typename> class Iter, typename ... Ts >
class frame_row
{
public:
    frame_row() = default;

    frame_row( const std::tuple< Iter<Ts>... >& iter ) 
        : m_iters( iter ) 
    {}

    template< size_t Ind >
    typename std::tuple_element< Ind, std::tuple< Ts ... > >::type& get()
    {
        auto ptr = std::get< Ind >( m_iters );
        return *ptr;
    }

    template< size_t Ind, typename T >
    typename std::tuple_element< Ind, std::tuple< Ts ... > >::type& get()
    {
        auto ptr = std::get< Ind >( m_iters );
        return *ptr;
    }

private:

    template< template<typename> class It, typename ... Us >
    friend class _frame_iterator;
    template< typename ... Us >
    friend class frame_iterator;
    template< typename ... Us >
    friend class reverse_frame_iterator;

    template< template<typename> typename It, typename ... Us >
    friend bool operator==( const frame_row< It, Us... >& l, 
                            const frame_row< It, Us... >& r );
    template< template<typename> typename It, typename ... Us >
    friend bool operator!=( const frame_row< It, Us... >& l, 
                            const frame_row< It, Us... >& r );

    std::tuple< Iter<Ts>... > m_iters;
};

template< typename T >
using series_iterator = typename series<T>::iterator;

template< typename T >
using series_reverse_iterator = typename series<T>::reverse_iterator;

template< template<typename> class Iter, typename ... Ts >
class _frame_iterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = ptrdiff_t;

    _frame_iterator() = default;

    _frame_iterator( const std::tuple<Iter<Ts> ...>& its )
        : m_row( its )
    {
    }

    template< size_t Ind >
    typename series_vector<typename std::tuple_element< Ind, std::tuple< Ts ... > >::type>::iterator column_iterator()
    {
        return std::get< Ind >( m_row.m_iters );
    }

    void operator++()
    {
        operator+=( 1 );
    }

    void operator++(int)
    {
        operator+=( 1 );
    }

    void operator--()
    {
        operator-=( 1 );
    }

    void operator--(int)
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
        std::get< Ind >( m_row.m_iters ) += n;
        if constexpr ( sizeof...( Us ) > 0 ) {
            inc< Ind+1, Us... >( n );
        }
    }

    template< size_t Ind, typename U, typename ... Us >
    void dec( size_t n )
    {
        std::get< Ind >( m_row.m_iters ) -= n;
        if constexpr ( sizeof...( Us ) > 0 ) {
            dec< Ind+1, Us... >( n );
        }
    }

    frame_row< Iter, Ts... >& operator*()
    {
        return m_row;
    }

    const frame_row< Iter, Ts... >& operator*() const
    {
        return m_row;
    }

    frame_row< Iter, Ts... >* operator->()
    {
        return &m_row;
    }

    const frame_row< Iter, Ts... >* operator->() const
    {
        return &m_row;
    }

    template< template<typename> typename It, typename ... Us >
    friend bool operator==( const _frame_iterator< It, Us... >& l, 
                            const _frame_iterator< It, Us... >& r );
    template< template<typename> typename It, typename ... Us >
    friend bool operator!=( const _frame_iterator< It, Us... >& l, 
                            const _frame_iterator< It, Us... >& r );

protected:

    frame_row< Iter, Ts... > m_row;
};

template< typename ... Ts >
class frame_iterator : public _frame_iterator< series_iterator, Ts... >
{
public:
    frame_iterator( const std::tuple<series_iterator<Ts> ...>& its )
        : _frame_iterator<series_iterator, Ts...>( its )
    {
    }

    frame_iterator( std::tuple<series<Ts>...>& cols, int offset )
    {
        init_impl<0, Ts...>( cols, offset );
    }

    template< size_t Ind, typename U, typename ... Us >
    void init_impl( std::tuple<series<Ts>...>& cols, int offset )
    {
        series<U>& s = std::get< Ind >( cols );
        auto begin = s.begin();
        std::get< Ind >( this->m_row.m_iters ) = begin + offset;
        if constexpr ( sizeof...( Us ) > 0 ) {
            init_impl< Ind+1, Us... >( cols, offset );
        }
    }

    ptrdiff_t operator-( const frame_iterator<Ts...>& other ) const
    {
        const auto& t = std::get<0>(this->m_row.m_iters);
        const auto& o = std::get<0>(other->m_iters);
        return t - o;
    }

    template< typename ... Us >
    frame_iterator< Ts..., Us... > combine( 
        const frame_iterator< Us... >& other )
    {
        auto ptr = tuple_cat( 
            this->m_row.m_iters, other->m_iters );
        frame_iterator< Ts..., Us... > out{ ptr };
        return out;
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
};

template< typename ... Ts >
class reverse_frame_iterator : public _frame_iterator< series_reverse_iterator, Ts... >
{
public:
    reverse_frame_iterator( const std::tuple<series_reverse_iterator<Ts> ...>& its )
        : _frame_iterator<series_reverse_iterator, Ts...>( its )
    {
    }

    reverse_frame_iterator( std::tuple<series<Ts>...>& cols, int offset )
    {
        init_impl<0, Ts...>( cols, offset );
    }

    template< size_t Ind, typename U, typename ... Us >
    void init_impl( std::tuple<series<Ts>...>& cols, int offset )
    {
        series<U>& s = std::get< Ind >( cols );
        auto begin = s.rbegin();
        std::get< Ind >( this->m_row.m_iters ) = begin + offset;
        if constexpr ( sizeof...( Us ) > 0 ) {
            init_impl< Ind+1, Us... >( cols, offset );
        }
    }

    ptrdiff_t operator-( const reverse_frame_iterator<Ts...>& other ) const
    {
        const auto& t = std::get<0>(this->m_row.m_iters);
        const auto& o = std::get<0>(other->m_iters);
        return t - o;
    }

    template< typename ... Us >
    reverse_frame_iterator< Ts..., Us... > combine( 
        const reverse_frame_iterator< Us... >& other )
    {
        auto ptr = tuple_cat( 
            this->m_row.m_iters, other->m_iters );
        reverse_frame_iterator< Ts..., Us... > out{ ptr };
        return out;
    }

    reverse_frame_iterator<Ts...> operator+( size_t n )
    {
        reverse_frame_iterator<Ts...> out{ *this };
        out += n;
        return out;
    }

    reverse_frame_iterator<Ts...> operator-( size_t n )
    {
        reverse_frame_iterator<Ts...> out{ *this };
        out -= n;
        return out;
    }
};

template< template<typename> typename It, typename ... Us >
bool operator==( const _frame_iterator< It, Us... >& l, const _frame_iterator< It, Us... >& r )
{
    return l.m_row == r.m_row;
}

template< template<typename> typename It, typename ... Us >                                                          
bool operator!=( const _frame_iterator< It, Us... >& l, const _frame_iterator< It, Us... >& r )
{
    return l.m_row != r.m_row;
}

template< template<typename> typename It, typename ... Us >
bool operator==( const frame_row< It, Us... >& l, const frame_row< It, Us... >& r )
{
    return l.m_iters == r.m_iters;
}

template< template<typename> typename It, typename ... Us >
bool operator!=( const frame_row< It, Us... >& l, const frame_row< It, Us... >& r )
{
    return l.m_iters != r.m_iters;
}

} // namespace mf

#endif // INCLUDED_mainframe_frame_iterator_h

