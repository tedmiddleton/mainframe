
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_miniframe_h
#define INCLUDED_miniframe_h

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <cmath>
#include <string>
#include <list>
#include <optional>
#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <ostream>
#include <sstream>
#include <variant>

#include "miniframe/base.hpp"
#include "miniframe/expression.hpp"
#include "miniframe/series.hpp"

namespace mf
{

template< typename ... Ts >
struct frame_iterator
{
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = ptrdiff_t;

    frame_iterator( std::tuple<typename series<Ts>::iterator ...>& its )
        : m_ptrs( its )
    {}

    frame_iterator( std::tuple<Series<Ts>...>& cols, int offset )
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

    FrameRow< Ts... >& operator*()
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

    std::tuple< series_iterator<Ts>... > m_ptrs;
};

template< typename ... Ts >
class frame 
{
public:
    using iterator = frame_iterator< Ts... >;

    frame() = default;
    frame( const frame& ) = default;
    frame( frame&& ) = default;
    frame& operator=( const frame& other ) = default;
    virtual ~frame() = default;

    iterator begin()
    {
        return iterator{ m_columns, 0 };
    }

    iterator end()
    {
        return iterator{ m_columns, static_cast<int>(size()) };
    }

    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    template< typename T >
    void new_series( std::string series_name )
    {
        auto s = make_series<T>( series_name );
        m_columns.push_back( s ); 
        reconcile_size();
    }

    void add_series( const series& series );

    void drop_series( std::string name );

    std::vector<std::string> column_names() const;

    void set_column_names( const std::vector<std::string>& names );

    frame to_string() const;

    size_t size() const;

    size_t num_columns() const;

    template< typename ...Tps >
    void push_back( Tps... args )
    {
        push_back_impl( 0, args... );
    }

    template< typename ...Tps >
    std::tuple< Tps... > row( size_t row ) const
    {
        std::tuple< Tps... > out;
        row_impl< Tps... >( out, 0, row );
        return out; 
    }

    template< typename Func >
    frame rows( Func expr ) const
    {
        (void)expr;
        frame out = clone_empty();
        for ( auto& col : m_columns ) {
            expr.bind_column( col );
        }
        return out;
    }

    series& column( std::string colname )
    {
        for ( auto it = m_columns.begin(); it != m_columns.end(); ++it ) {
            if ( it->name() == colname ) {
                return *it;
            }
        }
        throw std::out_of_range{ 
            std::string{ "Can't find column '" } + colname + std::string{ "'" } };
    }

    const series& column( std::string colname ) const
    {
        for ( auto it = m_columns.begin(); it != m_columns.end(); ++it ) {
            if ( it->name() == colname ) {
                return *it;
            }
        }
        throw std::out_of_range{ 
            std::string{ "Can't find column '" } + colname + std::string{ "'" } };
    }

    template< typename ...Tps >
    frame columns( Tps... selargs )
    {
        frame out;
        columns_impl<Tps...>( out, 0, selargs... );
        return out;
    }

    template< typename T >
    T& cell( size_t columnidx, size_t rowidx )
    {
        series& s = m_columns.at( columnidx );
        return s.at<T>( rowidx );
    }

    template< typename T >
    T& cell( const std::string& columnname, size_t rowidx )
    {
        series& s = column( columnname );
        return s.at<T>( rowidx );
    }

    friend std::ostream & operator<<( std::ostream& o, const frame & mf );

private:
    template< typename FirstTp, typename... Tps >
    void columns_impl( frame& out, size_t argnum, FirstTp first_arg, Tps... args )
    {
        add_column_impl( out, first_arg );
        columns_impl<Tps...>( out, argnum+1, args... );
    }

    template< typename Tp >
    void columns_impl( frame& out, size_t, Tp arg )
    {
        add_column_impl( out, arg );
    }

    void add_column_impl( frame& out, std::string colname )
    {
        series& series = column( colname );
        out.m_columns.push_back( series );
    }

    void add_column_impl( frame& out, size_t arg )
    {
        series& series = m_columns.at( arg );
        out.m_columns.push_back( series );
    }

    template< typename RemainTp, typename ...OrgTps >
    void row_impl( std::tuple< OrgTps... > & tup, size_t col, size_t row ) const
    {
        const series& s = m_columns.at( col );
        std::get<col>( tup ) = s.at<RemainTp>( row );
    }

    template< typename FirstTp, typename ...RemainTps, typename ...OrgTps >
    void row_impl( std::tuple< OrgTps... >& tup, size_t col, size_t row ) const
    {
        row_impl< RemainTps..., OrgTps... >( col+1, row );
        const series& s = m_columns.at( col );
        std::get<col>( tup ) = s.at<FirstTp>( row );
    }

    template< typename FirstTp, typename ... Tps >
    void add_empty_series( FirstTp, Tps ... args )
    {
        auto s = make_series<FirstTp>( "" );
        m_columns.push_back( s ); 
        add_empty_series<Tps...>( args... );
    }

    template< typename T >
    void add_empty_series( T )
    {
        auto s = make_series<T>( "" );
        m_columns.push_back( s ); 
    }

    template< typename FirstTp >
    void push_back_impl( size_t col, FirstTp arg )
    {
        // Special case - empty dataframe
        if ( m_columns.empty() ) {
            add_empty_series( arg );
        }
        series& series = m_columns.at( col );
        series.push_back<FirstTp>( arg );
    }

    template< typename FirstTp, typename ...Tps >
    void push_back_impl( size_t col, FirstTp arg, Tps... args )
    {
        // Special case - empty dataframe
        if ( m_columns.empty() ) {
            add_empty_series( arg, args... );
        }
        series& series = m_columns.at( col );
        series.push_back<FirstTp>( arg );
        push_back_impl( col+1, args... );
    }

    void reconcile_size();

    std::tuple<series<Ts>...> m_columns;
};

} // namespace mf


#endif // INCLUDED_miniframe_h

