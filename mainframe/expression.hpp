
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_expression_hpp
#define INCLUDED_mainframe_expression_hpp

#include <string>

#include "mainframe/series.hpp"
#include "mainframe/frame_iterator.hpp"

namespace mf
{

namespace expr_op
{
template< typename T >
struct quickval { static T value; };

template< typename L, typename R > 
struct GT 
{
    using return_type = decltype( quickval<L>::value > quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l > r; }
};
template< typename L, typename R > 
struct GE 
{
    using return_type = decltype( quickval<L>::value >= quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l >= r; }
};
template< typename L, typename R > 
struct LT 
{
    using return_type = decltype( quickval<L>::value < quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l < r; }
};
template< typename L, typename R > 
struct LE
{
    using return_type = decltype( quickval<L>::value <= quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l <= r; }
};
template< typename L, typename R > 
struct EQ
{
    using return_type = decltype( quickval<L>::value == quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l == r; }
};
template< typename L, typename R > 
struct NE
{
    using return_type = decltype( quickval<L>::value != quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l != r; }
};
template< typename L, typename R > 
struct AND
{
    using return_type = decltype( quickval<L>::value && quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l && r; }
};
template< typename L, typename R > 
struct OR
{
    using return_type = decltype( quickval<L>::value || quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l || r; }
};
template< typename L, typename R >
struct PLUS
{
    using return_type = decltype( quickval<L>::value + quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l + r; }
};
template< typename L, typename R >
struct MINUS
{
    using return_type = decltype( quickval<L>::value - quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l - r; }
};
template< typename L, typename R >
struct MULTIPLY
{
    using return_type = decltype( quickval<L>::value * quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l * r; }
};
template< typename L, typename R >
struct DIVIDE
{
    using return_type = decltype( quickval<L>::value / quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l / r; }
};
template< typename L, typename R >
struct MODULUS
{
    using return_type = decltype( quickval<L>::value % quickval<R>::value );
    static return_type exec( const L& l, const R& r ) { return l % r; }
};
template< typename T >
struct NOT
{
    using return_type = decltype( !quickval<T>::value );
    static return_type exec( const T& t ) { return !t; }
};
template< typename T >
struct NEGATE
{
    using return_type = decltype( -quickval<T>::value );
    static return_type exec( const T& t ) { return -t; }
};

} // namespace expr_op

template<typename T, typename V = void>
struct is_expression;

template<typename T, typename V>
struct is_expression
  : std::false_type
{};

template<typename T>
struct is_expression<T, typename T::is_expr>
  : std::true_type
{};

template<typename T>
struct is_expression<T &, void>
  : is_expression<T>
{};

template< typename T >
struct column_name
{
    column_name() 
        : m_colnum( std::numeric_limits<decltype(m_colnum)>::max )
    {
    }

    void set_column_name( size_t col, std::string name )
    {
        if ( name == m_name ) {
            m_colnum = col;
        }
    }

    template< typename ... Ts >
    const T& get_value( const frame_iterator< Ts... >& fi ) const
    {
        return fi.get( m_colnum );
    }

    template< typename ... Ts >
    T& get_value( frame_iterator< Ts... >& fi ) const
    {
        return fi.get( m_colnum );
    }

    std::string m_name;
    size_t m_colnum;
};

template< typename T >
struct terminal
{
    using is_expr = void;
    using return_type = T;

    terminal( T _t ) : t( _t ) {}

    void set_column_name( size_t, std::string )
    {
    }

    template< typename ... Ts >
    const T& get_value( const frame_iterator<Ts...>& )
    {
        return t;
    }

    template< typename ... Ts >
    T& get_value( const frame_iterator<Ts...>& )
    {
        return t;
    }

    T t;
};

template< typename T >
struct terminal< column_name<T> >
{
    using is_expr = void;
    using return_type = T;

    terminal( column_name<T> _t ) : t( _t ) {}

    void set_column_name( size_t colind, std::string colname )
    {
        t.set_column_name( colind, colname );
    }

    template< typename ... Ts >
    const T& get_value( const frame_iterator< Ts... >& fi ) const
    {
        return t.get_value( fi );
    }

    template< typename ... Ts >
    T& get_value( frame_iterator< Ts... >& fi )
    {
        return t.get_value( fi );
    }

    column_name<T> t;
};

template< template <typename, typename> class Op, typename L, typename R >
struct binary_expr
{
    using is_expr = void;
    using op = Op<typename L::return_type, typename R::return_type>;
    using return_type = typename op::return_type;
    static_assert( is_expression<L>::value , "binary expression left must be expression" );
    static_assert( is_expression<R>::value , "binary expression right must be expression" );

    binary_expr( L _l, R _r ) : l( _l ), r( _r ) {}

    void set_column_name( size_t colind, std::string colname )
    {
        l.set_column_name( colind, colname );
        r.set_column_name( colind, colname );
    }

    template< typename ... Ts >
    const return_type& get_value( const frame_iterator< Ts... >& fi ) const
    {
        return op::exec( l.get_value( fi ), r.get_value( fi ) );
    }

    template< typename ... Ts >
    return_type& get_value( frame_iterator< Ts... >& fi )
    {
        return op::exec( l.get_value( fi ), r.get_value( fi ) );
    }

