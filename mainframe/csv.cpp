
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <iomanip>
#include <mainframe/frame.hpp>

using namespace std;


namespace mf
{

#if 0

static 
bool parse_csv_line( string line, vector<string>& elems )
{
    enum class STATE
    {
        START,
        ELEMCHAR,
        COMMA,
    } state = STATE::START;

    auto it = line.begin();
    auto last_comma = it;
    for ( auto it = line.begin(); it != line.end(); it++ )
    {
        auto c = *it;
        switch ( state ) {
        case STATE::START:
            if ( c == ',' ) {
                elems.emplace_back( "" );
                last_comma = it;
                state = STATE::COMMA;
            }
            else {
                state = STATE::ELEMCHAR;
            }
            break;

        case STATE::ELEMCHAR:
            if ( c == ',' ) {
                elems.emplace_back( last_comma+1, it );
                last_comma = it;
                state = STATE::COMMA;
            }
            break;

        case STATE::COMMA:
            if ( c == ',' ) {
                elems.emplace_back( "" );
                last_comma = it;
            }
            else {
                state = STATE::ELEMCHAR;
            }
            break;
        }
    }
    elems.emplace_back( last_comma, it );
    return true;
}


//static 
Frame Frame::from_csv( std::istream& csv, bool header_line )
{
    Frame frame;
    int lineno = 0;
    for ( string line; std::getline( csv, line ); ++lineno ) {
        vector<string> elems;
        auto ok = parse_csv_line( line, elems );
        if ( 0 == lineno ) {
            if ( header_line ) {
                for ( auto eit = elems.begin(); eit != elems.end(); ++eit ) {

                    frame.emplace_back( Series<string> 
                    Series<string> s( 
                }
            }
        }
    }
    return Frame{};
}

#endif

} // namespace mf

