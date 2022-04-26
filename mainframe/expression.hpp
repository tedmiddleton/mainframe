
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
struct expr_column
{
    static const size_t index = Ind;

    expr_column() = default;
    expr_column( int o ) : offset( o ) {}
    expr_column( const expr_column& ) = default;
    expr_column( expr_column&& ) = default;
    expr_column& operator=( const expr_column& ) = default;
    expr_column& operator=( expr_column&& ) = default;

    expr_column operator[]( int off ) const
    {
        return expr_column{ off };
    }

    template< template<typename> class Iter, typename ... Ts >
    typename pack_element<Ind, Ts...>::type 
    get_value( 
        const base_frame_iterator< Iter, Ts... >& ,
        const base_frame_iterator< Iter, Ts... >& curr, 
        const base_frame_iterator< Iter, Ts... >& 
        ) const
    {
        return curr->template at<Ind>();
        //auto adjusted = curr + offset;
        //if ( begin <= adjusted && adjusted < end ) {
        //    return adjusted->template at<Ind>();
        //} 
        //else {
        //    using T = typename pack_element<Ind, Ts...>::type;
        //    // default-construct. If std::optional, this will be nullopt
        //    return T{};
        //}
    }

    int offset{ 0 };
};

template< typename T >
struct terminal
{
    using is_expr = void;

    terminal( T _t ) : t( _t ) {}

    template< template<typename> class Iter, typename ... Ts >
    T get_value( 
        const base_frame_iterator< Iter, Ts... >&,
        const base_frame_iterator< Iter, Ts... >&,
        const base_frame_iterator< Iter, Ts... >& 
        ) const
    {
        return t;
    }

    T t;
};

template< size_t Ind >
struct terminal< expr_column<Ind> >
{
    using is_expr = void;
    static const size_t index = Ind;

    terminal() = default;
    terminal( expr_column<Ind> _t ) : t( _t ) {}

    template< template<typename> class Iter, typename ... Ts >
    typename std::tuple_element<Ind, std::tuple<Ts...>>::type 
    get_value( 
        const base_frame_iterator< Iter, Ts... >& begin,
        const base_frame_iterator< Iter, Ts... >& curr,
        const base_frame_iterator< Iter, Ts... >& end 
        ) const
    {
        return t.get_value( begin, curr, end );
    }

    terminal< expr_column<Ind> > operator[]( int off ) const
    {
        return terminal{ t[ off ] };
    }

    expr_column<Ind> t;
};

template< typename Op, typename L, typename R >
struct binary_expr
{
    using is_expr = void;
    static_assert( is_expression<L>::value , "binary expression left must be expression" );
    static_assert( is_expression<R>::value , "binary expression right must be expression" );

    binary_expr( L _l, R _r ) : l( _l ), r( _r ) {}

