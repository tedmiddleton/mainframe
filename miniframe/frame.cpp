
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <iomanip>
#include <sys/ioctl.h>
#include <unistd.h>
#include <set>
#include <miniframe/frame.hpp>
#include <miniframe/series.hpp>
#include <miniframe/base.hpp>

using namespace std;

//118×12 DataFrame
// Row │ date        backlog_of_orders  customers_inventories  employment  imports  inventories  new_export_orders  new_orders  overall  prices  production  supplier_deliveries
//     │ Date…       Int64              Int64                  Int64       Int64    Int64        Int64              Int64       Int64    Int64   Int64       Int64
//─────┼─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
//   1 │ 2011-09-01                 -1                      1           1        1            1                 -1          -1        1       1          -1                   -1
//   2 │ 2011-11-01                 -1                      1           1       -1            1                 -1          -1       -1      -1          -1                    1
//   3 │ 2011-12-01                  1                     -1           1        1            1                  1           1        1      -1           1                    1
//  ⋮  │     ⋮               ⋮                    ⋮                ⋮          ⋮          ⋮               ⋮              ⋮          ⋮       ⋮         ⋮                ⋮
// 117 │ 2021-06-01                  1                     -1           1        1            1                  1           1        1       1           1                   -1
// 118 │ 2021-07-01                  1                     -1           1        1           -1                  1           1        1       1           1                   -1
//                                                                                                                                                               113 rows omitted

namespace mf
{

bool row_iterator::operator!=( const row_iterator& other ) const
{
    return other.m_idx != m_idx || &(other.m_frame) != &m_frame; 
}

void row_iterator::operator++()
{
    m_idx++;
}

row row_iterator::operator*()
{
    return row{ m_frame, m_idx };
}

frame frame::clone_empty() const
{
    frame out;
    for ( auto& col : m_columns ) {
        out.add_series( col.clone_empty() );
    }
    return out;
}

void frame::unref()
{
    for ( auto& col : m_columns ) {
        col.unref();
    }
}

row_iterator frame::begin()
{
    return row_iterator{ *this, 0 };
}

row_iterator frame::end()
{
    return row_iterator{ *this, size() };
}

const_row_iterator frame::begin() const
{
    return const_row_iterator{ *this, 0 };
}

const_row_iterator frame::end() const
{
    return const_row_iterator{ *this, size() };
}

const_row_iterator frame::cbegin() const
{
    return const_row_iterator{ *this, 0 };
}

const_row_iterator frame::cend() const
{
    return const_row_iterator{ *this, size() };
}


void frame::add_series( const series& series )
{
    m_columns.push_back( series );
    reconcile_size();
}

void frame::drop_series( string _name )
{
    for ( auto i = 0U; i < m_columns.size(); ) {
        auto colname = m_columns[ i ].name();
        if ( colname == _name ) {
            auto it = m_columns.begin() + i;
            m_columns.erase( it );
        }
        else {
            ++i;
        }
    }
}

std::vector<std::string> frame::column_names() const
{
    std::vector<std::string> out;
    for ( auto const & col : m_columns ) {
        out.push_back( col.name() );
    }
    return out;
}

void frame::set_column_names( const std::vector<std::string>& names )
{
    for ( size_t i=0; i < names.size(); ++i ) {

        m_columns.at( i ).set_name( names.at( i ) );
    }
}

frame frame::to_string() const
{
    frame strframe;
    for ( auto& col : m_columns ) {
        strframe.add_series( col.to_string() );
    }
    return strframe;
}

size_t frame::size() const
{
    if ( m_columns.size() == 0 ) {
        return 0;
    }
    return m_columns.at( 0 ).size();
}

size_t frame::num_columns() const 
{
    return m_columns.size();
}

void frame::reconcile_size()
{
    size_t maxlength = 0;
    for ( auto& col : m_columns ) {
        maxlength = std::max( col.size(), maxlength );
    }
    for ( auto& col : m_columns ) {
        col.resize( maxlength );
    }
}

std::ostream & operator<<( std::ostream& o, const frame & mf )
{
    frame strframe = mf.to_string();
    //if ( use_termio() ) {
    //}
    //else {
    //}
   
    auto num_columns = strframe.num_columns();
    vector<size_t> widths; 

    for ( auto& col : strframe.m_columns ) {
        auto colname = col.name();
        size_t maxwidth = colname.size();
        for ( auto it=col.begin<string>(); it!=col.end<string>(); ++it ) {
            maxwidth = std::max( it->size(), maxwidth );
        }
        widths.push_back( maxwidth );
    }

    auto num_rows = strframe.size();
    auto gutter_width = 
        num_rows > 0 ? std::ceil( std::log10( num_rows ) ) + 1 : 1;
    o << left;
    o << mf.num_columns() << "x" << mf.size() << " MiniFrame\n";
    o << setw( gutter_width ) << "Row" << "|";
    auto colnames = mf.column_names();
    for ( size_t c = 0; c < num_columns; ++c ) {
        auto width = widths[ c ] + 1;
        auto name = colnames[ c ];
        o << setw( width ) << name;
    }
    o << endl;

    o << get_horzrule( gutter_width ) << "|";
    for ( size_t c = 0; c < num_columns; ++c ) {
        auto width = widths[ c ] + 1;
        o << get_horzrule( width );
    }
    o << endl;

    for ( size_t r = 0; r < num_rows; ++r ) {
        o << setw( gutter_width ) << r << "|" ;
        for ( size_t c = 0; c < num_columns; ++c ) {
            //auto width = widths[ c ] + 1;
            //o << setw( width ) << strframe.cell_at<std::string>( r, c );
        }
        o << endl;
    }

    return o;
}

} // namespace mf

