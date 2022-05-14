
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

namespace detail
{
    template< size_t NumCols >
    struct msf_impl
    {
        using framecdr = typename msf_impl<NumCols-1>::type;
        using type = typename detail::prepend<mi<std::string>, framecdr>::type;
    };

    template<>
    struct msf_impl< 0 >
    {
        using type = frame<mi<std::string>>;
    };

    template< size_t NumCols >
    using make_string_frame = msf_impl<NumCols-1>;
}

#define load_csv( NUMCOLUMNS, STREAM, HEADERROW ) csv::load<NUMCOLUMNS>( STREAM, HEADERROW ) 
class csv
{
public:
    //static frame< Ts... >
    //load( const std::filesystem::path & path, unsigned options )
    //{
    //    std::ifstream fs{ path };
    //    return load( fs, options );
    //}

    template< size_t NumColumns >
    static typename detail::make_string_frame< NumColumns >::type
    load( std::istream& stream, bool header_row )
    {
        using frame_type = typename detail::make_string_frame< NumColumns >::type;
        frame_type out;
        using row_type = typename frame_type::row_type;
        if ( header_row ) {

            std::string line; 
            std::getline(stream, line);
            std::vector<mi<std::string>> elems;
            if ( !parse_line( line, elems ) ) {
                out.clear();
                return out;
            }
            std::array<std::string, NumColumns> column_names;
            for (size_t i = 0; i < elems.size(); ++i ) {
                auto& elem = elems[i];
                if (elem.has_value()) {
                    column_names[i] = *elem;
                } 
            }
            out.set_column_names(column_names);
        }
        for ( std::string line; std::getline( stream, line );  ) {
            row_type elems;
            if ( !parse_line( line, elems ) ) {
                out.clear();
                return out;
            }
            out.push_back(elems);
        }
        return out;
    }

private:
    template< typename T >
    static bool 
    parse_line( std::string line, std::vector<T>& elems )
    {
        enum class STATE
        {
            START,
            ELEMCHAR,
            QUOTEDELEMCHAR,
            ENDEDQUOTE,
            COMMA,
        } state = STATE::START;

        auto it = line.begin();
        std::string current;
        current.reserve( line.size() );
        for ( ;it != line.end(); it++ )
        {
            auto c = *it;
            switch ( state ) {
            case STATE::START:
                if ( c == ',' ) {
                    elems.emplace_back( missing );
                    state = STATE::COMMA;
                }
                else {
                    current.push_back( c );
                    state = STATE::ELEMCHAR;
                }
                break;

            case STATE::ELEMCHAR:
                if ( c == ',' ) {
                    elems.push_back( mi( current ) );
                    current.clear();
                    state = STATE::COMMA;
                }
                else {
                    current.push_back( c );
                }
                break;

            case STATE::QUOTEDELEMCHAR:
                if ( c == '"' ) {
                    state = STATE::ENDEDQUOTE;
                }
                else {
                    current.push_back( c );
                }
                break;

            case STATE::ENDEDQUOTE:
                if ( c == '"' ) {
                    current.push_back( '"' );
                    state = STATE::QUOTEDELEMCHAR;
                }
                else if ( c == ',' ) {
                    elems.push_back( mi( std::string{ current } ) );
                    state = STATE::COMMA;
                }
                break;

            case STATE::COMMA:
                if ( c == ',' ) {
                    elems.emplace_back( missing );
                }
                else if ( c == '"' ) {
                    state = STATE::QUOTEDELEMCHAR;
                }
                else {
                    current.push_back( c );
                    state = STATE::ELEMCHAR;
                }
                break;
            }
        }

        switch ( state ) {
        case STATE::START:
            break;

        case STATE::ELEMCHAR:
            elems.push_back( mi( current ) );
            break;

        case STATE::QUOTEDELEMCHAR:
            // broken - no end quote
            elems.emplace_back( missing );
            break;

        case STATE::ENDEDQUOTE:
            elems.push_back( mi( current ) );
            break;

        case STATE::COMMA:
            elems.emplace_back( missing );
            break;
        }

        return true;
    }

};

} // namespace mf
 
#endif // INCLUDED_mainframe_csv_hpp

