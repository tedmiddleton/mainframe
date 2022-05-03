
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_series_h
#define INCLUDED_mainframe_series_h

#include <cmath>
#include <type_traits>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <string>
#include <list>
#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <ostream>
#include <iostream>
#include <sstream>
#include <variant>

#include "mainframe/base.hpp"
#include "mainframe/missing.hpp"
#include "mainframe/series_vector.hpp"

namespace mf
{

template< typename T >
class series;

// This is an untyped version of series
class useries
{ 
public:
    useries() = default;
    useries( const useries& ) = default;
    useries& operator=( const useries& ) = default;

    template< typename T >
    useries( const series<T>& s )
        : m_name( s.m_name )
        , m_data( std::dynamic_pointer_cast<iseries_vector>(s.m_sharedvec) )
    {}

    template< typename T >
    operator series<T>() const
    {
        series<T> s;
        s.m_name = m_name;
        s.m_sharedvec = std::dynamic_pointer_cast<series_vector<T>>( m_data );
        return s;
    }

    virtual const std::string& name() const { return m_name; }
    virtual void set_name( const std::string& name ) { m_name = name; }
    virtual size_t size() const { return m_data->size(); };

private:
    std::string m_name;
    std::shared_ptr<iseries_vector> m_data;
};

template< typename T >
class series
{
public:
    // types
    using value_type        = T;
    using size_type         = typename series_vector<T>::size_type;
    using difference_type   = typename series_vector<T>::difference_type;
    using reference         = value_type&;
    using const_reference   = const value_type&;
    using pointer           = value_type*;
    using const_pointer	    = const value_type*;
    using iterator          = typename series_vector<T>::iterator;
    using const_iterator    = typename series_vector<T>::const_iterator;
    using reverse_iterator  = typename series_vector<T>::reverse_iterator;
    using const_reverse_iterator = typename series_vector<T>::const_reverse_iterator;

    // ctors
    series() 
        : m_sharedvec( std::make_shared< series_vector<T> >() )
    {}

    series( size_type count, const T& value )
        : m_sharedvec( std::make_shared< series_vector<T> >( count, value ) )
    {}

    explicit series( size_type count )
        : m_sharedvec( std::make_shared< series_vector<T> >( count ) )
    {}

    template< typename InputIt >
    series( InputIt f, InputIt l )
        : m_sharedvec( std::make_shared< series_vector<T> >( f, l ) )
    {}

    series( const series& other ) = default;

    series( series&& other )
        : m_name( std::move( other.m_name ) )
        , m_sharedvec( std::move( other.m_sharedvec ) )
    {
        // Don't leave other with nullptr
        other.m_sharedvec = std::make_shared< series_vector<T> >();
    }

    series( std::initializer_list<T> init )
        : m_sharedvec( std::make_shared< series_vector<T> >( init ) )
    {}

    virtual ~series() = default;

    // operator=
    series& operator=( series&& other ) 
    {
        m_name = other.m_name;
        m_sharedvec = std::move( other.m_sharedvec );
        other.m_sharedvec = std::make_shared< series_vector<T> >();
        return *this;
    }

    series& operator=( const series& ) = default;

    series& operator=( std::initializer_list<T> init )
    {
        m_sharedvec = std::make_shared< series_vector<T> >( init );
        return *this;
    }

    template<typename U = T,
        std::enable_if_t<is_missing<U>::value, bool> = true>
    series<T> allow_missing() const
    {
        return *this;
    }

    template<typename U = T,
        std::enable_if_t<!is_missing<U>::value, bool> = true>
    series<mi<T>> allow_missing() const
    {
        series<mi<T>> os;
        for ( auto& e : *m_sharedvec ) {
            os.push_back( e );
        }
        os.set_name( name() );
        return os;
    }

    // This requires default-construction. Can we do better?
    template<typename U = T,
        std::enable_if_t<is_missing<U>::value && 
            std::is_default_constructible<typename U::value_type>::value, 
        bool> = true>
    series<typename U::value_type> disallow_missing() const
    {
        using V = typename T::value_type;
        series<V> s;
        for ( auto& e : *m_sharedvec ) {
            if ( e.has_value() ) {
                s.push_back( *e );
            }
            else {
                s.push_back( V{} );
            }
        }
        s.set_name( name() );
        return s;
    }

    template<typename U = T,
        std::enable_if_t<!is_missing<U>::value, bool> = true>
    series<T> disallow_missing() const
    {
        return *this;
    }

    // assign
    void assign( size_type count, const T& value )
    {
        m_sharedvec = std::make_shared< series_vector<T> >( count, value );
    }

    template< typename InputIt >
    void assign( InputIt inbegin, InputIt inend )
    {
        m_sharedvec = std::make_shared< series_vector<T> >( inbegin, inend );
    }

