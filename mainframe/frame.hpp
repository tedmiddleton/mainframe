
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_frame_h
#define INCLUDED_mainframe_frame_h

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <cmath>
#include <string>
#include <list>
#include <iomanip>
#include <optional>
#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <ostream>
#include <sstream>
#include <variant>

#include "mainframe/base.hpp"
#include "mainframe/frame_iterator.hpp"
#include "mainframe/expression.hpp"
#include "mainframe/series.hpp"

namespace mf
{

template< typename ... Ts >
class frame;

class uframe
{
public:
    uframe() = default;
    uframe( const uframe& ) = default;
    uframe( uframe&& ) = default;
    uframe& operator=( const uframe& ) = default;
    uframe& operator=( uframe&& ) = default;

    template< typename ... Ts >
    uframe( frame< Ts... >& f )
    {
        init_impl< 0 >( f );
    }

    template< typename ... Ts >
    operator frame< Ts... >()
    {
        frame< Ts... > out;
        out.populate( m_columns );
        return out;
    }

    void add_series( const useries& s )
    {
        m_columns.push_back( s );
    }

    void set_series( size_t idx, const useries& s )
    {
        m_columns.at( idx ) = s;
    }

    size_t num_columns() const
    {
        return m_columns.size();
    }

private:

    template< size_t Ind, typename ... Ts >
    void init_impl( frame< Ts... >& f )
    {
        useries s = f.template column<Ind>();
        m_columns.push_back( s );
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            init_impl< Ind+1 >( f );
        }
    }

    std::vector< useries > m_columns;
};

template< typename ... Ts >
class frame;

template<typename T, typename Tpl>
struct prepend;

template<typename T, typename ... Ts>
struct prepend<T, frame<Ts...>>
{
    using type = frame<T, Ts...>;
};

template<size_t Ind, size_t Curr, typename ... Ts>
struct pack_element_impl;

template<size_t Matched, typename T, typename ... Ts>
struct pack_element_impl<Matched, Matched, T, Ts...>
{
    using type = T;
};

template<size_t Ind, size_t Curr, typename T, typename ... Ts>
struct pack_element_impl<Ind, Curr, T, Ts...>
{
    using type = typename pack_element_impl<Ind, Curr+1, Ts...>::type;
};

template<size_t Ind, typename ... Ts>
struct pack_element : pack_element_impl<Ind, 0, Ts...> {};

// This makes frame::columns(_0, ...) work
template<typename tpl, size_t...Inds> 
struct rearrange;

template<size_t IndHead, size_t...IndRest, typename ... Ts>
struct rearrange<frame<Ts...>, IndHead, IndRest...>
{
    using indexed_type = typename pack_element<IndHead, Ts...>::type;
    using remaining_frame = typename rearrange<frame<Ts...>, IndRest...>::type;
    using type = typename prepend<indexed_type, remaining_frame>::type;
};

template<size_t IndHead, typename ... Ts>
struct rearrange<frame<Ts...>, IndHead>
{
    using indexed_type = typename pack_element<IndHead, Ts...>::type;
    using type = frame<indexed_type>;
};

template < size_t Ind, size_t ... List >
struct contains : std::true_type {};

template < size_t Ind, size_t IndHead, size_t... IndRest >
struct contains<Ind, IndHead, IndRest...> : 
    std::conditional< 
        Ind == IndHead,
        std::true_type,
        contains<Ind, IndRest ... >
    >::type {};

template < size_t Ind >
struct contains<Ind> : std::false_type {};

template< typename T, typename U >
struct join_frames;

template< typename ... Ts, typename ... Us >
struct join_frames<frame< Ts... >, frame< Us... > >
{
    using type = frame< Ts..., Us... >;
};

template<size_t Ind, typename Frame>
struct frame_element;

template<size_t Ind, typename ... Ts>
struct frame_element<Ind, frame<Ts...>> : pack_element<Ind, Ts...> {};

template<typename T, size_t Curr, size_t ... IndList>
struct add_opt;

template<typename T, typename ... Ts, size_t Curr, size_t ... IndList>
struct add_opt<frame<std::optional<T>, Ts...>, Curr, IndList...>
{
    using frame_type = frame<std::optional<T>>; 
    using add_opt_type = typename add_opt<frame<Ts...>, Curr+1, IndList...>::type;
    using type = typename join_frames< frame_type, add_opt_type >::type;
};

