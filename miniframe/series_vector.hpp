
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
class sv_iterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = T;
    using pointer           = T*;  // or also value_type*
    using reference         = T&;  // or also value_type&u

    sv_iterator( T* ptr ) : m_curr( ptr ) {}

    operator sv_iterator< const T >() const
    {
        return sv_iterator< const T >{ m_curr };
    }

    operator T*() const
    {
        return m_curr;
    }

    difference_type operator-( const sv_iterator<T>& other ) const
    {
        return m_curr - other.m_curr;
    }

    sv_iterator<T> operator+( difference_type p ) const
    {
        return sv_iterator<T>{ m_curr + p };
    }

    sv_iterator<T> operator-( difference_type p ) const
    {
        return sv_iterator<T>{ m_curr - p };
    }

    void operator++() { m_curr++; }
    void operator--() { m_curr--; }
    void operator++(int) { ++m_curr; }
    void operator--(int) { --m_curr; }

    reference operator*() { return *m_curr; }
    pointer operator->() { return m_curr; }

protected:
    template< typename U >
    friend bool operator==( const sv_iterator<U>&, const sv_iterator<U>& );
    template< typename U >
    friend bool operator!=( const sv_iterator<U>&, const sv_iterator<U>& );

    T* m_curr;
};

template< typename T >
class reverse_sv_iterator : public sv_iterator<T>
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = T;
    using pointer           = T*;  // or also value_type*
    using reference         = T&;  // or also value_type&u

    using sv_iterator<T>::sv_iterator;

    difference_type operator-( const reverse_sv_iterator<T>& other ) const
    {
        return other.m_curr - this->m_curr;
    }

    sv_iterator<T> operator+( difference_type p ) const
    {
        return sv_iterator<T>{ this->m_curr - p };
    }

    sv_iterator<T> operator-( difference_type p ) const
    {
        return sv_iterator<T>{ this->m_curr + p };
    }

    void operator++() { this->m_curr--; }
    void operator--() { this->m_curr++; }
    void operator++(int) { --this->m_curr; }
    void operator--(int) { ++this->m_curr; }
};

template< typename T >
bool operator==( const sv_iterator<T>& l, const sv_iterator<T>& r )
{
    return l.m_curr == r.m_curr;
}

