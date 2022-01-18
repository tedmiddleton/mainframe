#ifndef INCLUDED_miniframe_series_h
#define INCLUDED_miniframe_series_h

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

#include "miniframe_base.hpp"
#include "miniframe_series_vector.hpp"
#include "miniframe_series_iterator.hpp"

namespace mf
{

class series
{
private:
    series() = default;

    template<typename T>
    friend series make_series( std::string _name );

    template<typename T>
    friend series make_series( std::string _name, std::vector<T> _data );

public:

    // types
    template<typename T>
    using iterator = typename std::vector<T>::iterator;
    template<typename T>
    using const_iterator = typename std::vector<T>::const_iterator;
    template<typename T>
    using reference = typename std::vector<T>::reference;
    template<typename T>
    using const_reference = typename std::vector<T>::const_reference;
    template<typename T>
    using pointer = typename std::vector<T>::pointer;
    template<typename T>
    using const_pointer	= typename std::vector<T>::const_pointer;

    // ctors
    series( series&& ) = default;
    series( const series& _other )
        : m_impl( _other.m_impl->clone() )
    {}
     
    // operator=
    series& operator=( series&& ) = default; 
    series& operator=( const series& _other ) 
    {
        std::unique_ptr<iseries_vector> s = _other.m_impl->clone();
        m_impl = std::move( s );
        return *this;
    }

    // assign
    
    // at
    template<typename T>
    T& at( size_t _n )
    {
        return std::dynamic_pointer_cast<series_vector<T>>(m_impl)->at( _n );
    }

    // operator=
    template<typename T>
    T& operator[]( size_t _n )
    {
        return std::dynamic_pointer_cast<series_vector<T>>(m_impl)[ _n ];
    }

    // front & back
    template<typename T>
    reference<T> front()
    {
        return std::dynamic_pointer_cast<series_vector<T>>(m_impl)->front();
    }

    // data
    template<typename T>
    T* data()
    {
        return std::dynamic_pointer_cast<series_vector<T>>(m_impl)->data();
    }

    // begin, cbegin, end, cend
    template<typename T>
    iterator<T> begin()
    {
        return std::dynamic_pointer_cast<series_vector<T>>(m_impl)->begin();
    }
    template<typename T>
    const_iterator<T> begin() const
    {
        return std::dynamic_pointer_cast<const series_vector<T>>(m_impl)->begin();
    }
    template<typename T>
    const_iterator<T> cbegin() const
    {
        return std::dynamic_pointer_cast<const series_vector<T>>(m_impl)->cbegin();
    }
    template<typename T>
    iterator<T> end()
    {
        return std::dynamic_pointer_cast<series_vector<T>>(m_impl)->end();
    }
    template<typename T>
    const_iterator<T> end() const
    {
        return std::dynamic_pointer_cast<const series_vector<T>>(m_impl)->end();
    }
    template<typename T>
    const_iterator<T> cend() const
    {
        return std::dynamic_pointer_cast<const series_vector<T>>(m_impl)->cend();
    }

    // rbegin, crbegin, rend, crend
    template<typename T>
    iterator<T> rbegin()
    {
        return std::dynamic_pointer_cast<series_vector<T>>(m_impl)->rbegin();
    }
    template<typename T>
    const_iterator<T> rbegin() const
    {
        return std::dynamic_pointer_cast<const series_vector<T>>(m_impl)->rbegin();
    }
    template<typename T>
    const_iterator<T> crbegin() const
    {
        return std::dynamic_pointer_cast<const series_vector<T>>(m_impl)->crbegin();
    }
    template<typename T>
    iterator<T> rend()
    {
        return std::dynamic_pointer_cast<series_vector<T>>(m_impl)->rend();
    }
    template<typename T>
    const_iterator<T> rend() const
    {
        return std::dynamic_pointer_cast<const series_vector<T>>(m_impl)->rend();
    }
    template<typename T>
    const_iterator<T> crend() const
    {
        return std::dynamic_pointer_cast<const series_vector<T>>(m_impl)->crend();
    }

    // capacity & modifiers
    bool empty() const
    {
        return m_impl->empty();
    }
    size_t size() const 
    { 
        return m_impl->size(); 
    }
    size_t max_size() const 
    { 
        return m_impl->max_size(); 
    }
    void reserve( size_t _size ) 
    { 
        m_impl->reserve( _size ); 
    }
    size_t capacity() const
    { 
        return m_impl->capacity(); 
    }
    void shrink_to_fit()
    {
        m_impl->shrink_to_fit();
    }
    void clear()
    {
        m_impl->clear();
    }