template<typename T, typename ... Ts, size_t Curr, size_t ... IndList>
struct add_opt<frame<T, Ts...>, Curr, IndList...>
{
    static const bool inds_contains = contains< Curr, IndList... >::value;
    using frame_type = typename 
        std::conditional<inds_contains, frame<std::optional<T>>, frame<T> >::type; 
    using add_opt_type = typename add_opt<frame<Ts...>, Curr+1, IndList...>::type;
    using type = typename join_frames< frame_type, add_opt_type >::type;
};

template<typename T, size_t Curr, size_t ... IndList>
struct add_opt<frame<std::optional<T>>, Curr, IndList...>
{
    using type = std::optional<T>;
};

template<typename T, size_t Curr, size_t ... IndList>
struct add_opt<frame<T>, Curr, IndList...>
{
    static const bool inds_contains = contains< Curr, IndList... >::value;
    using type = typename 
        std::conditional< inds_contains, frame<std::optional<T>>,  frame<T>>::type;
};

template<typename T>
struct add_all_opt;

template<typename T, typename ... Ts>
struct add_all_opt<frame<std::optional<T>, Ts...>>
{
    using remaining_frame = typename add_all_opt<frame<Ts...>>::type;
    using type = typename prepend<std::optional<T>, remaining_frame>::type;
};

template<typename T, typename ... Ts>
struct add_all_opt<frame<T, Ts...>>
{
    using remaining_frame = typename add_all_opt<frame<Ts...>>::type;
    using type = typename prepend<std::optional<T>, remaining_frame>::type;
};

template<typename T>
struct add_all_opt<frame<std::optional<T>>>
{
    using type = frame<std::optional<T>>;
};

template<typename T>
struct add_all_opt<frame<T>>
{
    using type = frame<std::optional<T>>;
};

template< typename ... Ts >
class frame
{
public:
    using iterator = frame_iterator< Ts... >;
    using const_iterator = const_frame_iterator< Ts... >;
    using reverse_iterator = reverse_frame_iterator< Ts... >;
    using const_reverse_iterator = const_reverse_frame_iterator< Ts... >;

    frame() = default;
    frame( const frame& ) = default;
    frame( frame&& ) = default;
    frame& operator=( const frame& ) = default;
    frame& operator=( frame&& ) = default;

    frame< Ts... > operator+( const frame< Ts... >& other ) const
    {
        frame< Ts... > out{ *this };
        out.insert( out.end(), other.cbegin(), other.cend() );
        return out;
    }

    iterator begin()
    {
        return iterator{ m_columns, 0 };
    }
    iterator end()
    {
        return iterator{ m_columns, static_cast<int>(size()) };
    }
    const_iterator begin() const
    {
        return const_iterator{ m_columns, 0 };
    }
    const_iterator end() const
    {
        return const_iterator{ m_columns, static_cast<int>(size()) };
    }
    const_iterator cbegin() const
    {
        return const_iterator{ m_columns, 0 };
    }
    const_iterator cend() const
    {
        return const_iterator{ m_columns, static_cast<int>(size()) };
    }

