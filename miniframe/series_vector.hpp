
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_miniframe_series_vector_h
#define INCLUDED_miniframe_series_vector_h

#include <type_traits>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <string>
#include <list>
#include <optional>
#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <ostream>
#include <iostream>
#include <sstream>
#include <variant>
#include <ios>

#include "miniframe/base.hpp"

namespace mf
{

class iseries_vector
{
public:
    virtual ~iseries_vector() = default;

    //virtual size_t size() const = 0;
    //virtual void resize( size_t _newsize ) = 0;
    //virtual void reserve( size_t _size ) = 0;
    //virtual bool empty() const = 0;
    //virtual size_t max_size() const = 0;
    //virtual size_t capacity() const = 0;
    //virtual void shrink_to_fit() = 0;
    //virtual void clear() = 0;
    //virtual void pop_back() = 0;

    //virtual std::unique_ptr<iseries_vector> unique() const = 0; 
    //virtual std::unique_ptr<iseries_vector> to_string() const = 0;
    //virtual std::unique_ptr<iseries_vector> clone() const = 0;
    //virtual std::unique_ptr<iseries_vector> clone_empty() const = 0;
};

template< typename T >
class series_iterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = T;
    using pointer           = T*;  // or also value_type*
    using reference         = T&;  // or also value_type&u

    series_iterator( T* ptr ) : m_curr( ptr ) {}

    difference_type operator-( const series_iterator<T>& other ) const
    {
        return m_curr - other.m_curr;
    }

    series_iterator<T> operator+( difference_type p ) const
    {
        return series_iterator<T>{ m_curr + p };
    }

    series_iterator<T> operator-( difference_type p ) const
    {
        return series_iterator<T>{ m_curr - p };
    }

    void operator++() { m_curr++; }
    void operator--() { m_curr--; }

    reference operator*() { return *m_curr; }
    pointer operator->() { return m_curr; }

protected:
    template< typename U >
    friend bool operator==( const series_iterator<U>&, const series_iterator<U>& );

    T* m_curr;
};

template< typename T >
class reverse_series_iterator : public series_iterator<T>
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = T;
    using pointer           = T*;  // or also value_type*
    using reference         = T&;  // or also value_type&u

    using series_iterator<T>::series_iterator;

    difference_type operator-( const reverse_series_iterator<T>& other ) const
    {
        return other.m_curr - this->m_curr;
    }

    series_iterator<T> operator+( difference_type p ) const
    {
        return series_iterator<T>{ this->m_curr - p };
    }

    series_iterator<T> operator-( difference_type p ) const
    {
        return series_iterator<T>{ this->m_curr + p };
    }

    void operator++() { this->m_curr--; }
    void operator--() { this->m_curr++; }
};

template< typename T >
bool operator==( const series_iterator<T>& l, const series_iterator<T>& r )
{
    return l.m_curr == r.m_curr;
}

template< typename T >
class series_vector : public iseries_vector
{
public:
    static const size_t DEFAULT_SIZE = 32;
    using value_type        = T;
    using size_type         = size_t;
    using difference_type   = ptrdiff_t;
    using reference         = value_type&;
    using const_reference   = const value_type&;
    using pointer           = value_type*;
    using const_pointer	    = const value_type*;
    using iterator          = series_iterator< T >;
    using const_iterator    = series_iterator< const T >;
    using reverse_iterator          = reverse_series_iterator< T >;
    using const_reverse_iterator    = reverse_series_iterator< const T >;

    series_vector()
    {
        create_storage( DEFAULT_SIZE );
    }
    series_vector( size_type count, const T& value ) 
    {
        std::ios_base::fmtflags f{ std::cout.flags() };
        create_storage( count );
        for ( ; m_end != m_begin + count; ++m_end ) {
            new( m_end ) T{ value };
        }
        std::cout.flags(f);
    }
    explicit series_vector( size_type count )
    {
        create_storage( count );
        for ( ; m_end != m_begin + count; ++m_end ) {
            new( m_end ) T{};
        }
    }
    template< typename InputIt >
    series_vector( InputIt f, InputIt l )
    {
        auto count = l - f;
        create_storage( count );
        InputIt curr = f;
        for ( ; curr != l; ++curr, ++m_end ) {
            new( m_end ) T{ *curr };
        }
    }
    series_vector( const series_vector& other )
    {
        create_storage( other.capacity() );
        auto* curr = other.m_begin;
        for ( ; curr != other.m_end; ++curr, ++m_end ) {
            new( m_end ) T{ *curr };
        }
    }
    series_vector( series_vector&& other ) 
    {
        create_storage( DEFAULT_SIZE );
        std::swap( m_begin, other.m_begin );
        std::swap( m_end, other.m_end );
        std::swap( m_max, other.m_max );
    }
    series_vector( std::initializer_list<T> _init )
        : series_vector( _init.begin(), _init.end() )
    {
    }

