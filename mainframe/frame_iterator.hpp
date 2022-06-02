
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_frame_iterator_h
#define INCLUDED_mainframe_frame_iterator_h

#include <iterator>
#include "mainframe/series.hpp"

namespace mf
{

template< typename ... Ts >
struct pointerize;

template< typename ... Ts >
struct pointerize<std::tuple< series<Ts>...>>
{
    using argument_type = std::tuple< series<Ts>... >;
    using type = std::tuple< Ts*... >;
    using const_type = std::tuple< Ts*... >;

    static type op( argument_type& arg, ptrdiff_t offset = 0 )
    {
        type out;
        assign_impl<0>( arg, offset, out );
        return out;
    }

    static type const_op( const argument_type& arg, ptrdiff_t offset = 0 )
    {
        const_type out;
        assign_const_impl<0>( arg, offset, out );
        return out;
    }

    template< size_t Ind >
    static void assign_impl( argument_type& arg, 
                             ptrdiff_t offset, 
                             type& out )
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        series<T>& argelem = std::get<Ind>( arg );
        std::get<Ind>( out ) = argelem.data() + offset;
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            assign_impl< Ind+1 >( arg, offset, out );
        }
    }

    template< size_t Ind >
    static void assign_const_impl( const argument_type& arg, 
                                   ptrdiff_t offset, 
                                   const_type& out )
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        const series<T>& argelem = std::get<Ind>( arg );
        std::get<Ind>( out ) = argelem.data() + offset;
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            assign_const_impl< Ind+1 >( arg, offset, out );
        }
    }
};

template< typename T >
using series_iterator = typename series<T>::iterator;

template< typename T >
using const_series_iterator = typename series<T>::const_iterator;

template< typename T >
using reverse_series_iterator = typename series<T>::reverse_iterator;

template< typename T >
using const_reverse_series_iterator = typename series<T>::const_reverse_iterator;

template< size_t Ind >
struct expr_column;

template< typename T >
struct terminal;

template< template<typename> class Iter, typename ... Ts >
class _row_proxy
{
public:
    _row_proxy() = default;

    _row_proxy( std::tuple< Iter<Ts>... > iter ) 
        : m_iters( iter ) 
    {
        init_ptrs( iter );
    }

    template< size_t Ind=0 >
    void init_ptrs( const std::tuple< Iter<Ts>... >& iter )
    {
        std::get<Ind>( m_ptrs ) = std::get<Ind>( iter ).data();
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            init_ptrs< Ind+1 >( iter );
        }
    }

    template< size_t Ind >
    typename std::tuple_element< Ind, std::tuple< typename Iter<Ts>::reference ... > >::type
    at() const
    {
        return *std::get<Ind>( m_ptrs );
    }

    template< size_t Ind >
    typename std::tuple_element< Ind, std::tuple< typename Iter<Ts>::reference ... > >::type
    at( terminal<expr_column<Ind>> ) const
    {
        return *std::get<Ind>( m_ptrs );
    }

    template< size_t Ind >
    typename std::tuple_element< Ind, std::tuple< typename Iter<Ts>::reference ... > >::type
    operator[]( terminal<expr_column<Ind>> ) const
    {
        return *std::get<Ind>( m_ptrs );
    }

    bool any_missing() const
    {
        return any_missing_impl<0, Ts...>();
    }

    template< size_t Ind, typename U, typename ... Us >
    bool any_missing_impl() const
    {
        const U& elem = at< Ind >();
        if constexpr ( detail::is_missing<U>::value ) {
            if ( !elem.has_value() ) {
                return true;
            }
        }
        if constexpr ( sizeof...(Us) > 0 ) {
            return any_missing_impl<Ind+1, Us...>();
        }
        return false;
    }

    template< size_t Ind=0 >
    void debug() const
    {
        if ( std::get<Ind>( m_ptrs ) != std::get<Ind>( m_iters ).data() ) {
            std::stringstream ss;
            ss << "debug failure with index " << Ind;
            throw std::logic_error{ ss.str() };
        }
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            debug< Ind+1 >();
        }
    }