    reverse_iterator rbegin()
    {
        return reverse_iterator{ m_columns, 0 };
    }
    reverse_iterator rend()
    {
        return reverse_iterator{ m_columns, static_cast<int>(size()) };
    }
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator{ m_columns, 0 };
    }
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator{ m_columns, static_cast<int>(size()) };
    }
    const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator{ m_columns, 0 };
    }
    const_reverse_iterator crend() const
    {
        return const_reverse_iterator{ m_columns, static_cast<int>(size()) };
    }

    template< size_t ... Inds >
    typename add_opt<frame<Ts...>, 0, Inds...>::type 
    allow_missing( terminal<expr_column<Inds>>... cols )
    {
        uframe u( *this );
        allow_missing_impl< 0, Inds... >( u, cols... );
        return u;
    }

    template< size_t Ind, size_t ... Inds >
    void allow_missing_impl( uframe& uf,
                             terminal<expr_column<Inds>>... cols )
    {
        if constexpr ( contains<Ind, Inds...>::value ) {
            auto& s = column<Ind>();
            auto ams = s.allow_missing();
            uf.set_series( Ind, ams );
        }
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            allow_missing_impl< Ind+1, Inds... >( uf, cols... );
        }
    }

    typename add_all_opt<frame<Ts...>>::type allow_missing()
    {
        uframe u;
        allow_missing_impl<0, Ts...>( u );
        return u;
    }

    template<size_t Ind, typename U, typename ... Us> 
    void allow_missing_impl( uframe& uf )
    {
        series<U>& s = std::get<Ind>( m_columns );
        auto os = s.allow_missing();
        uf.add_series( os );
        if constexpr ( sizeof...( Us ) > 0 ) {
            allow_missing_impl< Ind+1, Us... >( uf );
        }
    }

    //remove_all_opt<frame<Ts...>> disallow_missing()
    //{
    //}

    void clear()
    {
        clear_impl< 0 >();
    }

    useries column( const std::string& colname )
    {
        return column_impl< 0, Ts... >( colname );
    }

    template< size_t Ind >
    series<typename pack_element<Ind, Ts...>::type>& column()
    {
        return std::get<Ind>( m_columns );
    }

    template< size_t Ind >
    series<typename pack_element<Ind, Ts...>::type>& column( terminal<expr_column<Ind>> )
    {
        return std::get<Ind>( m_columns );
    }

    template< size_t Ind >
    std::string column_name() const
    {
        return std::get< Ind >( m_columns ).name();
    }

    std::array< std::string, sizeof...(Ts) > column_names() const
    {
        std::array< std::string, sizeof...(Ts) > out;
        column_names_impl< 0 >( out );
        return out;
    }

    template< size_t ... Inds >
    typename rearrange<frame<Ts...>, Inds...>::type 
    columns( terminal<expr_column<Inds>>... cols )
    {
        uframe f;
        columns_impl( f, cols... );
        return f;
    }

    template< typename ... Us >
    uframe columns( const Us& ... us )
    {
        uframe f;
        columns_impl( f, us... );
        return f;
    }

    bool empty() const
    {
        return std::get< 0 >( m_columns ).empty();
    }

    iterator erase( iterator first, iterator last )
    {
        return erase_impl< 0, Ts... >( first, last );
    }

    iterator erase( iterator pos )
    {
        return erase_impl< 0, Ts... >( pos );
    }

    iterator insert( iterator pos, const_iterator first, const_iterator last )
    {
        return insert_impl< 0, Ts... >( pos, first, last );
    }

    iterator insert( iterator pos, const Ts&... ts )
    {
        return insert_impl< 0, Ts... >( pos, 1, ts... );
    }

    iterator insert( iterator pos, size_t count, const Ts&... ts )
    {
        return insert_impl< 0, Ts... >( pos, count, ts... );
    }

    template< typename T >
    frame< Ts..., T > new_series( const std::string& series_name )
    {
        uframe plust( *this );
        series<T> ns( size() );
        ns.set_name( series_name );
        useries us( ns );
        plust.add_series( us );
        return plust;
    }

    template< typename T, typename Ex >
    frame< Ts..., T > new_series( const std::string& series_name, Ex expr )
    {
        uframe plust( *this );
        series<T> ns( size() );
        ns.set_name( series_name );
        useries us( ns );
        plust.add_series( us );
        frame< Ts..., T > out = plust;
        for ( auto it = out.begin(); it != out.end(); ++it ) {
            auto val = expr.get_value( it );
            it->template at<sizeof...(Ts)>() = val;
        }
        return out;
    }

    size_t num_columns() const
    {
        return sizeof...( Ts );
    }

    void pop_back()
    {
        pop_back_impl<0>();
    }

    template< template<typename> typename Iter >
    void push_back( const frame_row< Iter, Ts... >& fr )
    {
        push_back_impl<0>( fr );
    }

    template< typename U, typename V, typename ...Us >
    void push_back( U first_arg, V second_arg, Us... args )
    {
        push_back_impl<0, U, V, Us...>( first_arg, second_arg, args... );
    }

    void resize( size_t newsize )
    {
        resize_impl<0>( newsize );
    }

    template< typename Ex >
    frame< Ts... > rows( Ex ex ) const
    {
        frame< Ts... > out;
        out.set_column_names( column_names() );

        auto fi = cbegin();
        for ( ; fi != cend(); ++fi ) {
            auto e = ex.get_value( fi );
            if ( e ) {
                out.push_back( *fi );
            }
        }

        return out;
    }

    void set_column_names( const std::array< std::string, sizeof...( Ts ) >& names )
    {
        set_column_names_impl< 0 >( names );
    }

    template< typename ... Us >
    void set_column_names( const Us& ... colnames )
    {
        set_column_names_impl< 0, Us... >( colnames... );
    }

    size_t size() const
    {
        return size_impl_with_check<0, Ts...>();
    }

    std::vector<std::vector<std::string>> to_string() const
    {
        std::vector<std::vector<std::string>> out;
        to_string_impl<0, Ts...>( out );
        return out;
    }

