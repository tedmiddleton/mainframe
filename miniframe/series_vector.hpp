
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

#include "miniframe/base.hpp"
#include "miniframe/series_iterator.hpp"

#define CPP20LEVEL 202002L
#define CPP17LEVEL 201703L
#define CPP14LEVEL 201402L
#define CPP11LEVEL 201103L

#if __cplusplus >= CPP20LEVEL
#define SVB_NOEXCEPT noexcept
#define SVB_CONSTEXPR constexpr
#elif __cplusplus >= CPP17LEVEL
#define SVB_NOEXCEPT noexcept
#define SVB_CONSTEXPR
#elif __cplusplus >= CPP14LEVEL
#define SVB_NOEXCEPT
#define SVB_CONSTEXPR
#elif __cplusplus >= CPP11LEVEL
#define SVB_NOEXCEPT
#define SVB_CONSTEXPR
#else
#error "cppframe requires at least C++11"
#endif

namespace mf
{

class iseries_vector
{
public:
    virtual ~iseries_vector() = default;

    virtual size_t size() const = 0;
    virtual void resize( size_t _newsize ) = 0;
    virtual void reserve( size_t _size ) = 0;
    virtual bool empty() const = 0;
    virtual size_t max_size() const = 0;
    virtual size_t capacity() const = 0;
    virtual void shrink_to_fit() = 0;
    virtual void clear() = 0;
    virtual void pop_back() = 0;

    virtual std::unique_ptr<iseries_vector> unique() const = 0; 
    virtual std::unique_ptr<iseries_vector> to_string() const = 0;
    virtual std::unique_ptr<iseries_vector> clone() const = 0;
    virtual std::unique_ptr<iseries_vector> clone_empty() const = 0;
};

// Basically a pass-thru for std::vector, but with a base interface for type 
// erasure.
template<typename T>
class series_vector_base : public iseries_vector
{
public:
    using value_type        = typename std::vector<T>::value_type;
    using size_type         = typename std::vector<T>::size_type;
    using difference_type   = typename std::vector<T>::difference_type;
    using reference         = typename std::vector<T>::reference;
    using const_reference   = typename std::vector<T>::const_reference;
    using pointer           = typename std::vector<T>::pointer;
    using const_pointer	    = typename std::vector<T>::const_pointer;
    using iterator          = typename std::vector<T>::iterator;
    using const_iterator    = typename std::vector<T>::const_iterator;
    using reverse_iterator  = typename std::vector<T>::reverse_iterator;
    using const_reverse_iterator    
        = typename std::vector<T>::const_reverse_iterator;

    SVB_CONSTEXPR series_vector_base() SVB_NOEXCEPT = default;
    SVB_CONSTEXPR series_vector_base( size_type _count, const T& _value ) SVB_NOEXCEPT
        : m_array( _count, _value ) {}
#if __cplusplus >= CPP14LEVEL
    SVB_CONSTEXPR explicit series_vector_base( size_type _count )
        : m_array( _count ) {}
#else
    explicit series_vector_base( size_type _count )
        : m_array( _count ) {}
#endif
    template< typename InputIt >
    SVB_CONSTEXPR series_vector_base( InputIt _first, InputIt _last )
        : m_array( _first, _last ) {}
    SVB_CONSTEXPR series_vector_base( const series_vector_base& _other )
        : m_array( _other.m_array ) {}
    SVB_CONSTEXPR series_vector_base( series_vector_base&& ) SVB_NOEXCEPT = default; 
    SVB_CONSTEXPR series_vector_base( std::initializer_list<T> _init )
        : m_array( _init ) {}

    SVB_CONSTEXPR ~series_vector_base() = default;

    SVB_CONSTEXPR series_vector_base& operator=( const series_vector_base& ) = default; 
    SVB_CONSTEXPR series_vector_base& operator=( series_vector_base&& ) SVB_NOEXCEPT = default; 
    SVB_CONSTEXPR series_vector_base& operator=( std::initializer_list<T> _init )
    {
        m_array = std::vector<T>( _init );
    }

    SVB_CONSTEXPR void assign( size_type _count, const T& _value )
    {
        m_array.assign( _count, _value );
    }
    template< typename InputIt >
    SVB_CONSTEXPR void assign( InputIt _first, InputIt _last )
    {
        m_array.assign( _first, _last );
    }
    SVB_CONSTEXPR void assign( std::initializer_list<T> _init )
    {
        m_array.assign( _init );
    }

    SVB_CONSTEXPR reference at( size_type _pos )
    {
        return m_array.at( _pos );
    }
    SVB_CONSTEXPR const_reference at( size_type _pos ) const
    {
        return m_array.at( _pos );
    }

