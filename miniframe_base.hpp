
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_miniframe_base_h
#define INCLUDED_miniframe_base_h
#include <ostream>
#include <optional>

namespace mf
{

void set_use_termio( bool enable );
bool use_termio();
int get_termwidth();
const char * get_horzrule( size_t num );

namespace detail
{

template<typename T>
auto stringify( std::ostream & o, const T & t, bool ) -> decltype( o << t, o )
{
    o << t;
    return o;
}

template<>
std::ostream& stringify( std::ostream & o, const char & t, bool );

template<>
std::ostream& stringify( std::ostream & o, const unsigned char & t, bool );

template<typename T>
std::ostream& stringify( std::ostream & o, const std::optional<T> & t, bool )
{
    if ( !t ) {
        o << "nullopt";
        return o;
    }
    else {
        return stringify( o, *t, true );
    }
}


template<typename T>
auto stringify( std::ostream & o, const T & t, int ) -> std::ostream &
{
    o << "opaque";
    return o;
}

} // namespace detail

} // namespace mf


#endif // INCLUDED_miniframe_base_h