private:

    template< template<typename> class It, typename ... Us >
    friend class base_frame_iterator;
    template< typename ... Us >
    friend class frame_iterator;
    template< typename ... Us >
    friend class const_frame_iterator;
    template< typename ... Us >
    friend class reverse_frame_iterator;
    template< typename ... Us >
    friend class const_reverse_frame_iterator;

    template< 
        size_t Ind, 
        template<typename> class OtherIter, 
        typename U, 
        typename ... Us >
    void init_from_other_iter( std::tuple< OtherIter<Ts>...>& otheriters )
    {
        OtherIter<U>& oiter = std::get< Ind >( otheriters );
        Iter<U>& iter = std::get< Ind >( m_iters );
        iter = Iter<U>( oiter );
        if constexpr ( sizeof...( Us ) > 0 ) {
            init_from_other_iter< Ind+1, OtherIter, Us... >( otheriters );
        }
    }

    template< size_t Ind >
    typename std::tuple_element< Ind, std::tuple< Iter<Ts> ... > >::type
    get_iterator() const
    {
        auto& iter = std::get< Ind >( m_iters );
        return iter;
    }

    template< size_t Ind >
    void set_iterator(
        typename std::tuple_element< Ind, std::tuple< Iter<Ts> ... > >::type it ) 
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        std::get< Ind >( m_iters ) = it;
        std::get< Ind >( m_ptrs ) = const_cast< T* >( it.data() );
    }

    template< size_t Ind = 0 >
    void inc( ptrdiff_t n )
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        std::get< Ind >( m_iters ) += n;
        if constexpr ( std::is_same<Iter<T>, reverse_series_iterator<T>>::value || 
                       std::is_same<Iter<T>, const_reverse_series_iterator<T>>::value ) {
            std::get< Ind >( m_ptrs ) -= n;
        }
        else {
            std::get< Ind >( m_ptrs ) += n;
        }
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            inc< Ind+1 >( n );
        }
    }

    template< size_t Ind = 0 >
    void dec( ptrdiff_t n )
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        std::get< Ind >( m_iters ) -= n;
        if constexpr ( std::is_same<Iter<T>, reverse_series_iterator<T>>::value || 
                       std::is_same<Iter<T>, const_reverse_series_iterator<T>>::value ) {
            std::get< Ind >( m_ptrs ) += n;
        }
        else {
            std::get< Ind >( m_ptrs ) -= n;
        }
        if constexpr ( Ind+1 < sizeof...( Ts ) ) {
            dec< Ind+1 >( n );
        }
    }

    template< 
        template<typename> typename ItLeft, 
        template<typename> typename ItRight, 
        typename ... Us >
    friend bool operator==( const _row_proxy< ItLeft, Us... >& l, 
                            const _row_proxy< ItRight, Us... >& r );
    template< 
        template<typename> typename ItLeft, 
        template<typename> typename ItRight, 
        typename ... Us >
    friend bool operator!=( const _row_proxy< ItLeft, Us... >& l, 
                            const _row_proxy< ItRight, Us... >& r );
    template< 
        template<typename> typename ItLeft, 
        template<typename> typename ItRight, 
        typename ... Us >
    friend bool operator<=( const _row_proxy< ItLeft, Us... >& l, 
                            const _row_proxy< ItRight, Us... >& r );
    template< 
        template<typename> typename ItLeft, 
        template<typename> typename ItRight, 
        typename ... Us >
    friend bool operator>=( const _row_proxy< ItLeft, Us... >& l, 
                            const _row_proxy< ItRight, Us... >& r );
    template< 
        template<typename> typename ItLeft, 
        template<typename> typename ItRight, 
        typename ... Us >
    friend bool operator<( const _row_proxy< ItLeft, Us... >& l, 
                            const _row_proxy< ItRight, Us... >& r );
    template< 
        template<typename> typename ItLeft, 
        template<typename> typename ItRight, 
        typename ... Us >
    friend bool operator>( const _row_proxy< ItLeft, Us... >& l, 
                            const _row_proxy< ItRight, Us... >& r );

    std::tuple< Iter<Ts>... > m_iters;
    std::tuple< Ts*... > m_ptrs;
};

template< 
    template<typename> typename ItLeft, 
    template<typename> typename ItRight, 
    typename ... Us >
bool operator==( const _row_proxy< ItLeft, Us... >& l, 
                 const _row_proxy< ItRight, Us... >& r )
{
    return l.m_iters == r.m_iters;
}

template< 
    template<typename> typename ItLeft, 
    template<typename> typename ItRight, 
    typename ... Us >