    void deallocate( T* b, T* e )
    {
        auto curr = b;
        for ( ; curr != e; ++curr ) {
            curr->~T();
        }
        free( b );
    }

    virtual ~series_vector()
    {
        deallocate( m_begin, m_end );
    }

    series_vector& operator=( const series_vector& in )
    {
        series_vector other{ in };
        std::swap( m_begin, other.m_begin );
        std::swap( m_end, other.m_end );
        std::swap( m_max, other.m_max );
        return *this;
    }

    series_vector& operator=( series_vector&& other )
    {
        std::swap( m_begin, other.m_begin );
        std::swap( m_end, other.m_end );
        std::swap( m_max, other.m_max );
        return *this;
    }

    series_vector& operator=( std::initializer_list<T> _init )
    {
        series_vector other{ _init };
        std::swap( m_begin, other.m_begin );
        std::swap( m_end, other.m_end );
        std::swap( m_max, other.m_max );
        return *this;
    }

    void assign( size_type count, const T& value )
    {
        allocate_copy_if_needed( count );
        auto curr = m_begin;
        // operator= for existing
        for( ; curr < m_begin + count && curr < m_end; ++curr ) {
            *curr = value;
        }
        // copy-ctor for new. maybe none
        for( ; curr < m_begin + count; ++curr ) {
            new( curr ) T{ value };
        }
        // dtor for remaining. Maybe none.
        for( ; curr < m_end; ++curr ) {
            curr->~T();
        }
        m_end = m_begin + count;
    }
    template< typename InputIt >
    void assign( InputIt inbegin, InputIt inend )
    {
        allocate_copy_if_needed( inend - inbegin );
        auto curr = m_begin;
        auto incurr = inbegin;
        // operator= for existing
        for( ; incurr != inend && curr < m_end; ++curr, ++incurr ) {
            *curr = *incurr;
        }
        // copy-ctor for new. maybe none
        for( ; incurr != inend; ++curr, ++incurr ) {
            new( curr ) T{ *incurr };
        }
        // dtor for remaining. Maybe none.
        for( ; curr < m_end; ++curr ) {
            curr->~T();
        }
        m_end = m_begin + ( inend - inbegin );
    }
    void assign( std::initializer_list<T> _init )
    {
        assign( _init.begin(), _init.end() );
    }

    reference at( size_type pos )
    {
        if ( pos >= size() ) {
            throw std::out_of_range{ 
                "size() is " +
                std::to_string( size() ) + 
                ", pos is " +
                std::to_string( pos ) };
        }
        return m_begin[ pos ];
    }
    const_reference at( size_type pos ) const
    {
        if ( pos >= size() ) {
            throw std::out_of_range{ 
                "size() is " +
                std::to_string( size() ) + 
                ", pos is " +
                std::to_string( pos ) };
        }
        return m_begin[ pos ];
    }

    reference operator[]( size_type pos )
    {
        return m_begin[ pos ];
    }
    const_reference operator[]( size_type pos ) const
    {
        return m_begin[ pos ];
    }

    reference front()
    {
        return *m_begin;
    }
    const_reference front() const
    {
        return *m_begin;
    }

    reference back()
    {
        return *(m_end - 1);
    }
    const_reference back() const
    {
        return *(m_end - 1);
    }

    T* data() noexcept
    {
        return m_begin;
    }
    const T* data() const noexcept
    {
        return m_begin;
    }

    iterator begin() noexcept
    {
        return iterator{ m_begin };
    }
    const_iterator begin() const noexcept
    {
        return const_iterator{ m_begin };
    }
    const_iterator cbegin() const noexcept
    {
        return const_iterator{ m_begin };
    }

