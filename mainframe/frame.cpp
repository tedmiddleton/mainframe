
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <iomanip>
#include <sys/ioctl.h>
#include <unistd.h>
#include <set>
#include <mainframe/frame.hpp>
#include <mainframe/series.hpp>
#include <mainframe/base.hpp>

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

template< typename ... Ts >
std::ostream & operator<<( std::ostream& o, const frame< Ts... >& )
{
    //auto num_columns = strframe.num_columns();
    //vector<size_t> widths; 

    //for ( auto& col : strframe.m_columns ) {
    //    auto colname = col.name();
    //    size_t maxwidth = colname.size();
    //    for ( auto it=col.begin<string>(); it!=col.end<string>(); ++it ) {
    //        maxwidth = std::max( it->size(), maxwidth );
    //    }
    //    widths.push_back( maxwidth );
    //}

    //auto num_rows = strframe.size();
    //auto gutter_width = 
    //    num_rows > 0 ? std::ceil( std::log10( num_rows ) ) + 1 : 1;
    //o << left;
    //o << mf.num_columns() << "x" << mf.size() << " mainframe\n";
    //o << setw( gutter_width ) << "Row" << "|";
    //auto colnames = mf.column_names();
    //for ( size_t c = 0; c < num_columns; ++c ) {
    //    auto width = widths[ c ] + 1;
    //    auto name = colnames[ c ];
    //    o << setw( width ) << name;
    //}
    //o << endl;

    //o << get_horzrule( gutter_width ) << "|";
    //for ( size_t c = 0; c < num_columns; ++c ) {
    //    auto width = widths[ c ] + 1;
    //    o << get_horzrule( width );
    //}
    //o << endl;

    //for ( size_t r = 0; r < num_rows; ++r ) {
    //    o << setw( gutter_width ) << r << "|" ;
    //    for ( size_t c = 0; c < num_columns; ++c ) {
    //        //auto width = widths[ c ] + 1;
    //        //o << setw( width ) << strframe.cell_at<std::string>( r, c );
    //    }
    //    o << endl;
    //}

    return o;
}

} // namespace mf