bool operator!=( const _row_proxy< ItLeft, Us... >& l, 
                 const _row_proxy< ItRight, Us... >& r )
{
    return l.m_iters != r.m_iters;
}

template< 
    template<typename> typename ItLeft, 
    template<typename> typename ItRight, 
    typename ... Us >
bool operator<=( const _row_proxy< ItLeft, Us... >& l, 
                 const _row_proxy< ItRight, Us... >& r )
{
    return l.m_iters <= r.m_iters;
}

template< 
    template<typename> typename ItLeft, 
    template<typename> typename ItRight, 
    typename ... Us >
bool operator>=( const _row_proxy< ItLeft, Us... >& l, 
                 const _row_proxy< ItRight, Us... >& r )
{
    return l.m_iters >= r.m_iters;
}

template< 
    template<typename> typename ItLeft, 
    template<typename> typename ItRight, 
    typename ... Us >
bool operator<( const _row_proxy< ItLeft, Us... >& l, 
                 const _row_proxy< ItRight, Us... >& r )
{
    return l.m_iters < r.m_iters;
}

template< 
    template<typename> typename ItLeft, 
    template<typename> typename ItRight, 
    typename ... Us >
bool operator>( const _row_proxy< ItLeft, Us... >& l, 
                 const _row_proxy< ItRight, Us... >& r )
{
    return l.m_iters > r.m_iters;
}

template< template<typename> class Iter, typename ... Ts >
class base_frame_iterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = ptrdiff_t;

    base_frame_iterator() = default;

    base_frame_iterator( const std::tuple<Iter<Ts> ...>& its )
        : m_row( its )
    {
    }

    void debug() const
    {
        m_row.debug();
    }

    template< size_t Ind >
    Iter<typename std::tuple_element< Ind, std::tuple< Ts ... > >::type> 
    column_iterator()
    {
        return std::get< Ind >( m_row.m_iters );
    }

    void operator++() { operator+=( 1 ); }
    void operator++(int) { operator+=( 1 ); }
    void operator--() { operator-=( 1 ); }
    void operator--(int) { operator-=( 1 ); }

    void operator+=( ptrdiff_t n )
    {
        m_row.inc( n );
    }
    void operator-=( ptrdiff_t n )
    {
        m_row.dec( n );
    }

    _row_proxy< Iter, Ts... >& operator*() { return this->m_row; }
    const _row_proxy< Iter, Ts... >& operator*() const { return this->m_row; }
    _row_proxy< Iter, Ts... >* operator->() { return &this->m_row; }
    const _row_proxy< Iter, Ts... >* operator->() const { return &this->m_row; }

protected:

    template< size_t Ind >
    typename std::tuple_element< Ind, std::tuple< Iter<Ts> ... > >::type
    get_iterator() const   
    {
        auto iter = m_row.template get_iterator<Ind>();
        return iter;
    }

    template< template<typename> typename It, typename ... Us >
    friend bool operator==( const base_frame_iterator< It, Us... >& l, 
                            const base_frame_iterator< It, Us... >& r );
    template< template<typename> typename It, typename ... Us >
    friend bool operator!=( const base_frame_iterator< It, Us... >& l, 
                            const base_frame_iterator< It, Us... >& r );
    template< template<typename> typename It, typename ... Us >
    friend bool operator<=( const base_frame_iterator< It, Us... >& l, 
                            const base_frame_iterator< It, Us... >& r );
    template< template<typename> typename It, typename ... Us >
    friend bool operator>=( const base_frame_iterator< It, Us... >& l, 
                            const base_frame_iterator< It, Us... >& r );
    template< template<typename> typename It, typename ... Us >
    friend bool operator<( const base_frame_iterator< It, Us... >& l, 
                            const base_frame_iterator< It, Us... >& r );
    template< template<typename> typename It, typename ... Us >
    friend bool operator>( const base_frame_iterator< It, Us... >& l, 
                            const base_frame_iterator< It, Us... >& r );
    template< 
        template<typename> typename ItLeft, 
        template<typename> typename ItRight, 
        typename ... Us >
    friend ptrdiff_t operator-( 
        const base_frame_iterator<ItLeft, Us...>& left,
        const base_frame_iterator<ItRight, Us...>& right );

    _row_proxy< Iter, Ts... > m_row;
};

template< 
    template<typename> typename ItLeft, 
    template<typename> typename ItRight, 
    typename ... Us >