    void assign( std::initializer_list<T> init )
    {
        m_sharedvec = std::make_shared< series_vector<T> >( init );
    }
    
    // at
    reference at( size_type n )
    {
        unref();
        return m_sharedvec->at( n );
    }
    const_reference at( size_type n ) const
    {
        return m_sharedvec->at( n );
    }

    // operator[]
    // It seems like this should really be an unref-ing operation, otherwise 
    // there's the possibility that the user will damage multiple series.  
    reference operator[]( size_type n )
    {
        unref();
        return (*m_sharedvec)[ n ];
    }
    const_reference operator[]( size_type n ) const
    {
        return (*m_sharedvec)[ n ];
    }

    // front & back
    reference front()
    {
        unref();
        return m_sharedvec->front();
    }
    const_reference front() const
    {
        return m_sharedvec->front();
    }

    reference back()
    {
        unref();
        return m_sharedvec->back();
    }
    const_reference back() const
    {
        return m_sharedvec->back();
    }

    // data
    T* data()
    {
        unref();
        return m_sharedvec->data();
    }

    // begin, cbegin, end, cend
    iterator begin()
    {
        unref();
        return m_sharedvec->begin();
    }
    const_iterator begin() const
    {
        return m_sharedvec->begin();
    }
    const_iterator cbegin() const
    {
        return m_sharedvec->cbegin();
    }
    iterator end()
    {
        unref();
        return m_sharedvec->end();
    }
    const_iterator end() const
    {
        return m_sharedvec->end();
    }
    const_iterator cend() const
    {
        return m_sharedvec->cend();
    }

    // rbegin, crbegin, rend, crend
    reverse_iterator rbegin()
    {
        unref();
        return m_sharedvec->rbegin();
    }
    const_reverse_iterator rbegin() const
    {
        return m_sharedvec->rbegin();
    }
    const_reverse_iterator crbegin() const
    {
        return m_sharedvec->crbegin();
    }
    reverse_iterator rend()
    {
        unref();
        return m_sharedvec->rend();
    }
    const_reverse_iterator rend() const
    {
        return m_sharedvec->rend();
    }
    const_reverse_iterator crend() const
    {
        return m_sharedvec->crend();
    }

    // capacity & modifiers
    bool empty() const
    {
        return m_sharedvec->empty();
    }
    size_type size() const 
    { 
        return m_sharedvec->size(); 
    }
    size_type max_size() const 
    { 
        return m_sharedvec->max_size(); 
    }
    void reserve( size_type _size ) 
    { 
        m_sharedvec->reserve( _size ); 
    }
    size_type capacity() const
    { 
        return m_sharedvec->capacity(); 
    }
    void shrink_to_fit()
    {
        m_sharedvec->shrink_to_fit();
    }
    void clear()
    {
        unref();
        m_sharedvec->clear();
    }

    // insert & emplace
    iterator insert( const_iterator pos, const T& value )
    {
        if ( m_sharedvec->cbegin() <= pos && pos <= m_sharedvec->cend() ) {
            pos = unref( pos );
        }
        return m_sharedvec->insert( pos, value );
    }
    iterator insert( const_iterator pos, T&& value )
    {
        if ( m_sharedvec->cbegin() <= pos && pos <= m_sharedvec->cend() ) {
            pos = unref( pos );
        }
        return m_sharedvec->insert( pos, std::move( value ));
    }
    iterator insert( const_iterator pos, size_type count, const T& value )
    {
        if ( m_sharedvec->cbegin() <= pos && 
             pos <= m_sharedvec->cend() && 
             count > 0 ) {
            pos = unref( pos );
        }
        return m_sharedvec->insert( pos, count, value );
    }
    template< typename InputIt >
    iterator insert( const_iterator pos, InputIt first, InputIt last )
    {
        if ( m_sharedvec->cbegin() <= pos && 
             pos <= m_sharedvec->cend() &&
             last > first ) {
            pos = unref( pos );
        }
        return m_sharedvec->insert( pos, first, last );
    }
    iterator insert( const_iterator pos, std::initializer_list<T> init )
    {
        if ( m_sharedvec->cbegin() <= pos && 
             pos <= m_sharedvec->cend() &&
             !init.empty() ) {
            pos = unref( pos );
        }
        return m_sharedvec->insert( pos, init );
    }
    template< class... Args >
    iterator emplace( const_iterator pos, Args&&... args )
    {
        if ( m_sharedvec->cbegin() <= pos && pos <= m_sharedvec->cend() ) {
            pos = unref( pos );
        }
        return m_sharedvec->emplace( pos, std::forward<T>( args... ) );
    }

