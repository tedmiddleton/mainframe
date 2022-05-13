
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