ptrdiff_t operator-( 
    const base_frame_iterator<ItLeft, Us...>& left,
    const base_frame_iterator<ItRight, Us...>& right )
{
    auto itleft = left.template get_iterator<0>();
    auto itright = right.template get_iterator<0>();
    return itleft - itright;
}

template< typename ... Ts >
class frame_iterator : public base_frame_iterator< series_iterator, Ts... >
{
public:
    frame_iterator( const std::tuple<series_iterator<Ts> ...>& its )
        : base_frame_iterator<series_iterator, Ts...>( its )
    {}

    frame_iterator( std::tuple<series<Ts>...>& cols, int offset )
    {
        init_impl<0, Ts...>( cols, offset );
    }

    template< size_t Ind, typename U, typename ... Us >
    void init_impl( std::tuple<series<Ts>...>& cols, int offset )
    {
        series<U>& s = std::get< Ind >( cols );
        auto begin = s.begin();
        this->m_row.template set_iterator< Ind >( begin + offset );
        if constexpr ( sizeof...( Us ) > 0 ) {
            init_impl< Ind+1, Us... >( cols, offset );
        }
    }

    template< typename ... Us >
    frame_iterator< Ts..., Us... > combine( 
        const frame_iterator< Us... >& other )
    {
        auto ptr = tuple_cat( 
            this->m_row.m_iters, other->m_iters );
        frame_iterator< Ts..., Us... > out{ ptr };
        return out;
    }

    frame_iterator<Ts...> operator+( ptrdiff_t n ) const
    {
        frame_iterator<Ts...> out{ *this };
        out += n;
        return out;
    }

    frame_iterator<Ts...> operator-( ptrdiff_t n ) const
    {
        frame_iterator<Ts...> out{ *this };
        out -= n;
        return out;
    }

private:
    template< typename ... Us >
    friend class const_frame_iterator;
};

template< typename ... Ts >
class const_frame_iterator : public base_frame_iterator< const_series_iterator, Ts... >
{
public:
    const_frame_iterator( const std::tuple<const_series_iterator<Ts> ...>& its )
        : base_frame_iterator<const_series_iterator, Ts...>( its )
    {}

    const_frame_iterator( const std::tuple<series<Ts>...>& cols, int offset )
    {
        init_impl<0, Ts...>( cols, offset );
    }

    const_frame_iterator( const frame_iterator<Ts...>& fi )
        : base_frame_iterator<const_series_iterator, Ts...>( fi.m_row.m_iters )
    {
    }

    template< size_t Ind, typename U, typename ... Us >
    void init_impl( const std::tuple<series<Ts>...>& cols, int offset )
    {
        const series<U>& s = std::get< Ind >( cols );
        auto begin = s.cbegin();
        this->m_row.template set_iterator< Ind >( begin + offset );
        if constexpr ( sizeof...( Us ) > 0 ) {
            init_impl< Ind+1, Us... >( cols, offset );
        }
    }

    template< typename ... Us >
    const_frame_iterator< Ts..., Us... > combine( 
        const const_frame_iterator< Us... >& other )
    {
        auto ptr = tuple_cat( 
            this->m_row.m_iters, other->m_iters );
        const_frame_iterator< Ts..., Us... > out{ ptr };
        return out;
    }

    const_frame_iterator<Ts...> operator+( ptrdiff_t n ) const
    {
        const_frame_iterator<Ts...> out{ *this };
        out += n;
        return out;
    }

    const_frame_iterator<Ts...> operator-( ptrdiff_t n ) const
    {
        const_frame_iterator<Ts...> out{ *this };
        out -= n;
        return out;
    }
};

template< typename ... Ts >
class reverse_frame_iterator : public base_frame_iterator< reverse_series_iterator, Ts... >
{
public:
    reverse_frame_iterator( const std::tuple<reverse_series_iterator<Ts> ...>& its )
        : base_frame_iterator<reverse_series_iterator, Ts...>( its )
    {
    }

    reverse_frame_iterator( std::tuple<series<Ts>...>& cols, int offset )
    {
        init_impl<0, Ts...>( cols, offset );
    }

    template< size_t Ind, typename U, typename ... Us >
    void init_impl( std::tuple<series<Ts>...>& cols, int offset )
    {
        series<U>& s = std::get< Ind >( cols );
        auto begin = s.rbegin();
        this->m_row.template set_iterator< Ind >( begin + offset );
        if constexpr ( sizeof...( Us ) > 0 ) {
            init_impl< Ind+1, Us... >( cols, offset );
        }
    }

