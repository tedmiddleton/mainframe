//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_detail_expression_hpp
#define INCLUDED_mainframe_detail_expression_hpp

#include <string>

#include "mainframe/frame_iterator.hpp"
#include "mainframe/series.hpp"

namespace mf
{

namespace expr_op
{
struct GT
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l > r)
    {
        return l > r;
    }
};
struct GE
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l >= r)
    {
        return l >= r;
    }
};
struct LT
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l < r)
    {
        return l < r;
    }
};
struct LE
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l <= r)
    {
        return l <= r;
    }
};
struct EQ
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l == r)
    {
        return l == r;
    }
};
struct NE
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l != r)
    {
        return l != r;
    }
};
struct AND
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l && r)
    {
        return l && r;
    }
};
struct OR
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l || r)
    {
        return l || r;
    }
};
struct PLUS
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l + r)
    {
        return l + r;
    }
};
struct MINUS
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l - r)
    {
        return l - r;
    }
};
struct MULTIPLY
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l * r)
    {
        return l * r;
    }
};
struct DIVIDE
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l / r)
    {
        return l / r;
    }
};
struct MODULUS
{
    template<typename L, typename R>
    static auto
    exec(const L& l, const R& r) -> decltype(l % r)
    {
        return l % r;
    }
};
struct NOT
{
    template<typename T>
    static auto
    exec(const T& t) -> decltype(!t)
    {
        return !t;
    }
};
struct NEGATE
{
    template<typename T>
    static auto
    exec(const T& t) -> decltype(-t)
    {
        return -t;
    }
};

} // namespace expr_op

template<typename T, typename V = void>
struct is_expression;

template<typename T, typename V>
struct is_expression : std::false_type
{};

template<typename T>
struct is_expression<T, typename T::is_expr> : std::true_type
{};

template<typename T>
struct is_expression<T&, void> : is_expression<T>
{};

template<typename Op, typename T>
struct unary_expr;

template<typename Op, typename L, typename R>
struct binary_expr;

template<typename Func, typename... As>
struct func_expr;

template<typename T>
struct is_complex_expression : std::false_type
{};

template<typename Op, typename T>
struct is_complex_expression<unary_expr<Op, T>> : std::true_type
{};

template<typename Op, typename L, typename R>
struct is_complex_expression<binary_expr<Op, L, R>> : std::true_type
{};

template<typename Func, typename... As>
struct is_complex_expression<func_expr<Func, As...>> : std::true_type
{};

template<size_t Ind>
struct indexed_expr_column;

struct _empty_byte
{};

struct row_number
{
    _empty_byte _eb;
};

struct frame_length
{
    _empty_byte _eb;
};

template<size_t Ind>
struct expr_column
{
    static const size_t index = Ind;

    constexpr expr_column(){};
    expr_column(const expr_column&)            = default;
    expr_column(expr_column&&)                 = default;
    expr_column& operator=(const expr_column&) = default;
    expr_column& operator=(expr_column&&)      = default;

    indexed_expr_column<Ind>
    operator[](ptrdiff_t off) const
    {
        return indexed_expr_column<Ind>{ off };
    }

    template<template<bool, bool, typename...> typename Iter, bool IsConst, bool IsReverse,
        typename... Ts>
    typename detail::pack_element<Ind, Ts...>::type
    operator()(const Iter<IsConst, IsReverse, Ts...>&, const Iter<IsConst, IsReverse, Ts...>& curr,
        const Iter<IsConst, IsReverse, Ts...>&) const
    {
        return curr->template at<Ind>();
    }
    _empty_byte _eb;
};

template<size_t Ind>
struct indexed_expr_column
{
    static const size_t index = Ind;

    indexed_expr_column() = default;
    explicit indexed_expr_column(ptrdiff_t o)
        : offset(o)
    {}
    indexed_expr_column(const indexed_expr_column&)            = default;
    indexed_expr_column(indexed_expr_column&&)                 = default;
    indexed_expr_column& operator=(const indexed_expr_column&) = default;
    indexed_expr_column& operator=(indexed_expr_column&&)      = default;

