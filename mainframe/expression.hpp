
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

template< typename T >
struct quickval { static T value; };
namespace expr_op
{
struct GT 
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l > r )
    { return l > r; }
};
struct GE 
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l >= r )
    { return l >= r; }
};
struct LT 
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l < r )
    { return l < r; }
};
struct LE
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l <= r )
    { return l <= r; }
};
struct EQ
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l == r )
    { return l == r; }
};
struct NE
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l != r )
    { return l != r; }
};
struct AND
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l && r )
    { return l && r; }
};
struct OR
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l || r )
    { return l || r; }
};
struct PLUS
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l + r )
    { return l + r; }
};
struct MINUS
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l - r )
    { return l - r; }
};
struct MULTIPLY
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l * r )
    { return l * r; }
};
struct DIVIDE
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l / r )
    { return l / r; }
};
struct MODULUS
{
    template< typename L, typename R >
    static auto exec( const L& l, const R& r ) -> decltype( l % r )
    { return l % r; }
};
struct NOT
{
    template< typename T >
    static auto exec( const T& t ) -> decltype( !t )
    { return !t; }
};
struct NEGATE
{
    template< typename T >
    static auto exec( const T& t ) -> decltype( -t )
    { return -t; }
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

template< size_t Ind >
struct column
{
    static const size_t index = Ind;

    template< typename ... Ts >
    typename std::tuple_element<Ind, std::tuple<Ts...>>::type 
    get_value( const const_frame_iterator< Ts... >& fi ) const
    {
        return fi->template get<Ind>();
    }
};

template< typename T >
struct terminal
{
    using is_expr = void;

    terminal( T _t ) : t( _t ) {}

    template< typename ... Ts >
    T get_value( const const_frame_iterator<Ts...>& )
    {
        return t;
    }

    T t;
};

template< size_t Ind >
struct terminal< column<Ind> >
{
    using is_expr = void;

    terminal( column<Ind> _t ) : t( _t ) {}

    template< typename ... Ts >
    typename std::tuple_element<Ind, std::tuple<Ts...>>::type 
    get_value( const_frame_iterator< Ts... >& fi )
    {
        return t.get_value( fi );
    }

    column<Ind> t;
};

template< typename Op, typename L, typename R >
struct binary_expr
{
    using is_expr = void;
    static_assert( is_expression<L>::value , "binary expression left must be expression" );
    static_assert( is_expression<R>::value , "binary expression right must be expression" );

    binary_expr( L _l, R _r ) : l( _l ), r( _r ) {}

    template< typename ... Ts >
    auto get_value( const_frame_iterator< Ts... >& fi ) -> 
    decltype( Op::exec( quickval<L>::value.get_value( fi ), quickval<R>::value.get_value( fi ) ) )
    {
        return Op::exec( l.get_value( fi ), r.get_value( fi ) );
    }

    L l;
    R r;
};

// Op is a member type in expr_op
// T must be either terminal<>, unary_expr<>, or binary_expr<> (no 
// unwrapped types - that's why make_unary_expr exists)
template< typename Op, typename T >
struct unary_expr
{
    using is_expr = void;
    static_assert( is_expression<T>::value , "unary expression must contain expression");

    unary_expr( T _t ) : t( _t ) {}

    template< typename ... Ts >
    auto get_value( const_frame_iterator< Ts... >& fi ) -> 
    decltype( Op::exec( quickval<T>::value.get_value( fi ) ) )
    {
        return Op::exec( t.get_value( fi ) );
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

template<typename Op, typename T>
struct maybe_wrap<unary_expr<Op, T>>
{
    maybe_wrap() = delete;
    using type = unary_expr<Op, T>;
    static type wrap( const unary_expr<Op, T>& t )
    {
        return t;
    }
};

template<typename Op, typename L, typename R>
struct maybe_wrap<binary_expr<Op, L, R>>
{
    maybe_wrap() = delete;
    using type = binary_expr<Op, L, R>;
    static type wrap( const binary_expr<Op, L, R>& t )
    {
        return t;
    }
};

template< typename Op, typename T >
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

template< typename Op, typename L, typename R >
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

template< size_t Ind >
terminal<column<Ind>> col()
{
    return terminal<column<Ind>>{ column<Ind>{} };
}

} // namespace mf


#endif // INCLUDED_mainframe_expression_hpp
