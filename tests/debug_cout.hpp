
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_debug_cout_h
#define INCLUDED_debug_cout_h

#include <cstdlib>
#include <iostream>

class DebugCout 
{
public:
    bool do_printout() const
    {
        bool ret = false;
#ifdef _WIN32
        char* var = nullptr;
        size_t num_elems = 0;
        if ( 0 == _dupenv_s( &var, &num_elems, "DOUT" ) ) {
            if ( var != nullptr ) {
                ret = true;
                free( var );
            }
        }
#else
        if ( std::getenv( "DOUT" ) ) {
            ret = true;
        }
#endif
        return ret;
    }

    template <class T>
    DebugCout &operator<<(const T &v)
    {
        if ( do_printout() ) {
            std::cout << v;
        }
        return *this;
    }

    using endlfunc = std::ostream&(*)(std::ostream&);
    DebugCout &operator<<( endlfunc f ) 
    { 
        if ( do_printout() ) {
            std::cout << *f; 
        }
        return *this; 
    }
};

#define dout DebugCout{}



#endif // INCLUDED_debug_cout_h