    template<template<bool, bool, typename...> typename Iter, bool IsConst, bool IsReverse,
        typename... Ts>
    typename detail::ensure_missing<typename detail::pack_element<Ind, Ts...>::type>::type
    operator()(const Iter<IsConst, IsReverse, Ts...>& begin,
        const Iter<IsConst, IsReverse, Ts...>& curr,
        const Iter<IsConst, IsReverse, Ts...>& end) const
    {
        using T       = typename detail::pack_element<Ind, Ts...>::type;
        auto adjusted = curr + offset;
        if (begin <= adjusted && adjusted < end) {
            if constexpr (detail::is_missing<T>::value) {
                return adjusted->template at<Ind>();
            }
            else {
                return mi<T>{ adjusted->template at<Ind>() };
            }
        }
        else {
            return missing;
        }
    }

    ptrdiff_t offset{ 0 };
};

template<typename T>
struct terminal
{
    using is_expr = void;

    explicit terminal(T _t)
        : t(_t)
    {}

    template<template<bool, bool, typename...> typename Iter, bool IsConst, bool IsReverse,
        typename... Ts>
    T
    operator()(const Iter<IsConst, IsReverse, Ts...>&, const Iter<IsConst, IsReverse, Ts...>&,
        const Iter<IsConst, IsReverse, Ts...>&) const
    {
        return t;
    }

    T t;
};

template<size_t Ind>
struct terminal<indexed_expr_column<Ind>>
{
    using is_expr             = void;
    static const size_t index = Ind;

    terminal() = default;
    explicit terminal(indexed_expr_column<Ind> _t)
        : t(_t)
    {}

    template<template<bool, bool, typename...> typename Iter, bool IsConst, bool IsReverse,
        typename... Ts>
    typename detail::ensure_missing<typename detail::pack_element<Ind, Ts...>::type>::type
    operator()(const Iter<IsConst, IsReverse, Ts...>& begin,
        const Iter<IsConst, IsReverse, Ts...>& curr,
        const Iter<IsConst, IsReverse, Ts...>& end) const
    {
        return t.operator()(begin, curr, end);
    }

    indexed_expr_column<Ind> t;
};

template<size_t... Inds>
struct columnindexpack;

template<size_t Ind>
struct terminal<expr_column<Ind>>
{
    using is_expr             = void;
    static const size_t index = Ind;

    terminal() = default;
    explicit terminal(expr_column<Ind>) {}

    template<template<bool, bool, typename...> typename Iter, bool IsConst, bool IsReverse,
        typename... Ts>
    typename detail::pack_element<Ind, Ts...>::type
    operator()(const Iter<IsConst, IsReverse, Ts...>& begin,
        const Iter<IsConst, IsReverse, Ts...>& curr,
        const Iter<IsConst, IsReverse, Ts...>& end) const
    {
        expr_column<Ind> t;
        return t.operator()(begin, curr, end);
    }

    terminal<indexed_expr_column<Ind>>
    operator[](ptrdiff_t off) const
    {
        expr_column<Ind> t;
        return terminal<indexed_expr_column<Ind>>{ t[off] };
    }

    template<size_t Ind2>
    columnindexpack<Ind, Ind2>
    operator,(terminal<expr_column<Ind2>> ci) const
    {
        return columnindexpack<Ind, Ind2>(*this, ci);
    }

    _empty_byte _eb;
};

template<size_t... Inds>
struct columnindexpack
{
    columnindexpack() {}
    columnindexpack(terminal<expr_column<Inds>>...) {}

    template<size_t NewInd>
    columnindexpack<Inds..., NewInd>
    operator,(terminal<expr_column<NewInd>>) const
    {
        return columnindexpack<Inds..., NewInd>();
    }
};

template<>
struct terminal<row_number>
{
    using is_expr = void;

    terminal() = default;

    template<template<bool, bool, typename...> typename Iter, bool IsConst, bool IsReverse,
        typename... Ts>
    ptrdiff_t
    operator()(const Iter<IsConst, IsReverse, Ts...>& begin,
        const Iter<IsConst, IsReverse, Ts...>& curr,
        const Iter<IsConst, IsReverse, Ts...>& /*end*/) const
    {
        return curr - begin;
    }
};

template<>
struct terminal<frame_length>
{
    using is_expr = void;

    terminal() = default;

    template<template<bool, bool, typename...> typename Iter, bool IsConst, bool IsReverse,
        typename... Ts>
    ptrdiff_t
    operator()(const Iter<IsConst, IsReverse, Ts...>& begin,
        const Iter<IsConst, IsReverse, Ts...>& /*curr*/,
        const Iter<IsConst, IsReverse, Ts...>& end) const
    {
        return end - begin;
    }
};