    SVB_CONSTEXPR reference operator[]( size_type _pos )
    {
        return m_array[ _pos ];
    }
    SVB_CONSTEXPR const_reference operator[]( size_type _pos ) const
    {
        return m_array[ _pos ];
    }

    SVB_CONSTEXPR reference front()
    {
        return m_array.front();
    }
    SVB_CONSTEXPR const_reference front() const
    {
        return m_array.front();
    }

    SVB_CONSTEXPR reference back()
    {
        return m_array.back();
    }
    SVB_CONSTEXPR const_reference back() const
    {
        return m_array.back();
    }

    SVB_CONSTEXPR T* data() noexcept
    {
        return m_array.data();
    }
    SVB_CONSTEXPR const T* data() const noexcept
    {
        return m_array.data();
    }

    SVB_CONSTEXPR iterator begin() noexcept
    {
        return m_array.begin();
    }
    SVB_CONSTEXPR const_iterator begin() const noexcept
    {
        return m_array.begin();
    }
    SVB_CONSTEXPR const_iterator cbegin() const noexcept
    {
        return m_array.cbegin();
    }

    SVB_CONSTEXPR iterator end() noexcept
    {
        return m_array.end();
    }
    SVB_CONSTEXPR const_iterator end() const noexcept
    {
        return m_array.end();
    }
    SVB_CONSTEXPR const_iterator cend() const noexcept
    {
        return m_array.cend();
    }

    SVB_CONSTEXPR reverse_iterator rbegin() noexcept
    {
        return m_array.rbegin();
    }
    SVB_CONSTEXPR const_reverse_iterator rbegin() const noexcept
    {
        return m_array.rbegin();
    }
    SVB_CONSTEXPR const_reverse_iterator crbegin() const noexcept
    {
        return m_array.crbegin();
    }

    SVB_CONSTEXPR reverse_iterator rend() noexcept
    {
        return m_array.rend();
    }
    SVB_CONSTEXPR const_reverse_iterator rend() const noexcept
    {
        return m_array.rend();
    }
    SVB_CONSTEXPR const_reverse_iterator crend() const noexcept
    {
        return m_array.crend();
    }

#if __cplusplus >= CPP20LEVEL
    [[nodiscard]] constexpr bool empty() const noexcept
#else
    bool empty() const noexcept
#endif
    {
        return m_array.empty();
    }
    SVB_CONSTEXPR size_type size() const noexcept
    {
        return m_array.size();
    }
    SVB_CONSTEXPR size_type max_size() const noexcept
    {
        return m_array.max_size();
    }
    SVB_CONSTEXPR void reserve( size_type _newsize )
    {
        m_array.reserve( _newsize );
    }
    SVB_CONSTEXPR size_type capacity() const noexcept
    {
        return m_array.capacity();
    }
    SVB_CONSTEXPR void shrink_to_fit()
    {
        m_array.shrink_to_fit();
    }
    SVB_CONSTEXPR void clear() noexcept
    {
        m_array.clear();
    }

    SVB_CONSTEXPR iterator insert( const_iterator _pos, const T& _value )
    {
        return m_array.insert( _pos, _value );
    }
    SVB_CONSTEXPR iterator insert( const_iterator _pos, T&& _value )
    {
        return m_array.insert( _pos, std::move( _value ) );
    }
    SVB_CONSTEXPR iterator insert( const_iterator _pos, size_type _count, const T& _value )
    {
        return m_array.insert( _pos, _count,  _value );
    }
    template< typename InputIt >
    SVB_CONSTEXPR iterator insert( const_iterator _pos, InputIt _first, InputIt _last )
    {
        return m_array.insert( _pos, _first, _last );
    }
    SVB_CONSTEXPR iterator insert( const_iterator _pos, std::initializer_list<T> _init )
    {
        return m_array.insert( _pos, _init );
    }

    template< class... Args >
    SVB_CONSTEXPR iterator emplace( const_iterator _pos, Args&&... _args )
    {
        return m_array.emplace( _pos, std::forward( _args... ) );
    }

    SVB_CONSTEXPR iterator erase( const_iterator _pos )
    {
        return m_array.erase( _pos );
    }
    SVB_CONSTEXPR iterator erase( const_iterator _first, const_iterator _last )
    {
        return m_array.erase( _first, _last );
    }

