#include <iostream>
#include <iomanip>
#include <sys/ioctl.h>
#include <unistd.h>
#include "miniframe_base.hpp"
#include "miniframe_series.hpp"

using namespace std;

namespace mf
{

ostream & operator<<( ostream& o, const series & se )
{
    auto strvals = se.to_string();
    size_t maxwidth = 0;
    for ( auto it=strvals.begin<string>(); it!=strvals.end<string>(); ++it ) {
        maxwidth = std::max( maxwidth, it->size() );
    }
    auto name = strvals.name();
    if ( name.size() > 0 ) { 
        o << strvals.name() << ": " << strvals.size() << " element Series\n";
    }
    else {
        o << strvals.size() << " element Series\n";
    }
    o << get_horzrule( maxwidth ) << "\n";
    for ( auto it=strvals.begin<string>(); it!=strvals.end<string>(); ++it ) {
        o << *it << "\n";
    }
    return o;
}

} // namespace mf

