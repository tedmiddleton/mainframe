
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

class frame 
{
public:
    frame() = default;
    frame( const frame& other );
    frame( frame&& ) = default;
    frame& operator=( const frame& other );
    virtual ~frame() = default;

    frame clone_empty() const;

    template< typename T >
    void new_series( std::string series_name )
    {
        auto s = make_series<T>( series_name );
        m_columns.push_back( s ); 
        reconcile_size();
    }

    void add_series( const series& series )
    {
        m_columns.push_back( series );
        reconcile_size();
    }

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
    frame rows( Func func ) const
    {
        frame out = clone_empty();
        return out;
    }

    template< typename T >
    T& cell_at( size_t rowidx, size_t colidx )
    {
        auto& col = column( colidx );
        return col.at<T>( rowidx );
    }

    template< typename Tp1, typename Tp2, typename Func >
    void map( std::string col1name, std::string col2name, Func func )
    {
        auto& col1 = column( col1name );
        auto& col2 = column( col2name );
        size_t num = col1.size();
        for ( size_t i=0; i < num; ++i ) {
            func( col1.at<Tp1>( i ), col2.at<Tp2>( i ) );
        }
    }

    series& column( size_t idx )
    {
        return m_columns.at( idx );
    }

    const series& column( size_t idx ) const
    {
        return m_columns.at( idx );
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

    void unref();

    void drop_series( std::string name );

    std::vector<std::string> column_names() const;

    frame to_string() const;

    size_t size() const;

    size_t num_columns() const;

    friend std::ostream & operator<<( std::ostream& o, const frame & mf );

private:
    template< typename FirstTp, typename... Tps >
    void columns_impl( frame& out, size_t argnum, FirstTp first_arg, Tps... args )
    {
        add_column_impl( out, first_arg );
        columns_impl<Tps...>( out, argnum+1, args... );
    }

    template< typename Tp >
    void columns_impl( frame& out, size_t argnum, Tp arg )
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
        series& series = column( arg );
        out.m_columns.push_back( series );
    }

    template< typename RemainTp, typename ...OrgTps >
    void row_impl( std::tuple< OrgTps... > & tup, size_t col, size_t row ) const
    {
        const series& s = column( col );
        std::get<col>( tup ) = s.at<RemainTp>( row );
    }

    template< typename FirstTp, typename ...RemainTps, typename ...OrgTps >
    void row_impl( std::tuple< OrgTps... >& tup, size_t col, size_t row ) const
    {
        row_impl< RemainTps..., OrgTps... >( col+1, row );
        const series& s = column( col );
        std::get<col>( tup ) = s.at<FirstTp>( row );
    }

    template< typename FirstTp >
    void push_back_impl( size_t col, FirstTp arg )
    {
        series& series = column( col );
        series.push_back<FirstTp>( arg );
    }

    template< typename FirstTp, typename ...Tps >
    void push_back_impl( size_t col, FirstTp arg, Tps... args )
    {
        series& series = column( col );
        series.push_back<FirstTp>( arg );
        push_back_impl( col+1, args... );
    }

    void reconcile_size();

    std::vector<series> m_columns;
};


} // namespace mf


#endif // INCLUDED_miniframe_h