    // erase
    iterator erase( const_iterator pos )
    {
        if ( m_sharedvec->cbegin() <= pos && pos <= m_sharedvec->cend() ) {
            pos = unref( pos );
        }
        return m_sharedvec->erase( pos );
    }
    iterator erase( const_iterator first, const_iterator last )
    {
        if ( m_sharedvec->cbegin() <= first && 
             first < last && 
             last <= m_sharedvec->cend() ) {
            auto diff = last - first;
            first = unref( first );
            last = first + diff;
        }
        return m_sharedvec->erase( first, last );
    }

    // push_back, emplace_back, pop_back
    void push_back( const T& value )
    {
        unref();
        m_sharedvec->push_back( value );
    }
    void push_back( T&& value )
    {
        unref();
        m_sharedvec->push_back( std::move( value ) );
    }
    template< typename... Args > 
    reference emplace_back( Args&&... args )
    {
        unref();
        return m_sharedvec->emplace_back( std::forward<T>( args... ) );
    }
    void pop_back()
    {
        unref();
        m_sharedvec->pop_back();
    }

    // resize
    void resize( size_type newsize ) 
    { 
        unref();
        return m_sharedvec->resize( newsize ); 
    }
    void resize( size_type newsize, const T& value ) 
    { 
        unref();
        return m_sharedvec->resize( newsize, value );
    }

    // swap
 
    const std::string& name() const 
    { 
        return m_name; 
    }
    
    void set_name( const std::string& name )
    {
        m_name = name;
    }
    
    std::vector< std::string > to_string() const
    {
        std::vector< std::string > s;
        s.reserve( size() );

        for ( const T& t : *m_sharedvec ) {
            std::stringstream ss;
            ss << std::boolalpha;
            stringify( ss, t, true );
            s.push_back( ss.str() );
        }
        return s;
    }

    series unique() const
    { 
        series out;
        out.m_name = m_name;
        out.m_sharedvec = m_sharedvec->unique();
        return out;
    }

    size_t use_count() const
    {
        return m_sharedvec.use_count();
    }

    bool operator==( const series<T>& other ) const
    {
        return m_name == other.m_name &&
            *m_sharedvec == *(other.m_sharedvec);
    }

    bool operator!=( const series<T>& other ) const
    {
        return m_name != other.m_name || 
            *m_sharedvec != *(other.m_sharedvec);
    }

private:

    void unref()
    {
        if ( m_sharedvec.use_count() > 1 ) {
            std::shared_ptr<series_vector<T>> n = 
                std::make_shared<series_vector<T>>( *m_sharedvec );
            m_sharedvec = n;
        }
    }

    iterator unref( iterator it )
    {
        iterator newit = it;
        if ( m_sharedvec.use_count() > 1 ) {
            auto oldbegin = m_sharedvec->begin();
            std::shared_ptr<series_vector<T>> n = 
                std::make_shared<series_vector<T>>( *m_sharedvec );
            m_sharedvec = n;
            newit += (m_sharedvec->begin() - oldbegin);
        }
        return newit;
    }

    const_iterator unref( const_iterator it )
    {
        const_iterator newit = it;
        if ( m_sharedvec.use_count() > 1 ) {
            auto oldbegin = m_sharedvec->cbegin();
            std::shared_ptr<series_vector<T>> n = 
                std::make_shared<series_vector<T>>( *m_sharedvec );
            m_sharedvec = n;
            newit += (m_sharedvec->cbegin() - oldbegin);
        }
        return newit;
    }

    friend class useries;
    template< typename U >
    friend std::ostream& operator<<( std::ostream&, const series<U>& );

    std::string m_name;
    std::shared_ptr<series_vector<T>> m_sharedvec;
};

template< typename T >
std::ostream & operator<<( std::ostream& o, const series< T >& s )
{
    std::ios_base::fmtflags fl( o.flags() );

    std::vector<std::string> us = s.to_string();
    auto name = s.name();
    auto width = std::max( name.size(), get_max_string_length( us ) );
    const size_t num_rows = s.size();

    auto gutter_width = 
        num_rows > 0 ? std::ceil( std::log10( num_rows ) ) + 1 : 1;

    o << std::boolalpha;
    o << get_emptyspace( gutter_width ) << "| " << std::setw( width ) << name << "\n";
    o << get_horzrule( gutter_width ) << "|" << get_horzrule( width + 2 ) << "\n";

    for ( size_t rowind = 0; rowind < num_rows; ++rowind ) {
        std::string& datum = us[ rowind ];
        o << std::setw( gutter_width ) << rowind << "| " << std::setw( width ) << datum << "\n";
    }

    o.flags( fl );
    return o;
}

} // namespace mf


#endif // INCLUDED_mainframe_series_h