// Op is a member type in expr_op
// T must be either terminal<>, unary_expr<>, or binary_expr<> (no
// unwrapped types - that's why make_unary_expr exists)
template<typename Op, typename T>
struct unary_expr
{
    using is_expr = void;
    static_assert(is_expression<T>::value, "unary expression must contain expression");

    explicit unary_expr(T _t)
        : t(_t)
    {}

    template<template<bool, bool, typename...> typename Iter, bool IsConst, bool IsReverse,
        typename... Ts>
    auto
    operator()(const Iter<IsConst, IsReverse, Ts...>& begin,
        const Iter<IsConst, IsReverse, Ts...>& curr,
        const Iter<IsConst, IsReverse, Ts...>& end) const
        -> decltype(Op::exec(std::declval<T&>().
                             operator()(curr)))
    {
        return Op::exec(t.operator()(begin, curr, end));
    }

    T t;
};

template<typename Op, typename L, typename R>
struct binary_expr
{
    using is_expr = void;
    static_assert(is_expression<L>::value, "binary expression left must be expression");
    static_assert(is_expression<R>::value, "binary expression right must be expression");

    binary_expr(L _l, R _r)
        : l(_l)
        , r(_r)
    {}

    template<template<bool, bool, typename...> typename Iter, bool IsConst, bool IsReverse,
        typename... Ts>
    auto
    operator()(const Iter<IsConst, IsReverse, Ts...>& begin,
        const Iter<IsConst, IsReverse, Ts...>& curr,
        const Iter<IsConst, IsReverse, Ts...>& end) const
        -> decltype(Op::exec(std::declval<L&>().
                             operator()(begin, curr, end),
            std::declval<R&>().
            operator()(begin, curr, end)))
    {
        return Op::exec(l.operator()(begin, curr, end), r.operator()(begin, curr, end));
    }

    L l;
    R r;
};

template<typename Func, typename... As>
struct func_expr
{
    func_expr() = default;
    func_expr(Func& f, As... a) : func(&f), args(a...) {}
    
    using is_expr = void;

    template<typename Iter>
    using applied_args = std::tuple<decltype(std::declval<As&>()(
            std::declval<Iter&>(), std::declval<Iter&>(), std::declval<Iter&>())) ... >;

    using return_type = typename detail::get_return_type<Func>::type;

    template<template<bool, bool, typename...> typename Iter, bool IsConst, bool IsReverse,
        typename... Ts>
    return_type
    operator()(const Iter<IsConst, IsReverse, Ts...>& begin,
        const Iter<IsConst, IsReverse, Ts...>& curr,
        const Iter<IsConst, IsReverse, Ts...>& end) const
    {
        applied_args<Iter<IsConst, IsReverse, Ts...> > results;
        get_val<0>( begin, curr, end, results );
        return std::apply( func, results );
    }

    template< size_t Ind, template<bool, bool, typename...> typename Iter, bool IsConst, 
        bool IsReverse, typename... Ts >
    void get_val( const Iter<IsConst, IsReverse, Ts...>& begin, 
        const Iter<IsConst, IsReverse, Ts...>& curr, const Iter<IsConst, IsReverse, Ts...>& end,
        applied_args<Iter<IsConst, IsReverse, Ts...>> & results ) const
    {
        std::get<Ind>(results) = std::get<Ind>(args)(begin, curr, end);
        if constexpr (Ind+1 < sizeof...(As)) {
            get_val<Ind+1>(begin, curr, end, results);
        }
    }

    Func* func = nullptr;
    std::tuple<As...> args;
};

// If T is terminal<U>, unary_expr<Op,U> or binary_expr<Op,L,R>, just return T.
// If T is any other type, return terminal<T>
template<typename T>
struct maybe_wrap
{
    maybe_wrap() = delete;
    using type   = terminal<T>;
    static type
    wrap(const T& t)
    {
        type out{ t };
        return out;
    }
};

template<typename T>
struct maybe_wrap<terminal<T>>
{
    maybe_wrap() = delete;
    using type   = terminal<T>;
    static type
    wrap(const terminal<T>& t)
    {
        return t;
    }
};

template<typename Op, typename T>
struct maybe_wrap<unary_expr<Op, T>>
{
    maybe_wrap() = delete;
    using type   = unary_expr<Op, T>;
    static type
    wrap(const unary_expr<Op, T>& t)
    {
        return t;
    }
};

