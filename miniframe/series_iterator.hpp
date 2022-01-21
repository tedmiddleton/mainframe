
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_miniframe_series_iterator_h
#define INCLUDED_miniframe_series_iterator_h

#include <cstdlib>
#include <iterator>

template<typename T, typename It>
class series_iterator
{
public:
    series_iterator( It _it ) :
        m_it( _it )
    {}

    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = ptrdiff_t;
    using value_type        = T;
    using pointer           = value_type*;
    using reference         = value_type&;

    bool operator==( const series_iterator& other ) const
    {
        return m_it == other.m_it;
    }

    bool operator!=( const series_iterator& other ) const
    {
        return m_it != other.m_it;
    }

    void operator++()
    {
        m_it++;
    }

    void operator--()
    {
        m_it--;
    }

    void operator+=( size_t _amt )
    {
        m_it += _amt;
    }

    void operator-=( size_t _amt )
    {
        m_it -= _amt;
    }

    series_iterator operator+( size_t _amt )
    {
        return m_it + _amt;
    }

    series_iterator operator-( size_t _amt )
    {
        return m_it - _amt;
    }

    value_type& operator*()
    {
        return *m_it;
    }

    value_type* operator->()
    {
        return &(*m_it);
    }

private:
    It m_it;
};


#endif // INCLUDED_miniframe_series_iterator_h