    iterator end() noexcept
    {
        return iterator{ m_end };
    }
    const_iterator end() const noexcept
    {
        return const_iterator{ m_end };
    }
    const_iterator cend() const noexcept
    {
        return const_iterator{ m_end };
    }

    reverse_iterator rbegin() noexcept
    {
        return reverse_iterator{ m_end-1 };
    }
    const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator{ m_end-1 };
    }
    const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator{ m_end-1 };
    }

    reverse_iterator rend() noexcept
    {
        return reverse_iterator{ m_begin-1 };
    }
    const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator{ m_begin-1 };
    }
    const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator{ m_begin-1 };
    }

    bool empty() const noexcept
    {
        return m_end == m_begin;
    }
    size_type size() const noexcept
    {
        return m_end - m_begin;
    }
    size_type max_size() const noexcept
    {
        return std::numeric_limits<difference_type>::max();
    }
    void reserve( size_type newsize )
    {
        size_t n = pow_2( newsize );
        T* nbegin = static_cast<T*>( malloc ( n * sizeof( T ) ) );
        T* nend = duplicate( m_begin, m_end, nbegin );
        deallocate( m_begin, m_end );
        m_begin = nbegin;
        m_end = nend;
        m_max = m_begin + n;
    }
    size_type capacity() const noexcept
    {
        return m_max - m_begin;
    }
    void clear() noexcept
    {
        deallocate( m_begin, m_end );
        m_begin = m_end = m_max = nullptr;
    }

    //iterator insert( const_iterator _pos, const T& _value )
    //{
    //    return m_array.insert( _pos, _value );
    //}
    //iterator insert( const_iterator _pos, T&& _value )
    //{
    //    return m_array.insert( _pos, std::move( _value ) );
    //}
    //iterator insert( const_iterator _pos, size_type _count, const T& _value )
    //{
    //    return m_array.insert( _pos, _count,  _value );
    //}
    //template< typename InputIt >
    //iterator insert( const_iterator _pos, InputIt _first, InputIt _last )
    //{
    //    return m_array.insert( _pos, _first, _last );
    //}
    //iterator insert( const_iterator _pos, std::initializer_list<T> _init )
    //{
    //    return m_array.insert( _pos, _init );
    //}

    //template< class... Args >
    //iterator emplace( const_iterator _pos, Args&&... _args )
    //{
    //    return m_array.emplace( _pos, std::forward( _args... ) );
    //}

    //iterator erase( const_iterator _pos )
    //{
    //    return m_array.erase( _pos );
    //}
    //iterator erase( const_iterator _first, const_iterator _last )
    //{
    //    return m_array.erase( _first, _last );
    //}

    //void push_back( const T& _value )
    //{
    //    m_array.push_back( _value );
    //}
    //void push_back( T&& _value )
    //{
    //    m_array.push_back( std::move( _value ) );
    //}
    //template< typename... Args > 
    //reference emplace_back( Args&&... args )
    //{
    //    if ( size() == capacity() ) {
    //        copy_allocate_add( capacity() );
    //    }
    //    T t{ std::forward( args... ) };
    //    *m_end = std::move( t );
    //    //*m_end = std::forward( args... );
    //    reference out = *m_end;
    //    m_end++;
    //    return out;
    //}
    //void pop_back()
    //{
    //    m_array.pop_back();
    //}
    //void resize( size_type _newsize ) 
    //{ 
    //    m_array.resize( _newsize ); 
    //}
    //void resize( size_type _count, const value_type& _value )
    //{
    //    m_array.resize( _count, _value ); 
    //}
    //void swap( series_vector<T>& other ) SVB_NOEXCEPT
    //{
    //    swap( m_array, other.m_array );
    //}

    //template< class _T, class _Alloc >
    //friend bool operator==( const std::vector<_T,_Alloc>& _left, 
    //                                      const std::vector<_T,_Alloc>& _right );
    //template< class _T, class _Alloc >
    //friend bool operator!=( const std::vector<_T,_Alloc>& _left, 
    //                                      const std::vector<_T,_Alloc>& _right );
    //template< class _T, class _Alloc >
    //friend bool operator<( const std::vector<_T,_Alloc>& _left, 
    //                                     const std::vector<_T,_Alloc>& _right );
    //template< class _T, class _Alloc >
    //friend bool operator<=( const std::vector<_T,_Alloc>& _left, 
    //                                      const std::vector<_T,_Alloc>& _right );
    //template< class _T, class _Alloc >
    //friend bool operator>( const std::vector<_T,_Alloc>& _left, 
    //                                     const std::vector<_T,_Alloc>& _right );
    //template< class _T, class _Alloc >
    //friend bool operator>=( const std::vector<_T,_Alloc>& _left, 
    //                                      const std::vector<_T,_Alloc>& _right );