private:

    template< size_t Ind > 
    void clear_impl()
    {
        std::get< Ind >( m_columns ).clear();
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            clear_impl< Ind+1 >();
        }
    }

    template< size_t Ind, typename U, typename ... Us >
    useries column_impl( const std::string& colname )
    {
        series<U>& s = std::get<Ind>( m_columns );
        if ( s.name() == colname ) {
            return s;
        }
        if constexpr ( sizeof...( Us ) > 0 ) {
            return column_impl< Ind+1, Us... >( colname );
        }
        else {
            throw std::out_of_range{ "out of range" };
        }
    }

    template< size_t Ind >
    void column_names_impl( std::array< std::string, sizeof...(Ts) >& out ) const
    {
        auto& s = std::get<Ind>( m_columns );
        out[Ind] = s.name();
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            column_names_impl< Ind+1 >( out );
        }
    }

    template< typename U, typename ... Us >
    void columns_impl( uframe& f, const U& u, const Us& ... us )
    {
        useries s = column( u );
        f.add_series( s );
        if constexpr ( sizeof...( Us ) > 0 ) {
            columns_impl( f, us... );
        }
    }

    template< size_t Ind, typename U, typename ... Us >
    frame_iterator<U, Us...> erase_impl( iterator pos )
    {
        series<U>& s = std::get< Ind >( m_columns );
        auto column_pos = pos.template column_iterator< Ind >();
        typename series<U>::iterator newcpos = s.erase( column_pos );
        frame_iterator<U> out{ std::make_tuple( newcpos ) };
        if constexpr ( sizeof...( Us ) > 0 ) {
            frame_iterator<Us...> dstream = erase_impl<Ind+1, Us...>( pos );
            return out.combine( dstream );
        }
        else {
            return out;
        }
    }

    template< size_t Ind, typename U, typename ... Us >
    frame_iterator<U, Us...> erase_impl( iterator first, iterator last )
    {
        series<U>& s = std::get< Ind >( m_columns );
        auto column_first = first.template column_iterator< Ind >();
        auto column_last = last.template column_iterator< Ind >();
        typename series<U>::iterator newcpos = s.erase( column_first, column_last );
        frame_iterator<U> out{ std::make_tuple( newcpos ) };
        if constexpr ( sizeof...( Us ) > 0 ) {
            frame_iterator<Us...> dstream = erase_impl<Ind+1, Us...>( first, last );
            return out.combine( dstream );
        }
        else {
            return out;
        }
    }

    template< size_t Ind, typename U, typename ... Us >
    frame_iterator<U, Us...> insert_impl( iterator pos, size_t count, 
                                          const U& u, const Us& ... us )
    {
        series<U>& s = std::get< Ind >( m_columns );
        auto column_pos = pos.template column_iterator< Ind >();
        typename series<U>::iterator newcpos = s.insert( column_pos, count, u );
        frame_iterator<U> out{ std::make_tuple( newcpos ) };
        if constexpr ( sizeof...( Us ) > 0 ) {
            frame_iterator<Us...> dstream = insert_impl<Ind+1, Us...>( pos, count, us... );
            return out.combine( dstream );
        }
        else {
            return out;
        }
    }

    template< size_t Ind, typename U, typename ... Us >
    frame_iterator<U, Us...> insert_impl( iterator pos, 
                                          const_iterator first, 
                                          const_iterator last )
    {
        series<U>& s = std::get< Ind >( m_columns );
        auto column_pos = pos.template column_iterator< Ind >();
        auto column_first = first.template column_iterator< Ind >();
        auto column_last = last.template column_iterator< Ind >();
        typename series<U>::iterator newcpos = s.insert( 
            column_pos, 
            column_first, 
            column_last
            );
        frame_iterator<U> out{ std::make_tuple( newcpos ) };
        if constexpr ( sizeof...( Us ) > 0 ) {
            frame_iterator<Us...> dstream = insert_impl<Ind+1, Us...>( pos, first, last );
            return out.combine( dstream );
        }
        else {
            return out;
        }
    }

    template< size_t Ind >
    void pop_back_impl()
    {
        auto& s = std::get<Ind>( m_columns );
        s.pop_back();
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            pop_back_impl< Ind+1 >();
        }
    }

    void populate( std::vector<useries>& columns )
    {
        populate_impl< 0, Ts... >( columns );
    }

    template< size_t Ind, typename U, typename ... Us >
    void populate_impl( std::vector<useries>& columns )
    {
        useries& su = columns.at( Ind );
        series<U>& s = std::get< Ind >(m_columns);
        s = series<U>( su );
        if constexpr ( sizeof...( Us ) > 0 ) {
            populate_impl< Ind+1, Us... >( columns );
        }
    }

    template< size_t Ind, template<typename> typename Iter >
    void push_back_impl( const frame_row< Iter, Ts... >& fr )
    {
        auto elem = fr.template at< Ind >();
        std::get< Ind >( m_columns ).push_back( elem );
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            push_back_impl< Ind+1 >( fr );
        }
    }

    template< size_t Ind, typename U, typename ... Us >
    void push_back_impl( const U& t, const Us& ... us )
    {
        std::get< Ind >( m_columns ).push_back( t );
        if constexpr ( sizeof...( Us ) > 0 ) {
            push_back_impl< Ind+1 >( us... );
        }
    }

    template< size_t Ind >
    void resize_impl( size_t newsize )
    {
        auto& s = std::get<Ind>( m_columns );
        s.resize( newsize );
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            resize_impl< Ind+1 >( newsize );
        }
    }

    template< size_t Ind >
    void set_column_names_impl( const std::array< std::string, sizeof...( Ts ) >& names )
    {
        auto& s = std::get< Ind >( m_columns );
        s.set_name( names[ Ind ] );
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            set_column_names_impl< Ind+1 >( names );
        }
    }

    template< size_t Ind, typename U, typename ... Us >
    void set_column_names_impl( const U& colname, const Us& ... colnames )
    {
        auto& s = std::get< Ind >( m_columns );
        s.set_name( colname );
        if constexpr ( sizeof...( Us ) > 0 ) {
            set_column_names_impl< Ind+1, Us... >( colnames... );
        }
    }

    template< size_t Ind, typename U, typename ... Us >
    size_t size_impl_with_check() const
    {
        const series<U>& series = std::get< Ind >( m_columns );
        size_t s = series.size();
        if constexpr ( sizeof...( Us ) > 0 ) {
            size_t si = size_impl_with_check< Ind+1, Us... >();
            if ( si != s ) {
                throw std::logic_error{ "Inconsistent sizes!" };
            }
        }
        return s;
    }

    template< size_t Ind, typename U, typename ... Us >
    void to_string_impl( std::vector< std::vector< std::string > >& strs ) const
    {
        const series<U>& s = std::get< Ind >( m_columns );
        std::vector< std::string > colstrs = s.to_string();
        strs.push_back( colstrs );
        if constexpr ( sizeof...( Us ) > 0 ) {
            to_string_impl< Ind+1, Us... >( strs );
        }
    }

    template< typename ... Us >
    friend std::ostream & operator<<( std::ostream&, const frame< Us... >& );
    friend class uframe;

    std::tuple<series<Ts>...> m_columns;
};

