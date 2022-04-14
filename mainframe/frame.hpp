
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

    void add_series( useries& s )
    {
        m_columns.push_back( s );
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

    void clear()
    {
        clear_impl< 0 >();
    }

    useries column( const std::string& colname )
    {
        return column_impl< 0, Ts... >( colname );
    }

    template< size_t Ind >
    useries column()
    {
        auto& s = std::get<Ind>( m_columns );
        useries us( s );
        return us;
    }

    template< size_t Ind >
    useries column( terminal<expr_column<Ind>> )
    {
        auto& s = std::get<Ind>( m_columns );
        useries us( s );
        return us;
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
        auto cn = column_names();

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

    template< typename ... Us >
    friend std::ostream & operator<<( std::ostream&, const frame< Us... >& );
    friend class uframe;

    std::tuple<series<Ts>...> m_columns;
};

} // namespace mf


#endif // INCLUDED_mainframe_frame_h

