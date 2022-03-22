
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <iomanip>
#include <sys/ioctl.h>
#include <unistd.h>
#include <miniframe/base.hpp>
#include <miniframe/series.hpp>

using namespace std;

namespace mf
{

template< typename T >
ostream & operator<<( ostream& o, const series<T> & se )
{
    auto strvals = se.to_string();
    size_t maxwidth = 0;
    for ( auto& elemstr : strvals ) {
        maxwidth = std::max( maxwidth, elemstr.size() );
    }
    auto name = strvals.name();
    if ( name.size() > 0 ) { 
        o << strvals.name() << ": " << strvals.size() << " element Series\n";
    }
    else {
        o << strvals.size() << " element Series\n";
    }
    o << get_horzrule( maxwidth ) << "\n";
    for ( auto& elemstr : strvals ) {
        o << elemstr << "\n";
    }
    return o;
}

} // namespace mf

