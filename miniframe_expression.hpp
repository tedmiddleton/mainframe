
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_miniframe_expression_hpp
#define INCLUDED_miniframe_expression_hpp

#include <string>

namespace mf
{

template<typename T>
struct terminal
{
    terminal( T _t ) : t( _t ) {}

    using is_expr = void;
    T t;
};

template<typename OP, typename L, typename R >
struct binary_expr
{
    using is_expr = void;
    using op = OP;

    binary_expr( L _l, R _r ) : l( _l ), r( _r ) {}
    L l;
    R r;
};

struct column_name
{
    column_name( std::string _name ): name( _name ) {}
    std::string name;
};

struct op_gt {};
struct op_ge {};
struct op_lt {};
struct op_le {};
struct op_eq {};
struct op_ne {};
struct op_and {};
struct op_or {};

template<typename T>
struct make_terminal 
{
    using type = terminal<T>;
    terminal<T> operator()( const T& t )
    {
        return terminal<T>{ .t=t };
    }
};

template<typename T>
struct make_terminal<terminal<T>>
{
    using type = terminal<T>;
    terminal<T> operator()( const terminal<T>& t )
    {
        return t;
    }
};

template<typename Op, typename L, typename R>
struct make_terminal<binary_expr<Op, L, R>>
{
    using type = binary_expr<Op, L, R>;
    binary_expr<Op, L, R> operator()( const binary_expr<Op, L, R>& t )
    {
        return t;
    }
};

template<typename Op, typename L, typename R>
struct make_expr
{
    using ltype = typename make_terminal<L>::type;
    using rtype = typename make_terminal<R>::type;
    using type = binary_expr<Op, ltype, rtype>;

    type operator()( L l, R r )
    {
        return type{ 
            .l=make_terminal<L>{}(l), 
            .r=make_terminal<R>{}(r) };
    }
};

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

template< typename L, typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_expr<op_lt, L, R>::type>::type operator<( L l, R r )
{
    return make_expr<op_lt, L, R>{}( l, r );
}

template< typename L, typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_expr<op_gt, L, R>::type>::type operator>( L l, R r )
{
    return make_expr<op_gt, L, R>{}( l, r );
}

template< typename L, typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_expr<op_le, L, R>::type>::type operator<=( L l, R r )
{
    return make_expr<op_le, L, R>{}( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_expr<op_ge, L, R>::type>::type operator>=( L l, R r )
{
    return make_expr<op_ge, L, R>{}( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_expr<op_and, L, R>::type>::type operator&&( L l, R r )
{
    return make_expr<op_and, L, R>{}( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_expr<op_or, L, R>::type>::type operator||( L l, R r )
{
    return make_expr<op_or, L, R>{}( l, r );
}

template< typename L,typename R >
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value, typename make_expr<op_eq, L, R>::type>::type operator==( L l, R r )
{
    return make_expr<op_eq, L, R>{}( l, r );
}

//terminal<column_name> col( std::string colname )
//{
//    return terminal<column_name>{ column_name{ colname } };
//}


} // namespace mf


#endif // INCLUDED_miniframe_expression_hpp