    SVB_CONSTEXPR void push_back( const T& _value )
    {
        m_array.push_back( _value );
    }
    SVB_CONSTEXPR void push_back( T&& _value )
    {
        m_array.push_back( std::move( _value ) );
    }
#if __cplusplus >= CPP17LEVEL
    template< typename... Args > 
    SVB_CONSTEXPR reference emplace_back( Args&&... args )
    {
        return m_array.emplace_back( std::forward( args... ) );
    }
#else
    template< typename... Args > 
    void emplace_back( Args&&... args )
    {
        m_array.emplace_back( std::forward( args... ) );
    }
#endif
    SVB_CONSTEXPR void pop_back()
    {
        m_array.pop_back();
    }
    SVB_CONSTEXPR void resize( size_type _newsize ) 
    { 
        m_array.resize( _newsize ); 
    }
    SVB_CONSTEXPR void resize( size_type _count, const value_type& _value )
    {
        m_array.resize( _count, _value ); 
    }
    SVB_CONSTEXPR void swap( series_vector_base<T>& other ) SVB_NOEXCEPT
    {
        swap( m_array, other.m_array );
    }

    template< class _T, class _Alloc >
    friend SVB_CONSTEXPR bool operator==( const std::vector<_T,_Alloc>& _left, 
                                          const std::vector<_T,_Alloc>& _right );
    template< class _T, class _Alloc >
    friend SVB_CONSTEXPR bool operator!=( const std::vector<_T,_Alloc>& _left, 
                                          const std::vector<_T,_Alloc>& _right );
    template< class _T, class _Alloc >
    friend SVB_CONSTEXPR bool operator<( const std::vector<_T,_Alloc>& _left, 
                                         const std::vector<_T,_Alloc>& _right );
    template< class _T, class _Alloc >
    friend SVB_CONSTEXPR bool operator<=( const std::vector<_T,_Alloc>& _left, 
                                          const std::vector<_T,_Alloc>& _right );
    template< class _T, class _Alloc >
    friend SVB_CONSTEXPR bool operator>( const std::vector<_T,_Alloc>& _left, 
                                         const std::vector<_T,_Alloc>& _right );
    template< class _T, class _Alloc >
    friend SVB_CONSTEXPR bool operator>=( const std::vector<_T,_Alloc>& _left, 
                                          const std::vector<_T,_Alloc>& _right );

private:
    std::vector<T> m_array;
};

template< typename T, typename Allocator >
SVB_CONSTEXPR bool operator==( const series_vector_base<T>& _left, 
                               const series_vector_base<T>& _right )
{
    return _left.m_array == _right.m_array;
}

template< typename T, typename Allocator >
SVB_CONSTEXPR bool operator!=( const series_vector_base<T>& _left, 
                               const series_vector_base<T>& _right )
{
    return _left.m_array != _right.m_array;
}

template< typename T, typename Allocator >
SVB_CONSTEXPR bool operator<( const series_vector_base<T>& _left, 
                              const series_vector_base<T>& _right )
{
    return _left.m_array < _right.m_array;
}

template< typename T, typename Allocator >
SVB_CONSTEXPR bool operator<=( const series_vector_base<T>& _left, 
                               const series_vector_base<T>& _right )
{
    return _left.m_array <= _right.m_array;
}

template< typename T, typename Allocator >
SVB_CONSTEXPR bool operator>( const series_vector_base<T>& _left, 
                              const series_vector_base<T>& _right )
{
    return _left.m_array > _right.m_array;
}

template< typename T, typename Allocator >
SVB_CONSTEXPR bool operator>=( const series_vector_base<T>& _left, 
                               const series_vector_base<T>& _right )
{
    return _left.m_array >= _right.m_array;
}


template<typename T>
class series_vector : public series_vector_base<T>
{
public:
    series_vector() = default;

    std::unique_ptr<iseries_vector> unique() const override
    {
        auto out = std::make_unique<series_vector<T>>();
        std::unordered_set<T> bag;
        for ( const auto &elem : *this ) {
            if ( bag.find( elem ) == bag.end() ) {
                out->push_back( elem );
                bag.insert( elem );
            }
        }
        return out;
    }

    std::unique_ptr<iseries_vector> to_string() const override
    {
        auto out = std::make_unique<series_vector<std::string>>();
        out->reserve( this->size() );
        for ( const auto &elem : *this ) {
          std::stringstream ss;
          detail::stringify(ss, elem, true);
          out->push_back(ss.str());
        }
        return out;
    }

    std::unique_ptr<iseries_vector> clone() const override
    {
        return std::make_unique<series_vector<T>>( *this );
    }

    std::unique_ptr<iseries_vector> clone_empty() const override
    {
        return std::make_unique<series_vector<T>>();
    }
};

} // namespace mf


#endif // INCLUDED_miniframe_series_vector_h