private:

    size_t pow_2( size_t n )
    {
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        n++;
        return n;
    }

    size_t next_pow_2( size_t n )
    {
        return pow_2( n+1 );
    }

    void create_storage( size_t n )
    {
        n = next_pow_2( n );
        m_begin = static_cast<T*>( malloc( n * sizeof( T ) ) );
        m_end = m_begin;
        m_max = m_begin + n;
    }

    T* duplicate( T* b, T* e, T* ob )
    {
        for ( ; b < e; ++b, ++ob ) {
            new( ob ) T{ *b };
        }
        return ob;
    }

    void allocate_copy_if_needed( size_t needed )
    {
        if ( needed > capacity() ) {
            size_t n = next_pow_2( needed );
            T* nbegin = static_cast<T*>( malloc ( n * sizeof( T ) ) );
            T* nend = duplicate( m_begin, m_end, nbegin );
            deallocate( m_begin, m_end );
            m_begin = nbegin;
            m_end = nend;
            m_max = m_begin + n;
        }
    }

    T * m_begin;
    T * m_end;
    T * m_max;
};

//template< typename T, typename Allocator >
//bool operator==( const series_vector<T>& _left, 
//                               const series_vector<T>& _right )
//{
//    return _left.m_array == _right.m_array;
//}
//
//template< typename T, typename Allocator >
//bool operator!=( const series_vector<T>& _left, 
//                               const series_vector<T>& _right )
//{
//    return _left.m_array != _right.m_array;
//}
//
//template< typename T, typename Allocator >
//bool operator<( const series_vector<T>& _left, 
//                              const series_vector<T>& _right )
//{
//    return _left.m_array < _right.m_array;
//}
//
//template< typename T, typename Allocator >
//bool operator<=( const series_vector<T>& _left, 
//                               const series_vector<T>& _right )
//{
//    return _left.m_array <= _right.m_array;
//}
//
//template< typename T, typename Allocator >
//bool operator>( const series_vector<T>& _left, 
//                              const series_vector<T>& _right )
//{
//    return _left.m_array > _right.m_array;
//}
//
//template< typename T, typename Allocator >
//bool operator>=( const series_vector<T>& _left, 
//                               const series_vector<T>& _right )
//{
//    return _left.m_array >= _right.m_array;
//}
//
//
//template<typename T>
//class series_vector : public series_vector<T>
//{
//public:
//    series_vector() = default;
//
//    std::unique_ptr<iseries_vector> unique() const override
//    {
//        auto out = std::make_unique<series_vector<T>>();
//        std::unordered_set<T> bag;
//        for ( const auto &elem : *this ) {
//            if ( bag.find( elem ) == bag.end() ) {
//                out->push_back( elem );
//                bag.insert( elem );
//            }
//        }
//        return out;
//    }
//
//    std::unique_ptr<iseries_vector> to_string() const override
//    {
//        auto out = std::make_unique<series_vector<std::string>>();
//        out->reserve( this->size() );
//        for ( const auto &elem : *this ) {
//          std::stringstream ss;
//          detail::stringify(ss, elem, true);
//          out->push_back(ss.str());
//        }
//        return out;
//    }
//
//    std::unique_ptr<iseries_vector> clone() const override
//    {
//        return std::make_unique<series_vector<T>>( *this );
//    }
//
//    std::unique_ptr<iseries_vector> clone_empty() const override
//    {
//        return std::make_unique<series_vector<T>>();
//    }
//};

} // namespace mf


#endif // INCLUDED_miniframe_series_vector_h

