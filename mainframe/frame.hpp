
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