template< typename T >
bool operator!=( const sv_iterator<T>& l, const sv_iterator<T>& r )
{
    return l.m_curr != r.m_curr;
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
    using iterator          = sv_iterator< T >;
    using const_iterator    = sv_iterator< const T >;
    using reverse_iterator          = reverse_sv_iterator< T >;
    using const_reverse_iterator    = reverse_sv_iterator< const T >;

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

    virtual ~series_vector()
    {
        destroy( m_begin, m_end );
        free( m_begin );
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
        reserve( count );
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
        reserve( inend - inbegin );
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
        if ( newsize > capacity() ) {
            size_t n = pow_2( newsize );
            T* nbegin = static_cast<T*>( malloc ( n * sizeof( T ) ) );
            T* nend = placement_copy( m_begin, m_end, nbegin );
            clear();
            m_begin = nbegin;
            m_end = nend;
            m_max = m_begin + n;
        }
    }
    size_type capacity() const noexcept
    {
        return m_max - m_begin;
    }
    void clear() noexcept
    {
        destroy( m_begin, m_end );
        free( m_begin );
        m_begin = m_end = m_max = nullptr;
    }

    template< typename U >
    iterator insert( const_iterator cpos, U&& value )
    {
        auto offset = cpos - begin();
        reserve( size() + 1 ); // possibly invalidates pos
        iterator pos = begin() + offset; // Fix pos

        // Move existing items
        split_array( pos, 1 );

        // Splice in the value
        new( pos ) T{ std::forward<U>( value ) };
        return pos; 
    }
    iterator insert( const_iterator cpos, size_type count, const T& value )
    {
        if ( count == 0 )
            return remove_const( cpos );
        auto offset = cpos - begin();
        reserve( size() + count ); // possibly invalidates pos
        iterator pos = begin() + offset; // Fix pos

        // Move existing items
        split_array( pos, count );

        // Splice in the value
        iterator ocurr = pos;
        size_type i = 0;
        for ( ; i < count; ++i, ++ocurr ) {
            new( ocurr ) T{ value };
        }
        return pos; 
    }
    void debug_print()
    {
        std::cout << "debug_print():\n";
        for( auto it = begin(); it != end(); ++it ) {
            std::cout << &(*it) << ", " << *it << "\n";
        }
    }

    template< typename InputIt >
    iterator insert( const_iterator cpos, InputIt fst, InputIt lst )
    {
        if ( lst <= fst )
            return remove_const( cpos );
        auto offset = cpos - begin();
        size_t count = lst - fst;
        reserve( size() + count ); // possibly invalidates pos
        iterator pos = begin() + offset; // Fix pos

        // Move existing items
        split_array( pos, count );

        // Splice in [fst...lst)
        InputIt icurr = fst;
        iterator ocurr = pos;
        for ( ; icurr != lst; ++icurr, ++ocurr ) {
            new( ocurr ) T{ *icurr };
        }
        return pos; 
    }
    iterator insert( const_iterator pos, std::initializer_list<T> initlist )
    {
        return insert( pos, initlist.begin(), initlist.end() );
    }

    template< class... Args >
    iterator emplace( const_iterator cpos, Args&&... args )
    {
        size_type offset = cpos - begin();
        reserve( size() + sizeof...( args ) );
        iterator pos = begin() + offset; // Fix pos

        split_array( pos, sizeof...( args ) );

        emplace_impl( pos, std::forward<Args>(args)... );
        return pos;
    }

    iterator erase( const_iterator pos )
    {
        return erase( pos, pos+1 );
    }
    iterator erase( const_iterator cfst, const_iterator clst )
    {
        auto fst = remove_const( cfst );
        auto lst = remove_const( clst );
        if ( fst >= lst || fst >= end() )
            return fst;

        auto icurr = lst;
        auto ocurr = fst;
        auto count = lst - fst;
        auto nend = m_begin + (size() - count);
        for ( ; ocurr != m_end; ++ocurr, ++icurr ) {
            ocurr->~T();
            if ( icurr < m_end ) {
                new( ocurr ) T{ *icurr };
            }
        }
        m_end = nend;
        return fst;
    }

    void push_back( const T& value )
    {
        insert( end(), value );
    }
    void push_back( T&& value )
    {
        insert( end(), std::move( value ) );
    }
    template< typename... Args > 
    reference emplace_back( Args&&... args )
    {
        return *emplace( end(), std::forward<Args>(args)... );
    }
    void pop_back()
    {
        erase( end()-1 );
    }
    void resize( size_type newsize ) 
    { 
        if ( newsize > size() ) {
            if constexpr ( std::is_default_constructible<T>::value ) {
                reserve( newsize );
                for ( ; m_end < m_begin + newsize; ++m_end ) {
                    // Requires default ctor
                    new( m_end ) T{};
                }
            }
            else {
                throw std::logic_error{ "Type is not default contructible and cannot be resize'd larger!" };
            }
        }
        else {
            for ( T * curr = m_begin + newsize; curr != m_end; ++curr ) {
                curr->~T();
            }
            m_end = m_begin + newsize;
        }
    }
    void resize( size_type newsize, const value_type& value )
    {
        if ( newsize > size() ) {
            reserve( newsize );
            for ( ; m_end < m_begin + newsize; ++m_end ) {
                new( m_end ) T{ value };
            }
        }
        else {
            for ( T * curr = m_begin + newsize; curr != m_end; ++curr ) {
                curr->~T();
            }
            m_end = m_begin + newsize;
        }
    }
    void swap( series_vector<T>& other )
    {
        swap( m_begin, other.m_begin );
        swap( m_end, other.m_end );
        swap( m_max, other.m_max );
    }

private:

    void split_array( const_iterator pos, size_t count )
    {
        T* ricurr = m_end-1;
        T* riend = remove_const( pos )-1;
        T* rocurr = ricurr+count;
        for ( ; ricurr != riend; --ricurr, --rocurr ) {
            new( rocurr ) T{ *ricurr };
            ricurr->~T();
        }
        m_end += count;
    }

    template< class Arg >
    void emplace_impl( iterator pos, Arg&& arg )
    {
        new( pos ) T{ std::forward<T>( arg ) };
    }
    template< class ArgFirst, class... Args >
    void emplace_impl( iterator pos, ArgFirst&& argf, Args&& ... args )
    {
        new( pos ) T{ std::forward<T>( argf ) };
        emplace_impl( pos + 1, std::forward<Args>(args)... );
    }

    template< typename Iter >
    Iter placement_copy( Iter inbegin, Iter inend, Iter outbegin ) const
    {
        auto incurr = inbegin;
        auto outcurr = outbegin;
        for (; incurr != inend; ++incurr, ++outcurr ) {
            new( outcurr ) T{ *incurr };
        }
        return outcurr;
    }

    iterator remove_const( const_iterator it ) const
    {
        return iterator{ m_begin + ( &*it - m_begin ) };
    }

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

    template< typename Iter >
    void destroy( Iter begin, Iter end )
    {
        auto curr = begin;
        for ( ; curr != end; ++curr ) {
            curr->~T();
        }
    }

    T * m_begin;
    T * m_end;
    T * m_max;
};


template< typename T >
bool operator==( const series_vector<T>& left, 
                 const series_vector<T>& right )
{
    return (left.size() == right.size()) &&
        std::equal( left.begin(), left.end(), right.begin() );
}

template< typename T >
bool operator!=( const series_vector<T>& left, 
                 const series_vector<T>& right )
{
    return !( left == right );
}


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

