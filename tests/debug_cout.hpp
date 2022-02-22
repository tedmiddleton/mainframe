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