template< typename ... Ts >
std::ostream & operator<<( std::ostream& o, const frame< Ts... >& f )
{
    std::ios_base::fmtflags fl( o.flags() );

    std::vector<std::vector<std::string>> uf = f.to_string();
    auto names = f.column_names();
    auto widths = get_max_string_lengths( uf );
    constexpr size_t num_columns = sizeof...(Ts);
    const size_t num_rows = f.size();

    auto gutter_width = 
        num_rows > 0 ? std::ceil( std::log10( num_rows ) ) + 1 : 1;

    o << std::boolalpha;
    o << get_emptyspace( gutter_width );
    for ( size_t i = 0; i < num_columns; ++i ) {
        auto& name = names[ i ];
        auto& width = widths[ i ];
        width = std::max( width, name.size() );
        o << "| " << std::setw( width ) << name << " ";
    }
    o << "\n";

    o << get_horzrule( gutter_width );
    for ( size_t i = 0; i < num_columns; ++i ) {
        auto& width = widths[ i ];
        o << "|" << get_horzrule( width + 2 );
    }
    o << "\n";

    for ( size_t rowind = 0; rowind < num_rows; ++rowind ) {
        o << std::setw( gutter_width ) << rowind;
        for ( size_t colind = 0; colind < num_columns; ++colind ) {

            std::string& datum = uf[ colind ][ rowind ];
            auto width = widths[ colind ];

            o << "| " << std::setw( width ) << datum << " ";
        }
        o << "\n";
    }

    o.flags( fl );
    return o;
}

} // namespace mf

#endif // INCLUDED_mainframe_frame_h

