
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
    template <class T>
    DebugCout &operator<<(const T &v)
    {
        if (std::getenv( "DOUT" )) {
            std::cout << v;
        }
        return *this;
    }

    using endlfunc = std::ostream&(*)(std::ostream&);
    DebugCout &operator<<( endlfunc f ) 
    { 
        if (std::getenv( "DOUT" )) {
            std::cout << *f; 
        }
        return *this; 
    }
};

#define dout DebugCout{}



#endif // INCLUDED_debug_cout_h