    template< template<typename> class Iter, typename ... Ts >
    auto get_value( 
        const base_frame_iterator< Iter, Ts... >& begin,
        const base_frame_iterator< Iter, Ts... >& curr,
        const base_frame_iterator< Iter, Ts... >& end 
        ) const -> 
    decltype( Op::exec( 
            quickval<L>::value.get_value( begin, curr, end ), 
            quickval<R>::value.get_value( begin, curr, end ) ) )
    {
        return Op::exec( 
            l.get_value( begin, curr, end ), 
            r.get_value( begin, curr, end ) );
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

    template< template<typename> class Iter, typename ... Ts >
    auto get_value( 
        const base_frame_iterator< Iter, Ts... >& begin,
        const base_frame_iterator< Iter, Ts... >& curr, 
        const base_frame_iterator< Iter, Ts... >& end 
        ) const -> 
    decltype( Op::exec( quickval<T>::value.get_value( curr ) ) )
    {
        return Op::exec( t.get_value( begin, curr, end ) );
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
terminal<expr_column<Ind>> col()
{
    return terminal<expr_column<Ind>>{ expr_column<Ind>{} };
}

namespace placeholders
{
    inline constexpr terminal<expr_column<0>> _0;
    inline constexpr terminal<expr_column<1>> _1;
    inline constexpr terminal<expr_column<2>> _2;
    inline constexpr terminal<expr_column<3>> _3;
    inline constexpr terminal<expr_column<4>> _4;
    inline constexpr terminal<expr_column<5>> _5;
    inline constexpr terminal<expr_column<6>> _6;
    inline constexpr terminal<expr_column<7>> _7;
    inline constexpr terminal<expr_column<8>> _8;
    inline constexpr terminal<expr_column<9>> _9;
    inline constexpr terminal<expr_column<10>> _10;
    inline constexpr terminal<expr_column<11>> _11;
    inline constexpr terminal<expr_column<12>> _12;
    inline constexpr terminal<expr_column<13>> _13;
    inline constexpr terminal<expr_column<14>> _14;
    inline constexpr terminal<expr_column<15>> _15;
    inline constexpr terminal<expr_column<16>> _16;
    inline constexpr terminal<expr_column<17>> _17;
    inline constexpr terminal<expr_column<18>> _18;
    inline constexpr terminal<expr_column<19>> _19;
    inline constexpr terminal<expr_column<20>> _20;
    inline constexpr terminal<expr_column<21>> _21;
    inline constexpr terminal<expr_column<22>> _22;
    inline constexpr terminal<expr_column<23>> _23;
    inline constexpr terminal<expr_column<24>> _24;
    inline constexpr terminal<expr_column<25>> _25;
    inline constexpr terminal<expr_column<26>> _26;
    inline constexpr terminal<expr_column<27>> _27;
    inline constexpr terminal<expr_column<28>> _28;
    inline constexpr terminal<expr_column<29>> _29;
    inline constexpr terminal<expr_column<30>> _30;
    inline constexpr terminal<expr_column<31>> _31;
    inline constexpr terminal<expr_column<32>> _32;
    inline constexpr terminal<expr_column<33>> _33;
    inline constexpr terminal<expr_column<34>> _34;
    inline constexpr terminal<expr_column<35>> _35;
    inline constexpr terminal<expr_column<36>> _36;
    inline constexpr terminal<expr_column<37>> _37;
    inline constexpr terminal<expr_column<38>> _38;
    inline constexpr terminal<expr_column<39>> _39;
    inline constexpr terminal<expr_column<0>> col0;
    inline constexpr terminal<expr_column<1>> col1;
    inline constexpr terminal<expr_column<2>> col2;
    inline constexpr terminal<expr_column<3>> col3;
    inline constexpr terminal<expr_column<4>> col4;
    inline constexpr terminal<expr_column<5>> col5;
    inline constexpr terminal<expr_column<6>> col6;
    inline constexpr terminal<expr_column<7>> col7;
    inline constexpr terminal<expr_column<8>> col8;
    inline constexpr terminal<expr_column<9>> col9;
    inline constexpr terminal<expr_column<10>> col10;
    inline constexpr terminal<expr_column<11>> col11;
    inline constexpr terminal<expr_column<12>> col12;
    inline constexpr terminal<expr_column<13>> col13;
    inline constexpr terminal<expr_column<14>> col14;
    inline constexpr terminal<expr_column<15>> col15;
    inline constexpr terminal<expr_column<16>> col16;
    inline constexpr terminal<expr_column<17>> col17;
    inline constexpr terminal<expr_column<18>> col18;
    inline constexpr terminal<expr_column<19>> col19;
    inline constexpr terminal<expr_column<20>> col20;
    inline constexpr terminal<expr_column<21>> col21;
    inline constexpr terminal<expr_column<22>> col22;
    inline constexpr terminal<expr_column<23>> col23;
    inline constexpr terminal<expr_column<24>> col24;
    inline constexpr terminal<expr_column<25>> col25;
    inline constexpr terminal<expr_column<26>> col26;
    inline constexpr terminal<expr_column<27>> col27;
    inline constexpr terminal<expr_column<28>> col28;
    inline constexpr terminal<expr_column<29>> col29;
    inline constexpr terminal<expr_column<30>> col30;
    inline constexpr terminal<expr_column<31>> col31;
    inline constexpr terminal<expr_column<32>> col32;
    inline constexpr terminal<expr_column<33>> col33;
    inline constexpr terminal<expr_column<34>> col34;
    inline constexpr terminal<expr_column<35>> col35;
    inline constexpr terminal<expr_column<36>> col36;
    inline constexpr terminal<expr_column<37>> col37;
    inline constexpr terminal<expr_column<38>> col38;
    inline constexpr terminal<expr_column<39>> col39;

} // namespace placeholders

} // namespace mf


#endif // INCLUDED_mainframe_expression_hpp
