
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_csv_hpp
#define INCLUDED_mainframe_csv_hpp

#include <filesystem>
#include <mainframe/frame.hpp>

namespace mf
{

namespace CSV_OPTIONS
{
    enum
    {
        HEADER_ROW = (1 << 0),
        FAIL_ON_EXTRA_COLUMNS = (1 << 1),
        FAIL_ON_CONVERSION = (1 << 2),
    };
}




template< typename ... Ts >
class csv
{
public:
    static frame< Ts... >
    load( const std::filesystem::path & path, unsigned options )
    {
        std::ifstream fs{ filename };
        return load( fs, options );
    }

    static frame< Ts... >
    load( std::istream& stream, unsigned options )
    {
        frame< Ts... > out;
        if (options & CSV_OPTIONS::HEADER_ROW) {

            std::string line; 
            std::getline(stream, line);
            std::vector<mi<std::string>> elems;
            if ( !parse_line( line, elems ) ) {
                out.clear();
                return out;
            }
            std::array<std::string, sizeof...(Ts)> column_names;
            for (auto i = 0; i < elems.size(); ++i ) {
                auto& elem = elems[i];
                if (elem.has_value()) {
                    column_names[i] = *elem;
                } 
            }
            out.set_column_names(column_names);
        }
        for ( std::string line; std::getline( stream, line );  ) {
            std::vector<mi<std::string>> elems;
            if ( !parse_line( line, elems ) ) {
                out.clear();
                return out;
            }
            //out.push_back(elems);
        }
        return out;
    }

private:
    static bool 
    parse_line( std::string line, std::vector<mi<std::string>>& elems )
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
                    elems.push_back(mi<std::string>{ std::string{ first_char, it } } );
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
        elems.push_back(mi<std::string>{ std::string{ first_char, it } } );
        return true;
    }

};

} // namespace mf
 
#endif // INCLUDED_mainframe_csv_hpp

