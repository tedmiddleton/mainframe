
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

#include <miniframe/base.hpp>
#include <miniframe/expression.hpp>
#include <miniframe/series.hpp>

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
    void new_series( std::string _series_name )
    {
        auto s = make_series<T>( _series_name );
        m_columns.push_back( s ); 
        reconcile_size();
    }

    void add_series( const series& series )
    {
        m_columns.push_back( series );
        reconcile_size();
    }

    template< typename ..._Tps >
    void push_back( _Tps... _args )
    {
        _push_back( 0, _args... );
    }

    template< typename ..._Tps >
    std::tuple< _Tps... > row( size_t _row ) const
    {
        std::tuple< _Tps... > out;
        row_impl< _Tps... >( out, 0, _row );
        return out; 
    }

    template< typename _Func >
    frame rows( _Func func ) const
    {
        frame out = clone_empty();
        return out;
    }

    template< typename T >
    T& cell_at( size_t _row, size_t _col )
    {
        auto& col = column( _col );
        return col.at<T>( _row );
    }

    template< typename _Tp1, typename _Tp2, typename _Func >
    void map( std::string col1name, std::string col2name, _Func func )
    {
        auto& col1 = column( col1name );
        auto& col2 = column( col2name );
        size_t num = col1.size();
        for ( size_t i=0; i < num; ++i ) {
            func( col1.at<_Tp1>( i ), col2.at<_Tp2>( i ) );
        }
    }

    series& column( size_t _ind )
    {
        return m_columns.at( _ind );
    }

    const series& column( size_t _ind ) const
    {
        return m_columns.at( _ind );
    }

    series& column( std::string _colname )
    {
        for ( auto it = m_columns.begin(); it != m_columns.end(); ++it ) {
            auto colname = it->name();
            if ( colname == _colname ) {
                return *it;
            }
        }
        throw std::out_of_range{ 
            std::string{ "Can't find column '" } + _colname + std::string{ "'" } };
    }

    const series& column( std::string _colname ) const
    {
        for ( auto it = m_columns.begin(); it != m_columns.end(); ++it ) {
            auto colname = it->name();
            if ( colname == _colname ) {
                return *it;
            }
        }
        throw std::out_of_range{ 
            std::string{ "Can't find column '" } + _colname + std::string{ "'" } };
    }

    template< typename ..._Tps >
    frame columns( _Tps... selargs )
    {
        frame out;
        _columns<_Tps...>( out, 0, selargs... );
        return out;
    }

    void unref();

    void drop_series( std::string _name );

    std::vector<std::string> column_names() const;

    frame to_string() const;

    size_t size() const;

    size_t num_columns() const;

    friend std::ostream & operator<<( std::ostream& o, const frame & mf );

private:
    template< typename _FirstTp, typename... _Tps >
    void _columns( frame& _out, size_t _argnum, _FirstTp _first_arg, _Tps... _args )
    {
        _add_column( _out, _first_arg );
        _columns<_Tps...>( _out, _argnum+1, _args... );
    }

    template< typename _Tp >
    void _columns( frame& _out, size_t _argnum, _Tp _arg )
    {
        _add_column( _out, _arg );
    }

    void _add_column( frame& _out, std::string _colname )
    {
        series& series = column( _colname );
        _out.m_columns.push_back( series );
    }

    void _add_column( frame& _out, size_t _arg )
    {
        series& series = column( _arg );
        _out.m_columns.push_back( series );
    }

    template< typename _RemainTp, typename ..._OrgTps >
    void row_impl( std::tuple< _OrgTps... > & _tup, size_t _col, size_t _row ) const
    {
        const series& s = column( _col );
        std::get<_col>( _tup ) = s.at<_RemainTp>( _row );
    }

    template< typename _FirstTp, typename ..._RemainTps, typename ..._OrgTps >
    void row_impl( std::tuple< _OrgTps... >& _tup, size_t _col, size_t _row ) const
    {
        row_impl< _RemainTps..., _OrgTps... >( _col+1, _row );
        const series& s = column( _col );
        std::get<_col>( _tup ) = s.at<_FirstTp>( _row );
    }

    template< typename _FirstTp >
    void _push_back( size_t _col, _FirstTp _arg )
    {
        series& series = column( _col );
        series.push_back<_FirstTp>( _arg );
    }

    template< typename _FirstTp, typename ..._Tps >
    void _push_back( size_t _col, _FirstTp _arg, _Tps... _args )
    {
        series& series = column( _col );
        series.push_back<_FirstTp>( _arg );
        _push_back( _col+1, _args... );
    }

    void reconcile_size();

    using ColumnCollection = std::vector<series>;

    ColumnCollection m_columns;
};


} // namespace mf


#endif // INCLUDED_miniframe_h

