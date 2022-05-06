
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <fstream>
#include <iomanip>
#include <mainframe/csv.hpp>
#include <mainframe/frame.hpp>

using namespace std;


namespace mf
{

static bool parse_csv_line( string line, vector<mi<string>>& elems )
{
    enum class STATE
    {
        START,
        ELEMCHAR,
        COMMA,
    } state = STATE::START;

    auto it = line.begin();
    auto first_char = it;
    for ( ;it != line.end(); it++ )
    {
        auto c = *it;
        switch ( state ) {
        case STATE::START:
            if ( c == ',' ) {
                elems.emplace_back( missing );
                first_char = it+1;
                state = STATE::COMMA;
            }
            else {
                state = STATE::ELEMCHAR;
            }
            break;

        case STATE::ELEMCHAR:
            if ( c == ',' ) {
                elems.push_back(mi<string>{ string{ first_char, it } } );
                first_char = it+1;
                state = STATE::COMMA;
            }
            break;

        case STATE::COMMA:
            if ( c == ',' ) {
                elems.emplace_back( missing );
                first_char = it+1;
            }
            else {
                state = STATE::ELEMCHAR;
            }
            break;
        }
    }
    elems.push_back(mi<string>{ string{ first_char, it } } );
    return true;
}

uframe load_csv( const std::filesystem::path & filename, bool header_line )
{
    std::ifstream fs{ filename };
    return load_csv( fs, header_line );
}

uframe
load_csv( std::istream& csv, bool header_line )
{
    uframe out;
    int lineno = 0;
    for ( string line; std::getline( csv, line ); ++lineno ) {
        vector<mi<string>> elems;
        if ( !parse_csv_line( line, elems ) ) {
            out.clear();
            return out;
        }
        if ( 0 == lineno ) {
            if ( header_line ) {
                for ( auto eit = elems.begin(); eit != elems.end(); ++eit ) {

                }
            }
        }
        else {
        }
    }
    return out;
}


} // namespace mf