    L l;
    R r;
};

// Op is a member type in expr_op
// T must be either terminal<>, unary_expr<>, or binary_expr<> (no 
// unwrapped types - that's why make_unary_expr exists)
template< template <typename> class Op, typename T >
struct unary_expr
{
    using is_expr = void;
    using op = Op<typename T::return_type>;
    using return_type = typename op::return_type;
    static_assert( is_expression<T>::value , "unary expression must contain expression");

    unary_expr( T _t ) : t( _t ) {}

    void set_column_name( size_t colind, std::string colname )
    {
        t.set_column_name( colind, colname );
    }

    template< typename ... Ts >
    const return_type& get_value( const frame_iterator< Ts... >& fi ) const
    {
        return op::exec( t.get_value( fi ) );
    }

    template< typename ... Ts >
    return_type& get_value( frame_iterator< Ts... >& fi )
    {
        return op::exec( t.get_value( fi ) );
    }

    T t;
};

// If T is terminal<U>, unary_expr<Op,U> or binary_expr<Op,L,R>, just return T.
// If T is any other type, return terminal<T>
template<typename T>
struct maybe_wrap 
{
    maybe_wrap() = delete;
    using type = terminal< T >;
    static type wrap( const T& t )
    {
        type out{ t };
        return out;
    }
};

template<typename T>
struct maybe_wrap<terminal<T>>
{
    maybe_wrap() = delete;
    using type = terminal<T>;
    static type wrap( const terminal<T>& t )
    {
        return t;
    }
};

template<template <typename> class Op, typename T>
struct maybe_wrap<unary_expr<Op, T>>
{
    maybe_wrap() = delete;
    using type = unary_expr<Op, T>;
    static type wrap( const unary_expr<Op, T>& t )
    {
        return t;
    }
};

template<template <typename, typename> class Op, typename L, typename R>
struct maybe_wrap<binary_expr<Op, L, R>>
{
    maybe_wrap() = delete;
    using type = binary_expr<Op, L, R>;
    static type wrap( const binary_expr<Op, L, R>& t )
    {
        return t;
    }
};

template< template <typename> class Op, typename T >
struct make_unary_expr
{
    make_unary_expr() = delete;
    using type = unary_expr< Op, typename maybe_wrap< T >::type >;

    static type create( const T& t )
    {
        type out{ maybe_wrap< T >::wrap( t ) };
        return out;
    }
};

template< template <typename, typename> class Op, typename L, typename R >
struct make_binary_expr
{
    make_binary_expr() = delete;
    using type = binary_expr< Op, typename maybe_wrap< L >::type, typename maybe_wrap< R >::type >;

    static type create( const L& l, const R& r )
    {
        type out{ maybe_wrap< L >::wrap( l ), maybe_wrap< R >::wrap( r ) };
        return out;
    }
};

template< typename T >
terminal<column_name<T>> col( const char * colname )
{
    column_name<T> c;
    c.name = colname;
    return maybe_wrap<column_name<T>>::wrap( c );
}

template< typename L, typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::LT, L, R>::type>::type operator<( L l, R r )
{
    return make_binary_expr<expr_op::LT, L, R>::create( l, r );
}

template< typename L, typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::GT, L, R>::type>::type operator>( L l, R r )
{
    return make_binary_expr<expr_op::GT, L, R>::create( l, r );
}

template< typename L, typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::LE, L, R>::type>::type operator<=( L l, R r )
{
    return make_binary_expr<expr_op::LE, L, R>::create( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::GE, L, R>::type>::type operator>=( L l, R r )
{
    return make_binary_expr<expr_op::GE, L, R>::create( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::AND, L, R>::type>::type operator&&( L l, R r )
{
    return make_binary_expr<expr_op::AND, L, R>::create( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::OR, L, R>::type>::type operator||( L l, R r )
{
    return make_binary_expr<expr_op::OR, L, R>::create( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::EQ, L, R>::type>::type operator==( L l, R r )
{
    return make_binary_expr<expr_op::EQ, L, R>::create( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::NE, L, R>::type>::type operator!=( L l, R r )
{
    return make_binary_expr<expr_op::NE, L, R>::create( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::PLUS, L, R>::type>::type operator+( L l, R r )
{
    return make_binary_expr<expr_op::PLUS, L, R>::create( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::MINUS, L, R>::type>::type operator-( L l, R r )
{
    return make_binary_expr<expr_op::MINUS, L, R>::create( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::MULTIPLY, L, R>::type>::type operator*( L l, R r )
{
    return make_binary_expr<expr_op::MULTIPLY, L, R>::create( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::DIVIDE, L, R>::type>::type operator/( L l, R r )
{
    return make_binary_expr<expr_op::DIVIDE, L, R>::create( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::MODULUS, L, R>::type>::type operator%( L l, R r )
{
    return make_binary_expr<expr_op::MODULUS, L, R>::create( l, r );
}

template< typename T >
typename std::enable_if<is_expression<T>::value, typename make_unary_expr<expr_op::NEGATE, T>::type>::type operator-( T t )
{
    return make_unary_expr<expr_op::NEGATE, T>::create( t );
}

template< typename T >
typename std::enable_if<is_expression<T>::value, typename make_unary_expr<expr_op::NOT, T>::type>::type operator!( T t )
{
    return make_unary_expr<expr_op::NOT, T>::create( t );
}

template< typename T >
terminal< column_name<T> > col( const std::string & colname )
{
    return terminal< column_name<T> >{ column_name<T>{ colname } };
}


} // namespace mf


#endif // INCLUDED_mainframe_expression_hpp