    template< typename ... Us >
    reverse_frame_iterator< Ts..., Us... > combine( 
        const reverse_frame_iterator< Us... >& other )
    {
        auto ptr = tuple_cat( 
            this->m_row.m_iters, other->m_iters );
        reverse_frame_iterator< Ts..., Us... > out{ ptr };
        return out;
    }

    reverse_frame_iterator<Ts...> operator+( ptrdiff_t n ) const
    {
        reverse_frame_iterator<Ts...> out{ *this };
        out += n;
        return out;
    }

    reverse_frame_iterator<Ts...> operator-( ptrdiff_t n ) const
    {
        reverse_frame_iterator<Ts...> out{ *this };
        out -= n;
        return out;
    }

private:
    template< typename ... Us >
    friend class const_reverse_frame_iterator;
};

template< typename ... Ts >
class const_reverse_frame_iterator : public base_frame_iterator< const_reverse_series_iterator, Ts... >
{
public:
    const_reverse_frame_iterator( const std::tuple<const_reverse_series_iterator<Ts> ...>& its )
        : base_frame_iterator<const_reverse_series_iterator, Ts...>( its )
    {}

    const_reverse_frame_iterator( const std::tuple<series<Ts>...>& cols, int offset )
    {
        init_impl<0, Ts...>( cols, offset );
    }

    const_reverse_frame_iterator( const reverse_frame_iterator<Ts...>& fi )
        : base_frame_iterator<const_reverse_series_iterator, Ts...>( fi.m_row.m_iters )
    {
    }

    template< size_t Ind, typename U, typename ... Us >
    void init_impl( const std::tuple<series<Ts>...>& cols, int offset )
    {
        const series<U>& s = std::get< Ind >( cols );
        auto begin = s.crbegin();
        this->m_row.template set_iterator< Ind >( begin + offset );
        if constexpr ( sizeof...( Us ) > 0 ) {
            init_impl< Ind+1, Us... >( cols, offset );
        }
    }

    template< typename ... Us >
    const_reverse_frame_iterator< Ts..., Us... > combine( 
        const const_reverse_frame_iterator< Us... >& other )
    {
        auto ptr = tuple_cat( 
            this->m_row.m_iters, other->m_iters );
        const_reverse_frame_iterator< Ts..., Us... > out{ ptr };
        return out;
    }

    const_reverse_frame_iterator<Ts...> operator+( ptrdiff_t n ) const
    {
        const_reverse_frame_iterator<Ts...> out{ *this };
        out += n;
        return out;
    }

    const_reverse_frame_iterator<Ts...> operator-( ptrdiff_t n ) const
    {
        const_reverse_frame_iterator<Ts...> out{ *this };
        out -= n;
        return out;
    }
};

template< template<typename> typename It, typename ... Us >
bool operator==( const base_frame_iterator< It, Us... >& l, 
                 const base_frame_iterator< It, Us... >& r )
{
    return l.m_row == r.m_row;
}

template< template<typename> typename It, typename ... Us >                                                          
bool operator!=( const base_frame_iterator< It, Us... >& l, 
                 const base_frame_iterator< It, Us... >& r )
{
    return l.m_row != r.m_row;
}

template< template<typename> typename It, typename ... Us >
bool operator<=( const base_frame_iterator< It, Us... >& l, 
                 const base_frame_iterator< It, Us... >& r )
{
    return l.m_row <= r.m_row;
}

template< template<typename> typename It, typename ... Us >                                                          
bool operator>=( const base_frame_iterator< It, Us... >& l, 
                 const base_frame_iterator< It, Us... >& r )
{
    return l.m_row >= r.m_row;
}

template< template<typename> typename It, typename ... Us >
bool operator<( const base_frame_iterator< It, Us... >& l, 
                 const base_frame_iterator< It, Us... >& r )
{
    return l.m_row < r.m_row;
}

template< template<typename> typename It, typename ... Us >                                                          
bool operator>( const base_frame_iterator< It, Us... >& l, 
                 const base_frame_iterator< It, Us... >& r )
{
    return l.m_row > r.m_row;
}

} // namespace mf

#endif // INCLUDED_mainframe_frame_iterator_h