    // insert & emplace
    template<typename T>
    iterator<T> insert( const_iterator<T> _pos, const T& _value )
    {
        auto s = std::dynamic_pointer_cast<series_vector<T>>(m_impl);
        return s->insert( _pos, _value );
    }
    template<typename T>
    iterator<T> insert( const_iterator<T> _pos, T&& _value )
    {
        auto s = std::dynamic_pointer_cast<series_vector<T>>(m_impl);
        return s->insert( _pos, std::move( _value ));
    }
    template<typename T>
    iterator<T> insert( const_iterator<T> _pos, size_t _count, const T& _value )
    {
        auto s = std::dynamic_pointer_cast<series_vector<T>>(m_impl);
        return s->insert( _pos, _count, _value );
    }
    template< typename InputIt, typename T >
    iterator<T> insert( const_iterator<T> _pos, InputIt _first, InputIt _last )
    {
        auto s = std::dynamic_pointer_cast<series_vector<T>>(m_impl);
        s->insert( _pos, _first, _last );
    }
    template<typename T>
    iterator<T> insert( const_iterator<T> _pos, std::initializer_list<T> _init )
    {
        auto s = std::dynamic_pointer_cast<series_vector<T>>(m_impl);
        s->insert( _pos, _init );
    }
    template< typename T, class... Args >
    iterator<T> emplace( const_iterator<T> _pos, Args&&... _args )
    {
        auto s = std::dynamic_pointer_cast<series_vector<T>>(m_impl);
        s->emplace( _pos, std::forward( _args... ) );
    }

    // erase
    template<typename T>
    iterator<T> erase( const_iterator<T> _pos )
    {
        return std::dynamic_pointer_cast<series_vector<T>>(m_impl)->erase( _pos );
    }
    template<typename T>
    iterator<T> erase( const_iterator<T> _first, const_iterator<T> _last )
    {
        return std::dynamic_pointer_cast<series_vector<T>>(m_impl)->erase( _first, _last );
    }

    // push_back, emplace_back, pop_back
    template<typename T>
    void push_back( const T& _value )
    {
        std::dynamic_pointer_cast<series_vector<T>>(m_impl)->push_back( _value );
    }
    template<typename T>
    void push_back( T&& _value )
    {
        std::dynamic_pointer_cast<series_vector<T>>(m_impl)->push_back( std::move( _value ) );
    }
    template< typename T, typename... Args > 
    reference<T> emplace_back( Args&&... _args )
    {
        auto s = std::dynamic_pointer_cast<series_vector<T>>(m_impl);
        return s->emplace_back( std::forward( _args... ) );
    }
    void pop_back()
    {
        m_impl->pop_back();
    }

    // resize
    void resize( size_t _newsize ) 
    { 
        return m_impl->resize( _newsize ); 
    }
    template<typename T>
    void resize( size_t _newsize, const T& _value ) 
    { 
        auto s = std::dynamic_pointer_cast<series_vector<T>>(m_impl);
        return s->resize( _newsize, _value );
    }

    // swap
 
    std::string name() const 
    { 
        return m_name; 
    }
    
    void set_name( std::string _name ) 
    { 
        m_name = _name; 
    }
    
    series unique() const
    { 
        series out;
        out.m_name = m_name;
        out.m_impl = m_impl->unique();
        return out;
    }

    series to_string() const
    {
        series out;
        out.m_name = m_name;
        out.m_impl = m_impl->to_string();
        return out;
    }

    series clone_empty() const
    {
        series out;
        out.m_name = m_name;
        out.m_impl = m_impl->clone_empty();
        return out;
    }

    void unref()
    {
        auto n = m_impl->clone();
        m_impl = std::shared_ptr<iseries_vector>{ std::move( n ) };
    }

    friend std::ostream& operator<<( std::ostream&, const series& );

private:

    std::string m_name;
    std::shared_ptr<iseries_vector> m_impl;
};

template<typename T>
series make_series( std::string _name )
{
    series out;
    out.m_name = _name;
    out.m_impl = std::make_unique<series_vector<T>>();
    return out;
}

template<typename T>
series make_series( std::string _name, std::vector<T> _data )
{
    series out;
    out.m_name = _name;
    out.m_impl = std::make_unique<series_vector<T>>( _data );
    return out;
}

} // namespace mf


#endif // INCLUDED_miniframe_series_h

