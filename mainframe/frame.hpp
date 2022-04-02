
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
//#include "mainframe/expression.hpp"
#include "mainframe/series.hpp"

namespace mf
{

template< typename ... Ts >
class frame 
{
public:
    using iterator = frame_iterator< Ts... >;
    using const_iterator = frame_iterator< const Ts... >;
    using reverse_iterator = reverse_frame_iterator< Ts... >;
    using const_reverse_iterator = reverse_frame_iterator< const Ts... >;

    frame() = default;
    frame( const frame& ) = default;
    frame( frame&& ) = default;
    frame& operator=( const frame& ) = default;
    frame& operator=( frame&& ) = default;

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

    bool empty() const
    {
        return std::get< 0 >( m_columns ).empty();
    }

    void clear()
    {
        clear_impl< 0 >();
    }

    template< size_t Ind > 
    void clear_impl()
    {
        std::get< Ind >( m_columns ).clear();
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            clear_impl< Ind+1 >();
        }
    }

    iterator insert( iterator pos, iterator first, iterator last )
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

    template< size_t Ind, typename U, typename ... Us >
    frame_iterator<U, Us...> insert_impl( iterator pos, iterator first, iterator last )
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

    iterator erase( iterator first, iterator last )
    {
        return erase_impl< 0, Ts... >( first, last );
    }

    iterator erase( iterator pos )
    {
        return erase_impl< 0, Ts... >( pos );
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

    template< size_t Ind > 
    std::string column_name() const
    {
        return std::get< Ind >( m_columns ).name();
    }

    size_t num_columns() const
    {
        return sizeof...( Ts );
    }

    template< typename ...Us >
    void push_back( Us... args )
    {
        push_back_impl<0, Us...>( args... );
    }

    template< size_t Ind, typename U, typename ... Us >
    void push_back_impl( const U& t, const Us& ... us )
    {
        std::get< Ind >( m_columns ).push_back( t );
        if constexpr ( sizeof...( Us ) > 0 ) {
            push_back_impl< Ind+1 >( us... );
        }
    }

    template< typename ... Us >
    void set_column_names( const Us& ... colnames )
    {
        set_column_names_impl< 0, Us... >( colnames... );
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

    size_t size() const
    {
        return size_impl_with_check<0, Ts...>();
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



    std::tuple<series<Ts>...> m_columns;
};

} // namespace mf


#endif // INCLUDED_mainframe_frame_h