template<typename Op, typename L, typename R>
struct maybe_wrap<binary_expr<Op, L, R>>
{
    maybe_wrap() = delete;
    using type   = binary_expr<Op, L, R>;
    static type
    wrap(const binary_expr<Op, L, R>& t)
    {
        return t;
    }
};

template<typename Func, typename... As>
struct maybe_wrap<func_expr<Func, As...>>
{
    maybe_wrap() = delete;
    using type   = func_expr<Func, As...>;
    static type
    wrap(const func_expr<Func, As...>& t)
    {
        return t;
    }
};

template<typename Op, typename T>
struct make_unary_expr
{
    make_unary_expr() = delete;
    using type        = unary_expr<Op, typename maybe_wrap<T>::type>;

    static type
    create(const T& t)
    {
        type out{ maybe_wrap<T>::wrap(t) };
        return out;
    }
};

template<typename Op, typename L, typename R>
struct make_binary_expr
{
    make_binary_expr() = delete;
    using type = binary_expr<Op, typename maybe_wrap<L>::type, typename maybe_wrap<R>::type>;

    static type
    create(const L& l, const R& r)
    {
        type out{ maybe_wrap<L>::wrap(l), maybe_wrap<R>::wrap(r) };
        return out;
    }
};

template<typename Func, typename... Args>
struct make_func_expr
{
    make_func_expr() = delete;
    using type = func_expr< Func, typename maybe_wrap< Args >::type ... >;

    static type 
    create( Func& func, Args... args )
    {
        type out( func, maybe_wrap<Args>::wrap(args)... );
        return out;
    }
};

template<typename Func, typename... Args>
typename make_func_expr<Func, Args...>::type fn( Func& func, Args... args )
{
    return make_func_expr<Func, Args...>::create( func, args... );
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::LT, L, R>::type>::type
operator<(L l, R r)
{
    return make_binary_expr<expr_op::LT, L, R>::create(l, r);
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::GT, L, R>::type>::type
operator>(L l, R r)
{
    return make_binary_expr<expr_op::GT, L, R>::create(l, r);
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::LE, L, R>::type>::type
operator<=(L l, R r)
{
    return make_binary_expr<expr_op::LE, L, R>::create(l, r);
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::GE, L, R>::type>::type
operator>=(L l, R r)
{
    return make_binary_expr<expr_op::GE, L, R>::create(l, r);
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::AND, L, R>::type>::type
operator&&(L l, R r)
{
    return make_binary_expr<expr_op::AND, L, R>::create(l, r);
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::OR, L, R>::type>::type
operator||(L l, R r)
{
    return make_binary_expr<expr_op::OR, L, R>::create(l, r);
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::EQ, L, R>::type>::type
operator==(L l, R r)
{
    return make_binary_expr<expr_op::EQ, L, R>::create(l, r);
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::NE, L, R>::type>::type
operator!=(L l, R r)
{
    return make_binary_expr<expr_op::NE, L, R>::create(l, r);
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::PLUS, L, R>::type>::type
operator+(L l, R r)
{
    return make_binary_expr<expr_op::PLUS, L, R>::create(l, r);
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::MINUS, L, R>::type>::type
operator-(L l, R r)
{
    return make_binary_expr<expr_op::MINUS, L, R>::create(l, r);
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::MULTIPLY, L, R>::type>::type
operator*(L l, R r)
{
    return make_binary_expr<expr_op::MULTIPLY, L, R>::create(l, r);
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::DIVIDE, L, R>::type>::type
operator/(L l, R r)
{
    return make_binary_expr<expr_op::DIVIDE, L, R>::create(l, r);
}

template<typename L, typename R>
typename std::enable_if<std::disjunction<is_expression<L>, is_expression<R>>::value,
    typename make_binary_expr<expr_op::MODULUS, L, R>::type>::type
operator%(L l, R r)
{
    return make_binary_expr<expr_op::MODULUS, L, R>::create(l, r);
}

template<typename T>
typename std::enable_if<is_expression<T>::value,
    typename make_unary_expr<expr_op::NEGATE, T>::type>::type
operator-(T t)
{
    return make_unary_expr<expr_op::NEGATE, T>::create(t);
}

template<typename T>
typename std::enable_if<is_expression<T>::value,
    typename make_unary_expr<expr_op::NOT, T>::type>::type
operator!(T t)
{
    return make_unary_expr<expr_op::NOT, T>::create(t);
}

} // namespace mf


#endif // INCLUDED_mainframe_detail_expression_hpp
