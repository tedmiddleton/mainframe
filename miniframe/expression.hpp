
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_miniframe_expression_hpp
#define INCLUDED_miniframe_expression_hpp

#include <string>

#include "miniframe/series.hpp"

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
struct column
{
    column() 
    {
        begin = end;
    }

    bool bind_vec( const std::string& n, const std::vector<T>& vec )
    {
        if ( name == n ) {
            begin = vec.begin();
            end = vec.end();
            return true;
        }
        return false;
    }

    const T& get_value( size_t row ) const
    {
        if ( end>begin && row < static_cast<size_t>(end-begin) ) {
            return *(begin + row);
        } 
        else {
            throw std::out_of_range{ "out of range" };
        }
    }

    std::string name;
    typename std::vector<T>::const_iterator begin;
    typename std::vector<T>::const_iterator end;
};

template< typename T >
struct terminal
{
    using is_expr = void;
    using return_type = T;

    terminal( T _t ) : t( _t ) {}

    template< typename U >
    bool bind_vec( const std::string& /*name*/, const std::vector<U>& /*vec*/ )
    {
        return false;
    }

    const T& get_value( size_t /*row*/ ) const
    {
        return t;
    }

    T t;
};

template< typename T >
struct terminal< column<T> >
{
    using is_expr = void;
    using return_type = T;

    terminal( column<T> _t ) : t( _t ) {}

    bool bind_vec( const std::string& name, const std::vector<T>& vec )
    {
        return t.bind_vec( name, vec );
    }

    const T& get_value( size_t row ) const
    {
        return t.get_value( row );
    }

    column<T> t;
};

template< template <typename, typename> typename Op, typename L, typename R >
struct binary_expr
{
    using is_expr = void;
    using op = Op<typename L::return_type, typename R::return_type>;
    using return_type = typename op::return_type;
    static_assert( is_expression<L>::value );
    static_assert( is_expression<R>::value );

    binary_expr( L _l, R _r ) : l( _l ), r( _r ) {}

    template< typename T >
    bool bind_vec( const std::string& name, const std::vector<T>& vec )
    {
        auto lb = l.bind_vec( name, vec );
        auto rb = r.bind_vec( name, vec );
        return lb || rb;
    }

    return_type get_value( size_t rowidx ) const
    {
        return op::exec( l.get_value( rowidx ), r.get_value( rowidx ) );
    }

    L l;
    R r;
};

// Op is a member type in expr_op
// T must be either terminal<>, unary_expr<>, or binary_expr<> (no 
// unwrapped types - that's why make_unary_expr exists)
template< template <typename> typename Op, typename T >
struct unary_expr
{
    using is_expr = void;
    using op = Op<typename T::return_type>;
    using return_type = typename op::return_type;
    static_assert( is_expression<T>::value );

    unary_expr( T _t ) : t( _t ) {}

    template< typename U >
    bool bind_vec( const std::string& name, const std::vector<U>& vec )
    {
        return t.bind_vec( name, vec );
    }

    return_type get_value( size_t rowidx ) const
    {
        return op::exec( t.get_value( rowidx ) );
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
        type out;
        out.t = t;
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

template<template <typename> typename Op, typename T>
struct maybe_wrap<unary_expr<Op, T>>
{
    maybe_wrap() = delete;
    using type = unary_expr<Op, T>;
    static type wrap( const unary_expr<Op, T>& t )
    {
        return t;
    }
};

template<template <typename, typename> typename Op, typename L, typename R>
struct maybe_wrap<binary_expr<Op, L, R>>
{
    maybe_wrap() = delete;
    using type = binary_expr<Op, L, R>;
    static type wrap( const binary_expr<Op, L, R>& t )
    {
        return t;
    }
};

template< template <typename> typename Op, typename T >
struct make_unary_expr
{
    make_unary_expr() = delete;
    using type = unary_expr< Op, typename maybe_wrap< T >::type >;

    static type create( const T& t )
    {
        type out;
        out.t = maybe_wrap< T >::wrap( t );
        return out;
    }
};

template< template <typename, typename> typename Op, typename L, typename R >
struct make_binary_expr
{
    make_binary_expr() = delete;
    using type = binary_expr< Op, typename maybe_wrap< L >::type, typename maybe_wrap< R >::type >;

    static type create( const L& l, const R& r )
    {
        type out;
        out.l = maybe_wrap< L >::wrap( l );
        out.r = maybe_wrap< R >::wrap( r );
        return out;
    }
};

template< typename T >
terminal<column<T>> col( const char * colname )
{
    column<T> c;
    c.name = colname;
    return maybe_wrap<column<T>>::wrap( c );
}

template< typename L, typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::LT, L, R>::type>::type operator<( L l, R r )
{
    return make_binary_expr<expr_op::LT, L, R>{}( l, r );
}

template< typename L, typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::GT, L, R>::type>::type operator>( L l, R r )
{
    return make_binary_expr<expr_op::GT, L, R>{}( l, r );
}

template< typename L, typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::LE, L, R>::type>::type operator<=( L l, R r )
{
    return make_binary_expr<expr_op::LE, L, R>{}( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::GE, L, R>::type>::type operator>=( L l, R r )
{
    return make_binary_expr<expr_op::GE, L, R>{}( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::AND, L, R>::type>::type operator&&( L l, R r )
{
    return make_binary_expr<expr_op::AND, L, R>{}( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::OR, L, R>::type>::type operator||( L l, R r )
{
    return make_binary_expr<expr_op::OR, L, R>{}( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_binary_expr<expr_op::EQ, L, R>::type>::type operator==( L l, R r )
{
    return make_binary_expr<expr_op::EQ, L, R>{}( l, r );
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

//terminal<column_name> col( std::string colname )
//{
//    return terminal<column_name>{ column_name{ colname } };
//}


} // namespace mf


#endif // INCLUDED_miniframe_expression_hpp
