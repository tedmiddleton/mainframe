//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_expression_hpp
#define INCLUDED_mainframe_expression_hpp

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

template<typename T>
struct is_complex_expression : std::false_type
{};

template<typename Op, typename T>
struct is_complex_expression<unary_expr<Op, T>> : std::true_type
{};

template<typename Op, typename L, typename R>
struct is_complex_expression<binary_expr<Op, L, R>> : std::true_type
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
    explicit terminal(expr_column<Ind>)
    {}

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

template<size_t Ind>
using columnindex = terminal<expr_column<Ind>>;

template<size_t... Inds>
struct columnindexpack
{
    columnindexpack() {}
    columnindexpack(columnindex<Inds>...) {}

    template<size_t NewInd>
    columnindexpack<Inds..., NewInd>
    operator,(columnindex<NewInd>) const
    {
        return columnindexpack<Inds..., NewInd>();
    }
};

template<>
struct terminal<row_number>
{
    using is_expr             = void;

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
    using is_expr             = void;

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
        -> decltype(Op::exec(detail::quickval<L>::value.
                             operator()(begin, curr, end),
            detail::quickval<R>::value.
            operator()(begin, curr, end)))
    {
        return Op::exec(l.operator()(begin, curr, end), r.operator()(begin, curr, end));
    }

    L l;
    R r;
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
        -> decltype(Op::exec(detail::quickval<T>::value.
                             operator()(curr)))
    {
        return Op::exec(t.operator()(begin, curr, end));
    }

    T t;
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

template<size_t Ind>
columnindex<Ind>
col()
{
    return columnindex<Ind>{ expr_column<Ind>{} };
}

namespace placeholders
{
inline constexpr terminal<row_number> rownum;
inline constexpr terminal<frame_length> framelen;
inline constexpr columnindex<0> _0;
inline constexpr columnindex<1> _1;
inline constexpr columnindex<2> _2;
inline constexpr columnindex<3> _3;
inline constexpr columnindex<4> _4;
inline constexpr columnindex<5> _5;
inline constexpr columnindex<6> _6;
inline constexpr columnindex<7> _7;
inline constexpr columnindex<8> _8;
inline constexpr columnindex<9> _9;
inline constexpr columnindex<10> _10;
inline constexpr columnindex<11> _11;
inline constexpr columnindex<12> _12;
inline constexpr columnindex<13> _13;
inline constexpr columnindex<14> _14;
inline constexpr columnindex<15> _15;
inline constexpr columnindex<16> _16;
inline constexpr columnindex<17> _17;
inline constexpr columnindex<18> _18;
inline constexpr columnindex<19> _19;
inline constexpr columnindex<20> _20;
inline constexpr columnindex<21> _21;
inline constexpr columnindex<22> _22;
inline constexpr columnindex<23> _23;
inline constexpr columnindex<24> _24;
inline constexpr columnindex<25> _25;
inline constexpr columnindex<26> _26;
inline constexpr columnindex<27> _27;
inline constexpr columnindex<28> _28;
inline constexpr columnindex<29> _29;
inline constexpr columnindex<30> _30;
inline constexpr columnindex<31> _31;
inline constexpr columnindex<32> _32;
inline constexpr columnindex<33> _33;
inline constexpr columnindex<34> _34;
inline constexpr columnindex<35> _35;
inline constexpr columnindex<36> _36;
inline constexpr columnindex<37> _37;
inline constexpr columnindex<38> _38;
inline constexpr columnindex<39> _39;
inline constexpr columnindex<40> _40;
inline constexpr columnindex<41> _41;
inline constexpr columnindex<42> _42;
inline constexpr columnindex<43> _43;
inline constexpr columnindex<44> _44;
inline constexpr columnindex<45> _45;
inline constexpr columnindex<46> _46;
inline constexpr columnindex<47> _47;
inline constexpr columnindex<48> _48;
inline constexpr columnindex<49> _49;
inline constexpr columnindex<50> _50;
inline constexpr columnindex<51> _51;
inline constexpr columnindex<52> _52;
inline constexpr columnindex<53> _53;
inline constexpr columnindex<54> _54;
inline constexpr columnindex<55> _55;
inline constexpr columnindex<56> _56;
inline constexpr columnindex<57> _57;
inline constexpr columnindex<58> _58;
inline constexpr columnindex<59> _59;
inline constexpr columnindex<60> _60;
inline constexpr columnindex<61> _61;
inline constexpr columnindex<62> _62;
inline constexpr columnindex<63> _63;
inline constexpr columnindex<64> _64;
inline constexpr columnindex<65> _65;
inline constexpr columnindex<66> _66;
inline constexpr columnindex<67> _67;
inline constexpr columnindex<68> _68;
inline constexpr columnindex<69> _69;
inline constexpr columnindex<70> _70;
inline constexpr columnindex<71> _71;
inline constexpr columnindex<72> _72;
inline constexpr columnindex<73> _73;
inline constexpr columnindex<74> _74;
inline constexpr columnindex<75> _75;
inline constexpr columnindex<76> _76;
inline constexpr columnindex<77> _77;
inline constexpr columnindex<78> _78;
inline constexpr columnindex<79> _79;
inline constexpr columnindex<80> _80;
inline constexpr columnindex<81> _81;
inline constexpr columnindex<82> _82;
inline constexpr columnindex<83> _83;
inline constexpr columnindex<84> _84;
inline constexpr columnindex<85> _85;
inline constexpr columnindex<86> _86;
inline constexpr columnindex<87> _87;
inline constexpr columnindex<88> _88;
inline constexpr columnindex<89> _89;
inline constexpr columnindex<90> _90;
inline constexpr columnindex<91> _91;
inline constexpr columnindex<92> _92;
inline constexpr columnindex<93> _93;
inline constexpr columnindex<94> _94;
inline constexpr columnindex<95> _95;
inline constexpr columnindex<96> _96;
inline constexpr columnindex<97> _97;
inline constexpr columnindex<98> _98;
inline constexpr columnindex<99> _99;
inline constexpr columnindex<0> col0;
inline constexpr columnindex<1> col1;
inline constexpr columnindex<2> col2;
inline constexpr columnindex<3> col3;
inline constexpr columnindex<4> col4;
inline constexpr columnindex<5> col5;
inline constexpr columnindex<6> col6;
inline constexpr columnindex<7> col7;
inline constexpr columnindex<8> col8;
inline constexpr columnindex<9> col9;
inline constexpr columnindex<10> col10;
inline constexpr columnindex<11> col11;
inline constexpr columnindex<12> col12;
inline constexpr columnindex<13> col13;
inline constexpr columnindex<14> col14;
inline constexpr columnindex<15> col15;
inline constexpr columnindex<16> col16;
inline constexpr columnindex<17> col17;
inline constexpr columnindex<18> col18;
inline constexpr columnindex<19> col19;
inline constexpr columnindex<20> col20;
inline constexpr columnindex<21> col21;
inline constexpr columnindex<22> col22;
inline constexpr columnindex<23> col23;
inline constexpr columnindex<24> col24;
inline constexpr columnindex<25> col25;
inline constexpr columnindex<26> col26;
inline constexpr columnindex<27> col27;
inline constexpr columnindex<28> col28;
inline constexpr columnindex<29> col29;
inline constexpr columnindex<30> col30;
inline constexpr columnindex<31> col31;
inline constexpr columnindex<32> col32;
inline constexpr columnindex<33> col33;
inline constexpr columnindex<34> col34;
inline constexpr columnindex<35> col35;
inline constexpr columnindex<36> col36;
inline constexpr columnindex<37> col37;
inline constexpr columnindex<38> col38;
inline constexpr columnindex<39> col39;
inline constexpr columnindex<40> col40;
inline constexpr columnindex<41> col41;
inline constexpr columnindex<42> col42;
inline constexpr columnindex<43> col43;
inline constexpr columnindex<44> col44;
inline constexpr columnindex<45> col45;
inline constexpr columnindex<46> col46;
inline constexpr columnindex<47> col47;
inline constexpr columnindex<48> col48;
inline constexpr columnindex<49> col49;
inline constexpr columnindex<50> col50;
inline constexpr columnindex<51> col51;
inline constexpr columnindex<52> col52;
inline constexpr columnindex<53> col53;
inline constexpr columnindex<54> col54;
inline constexpr columnindex<55> col55;
inline constexpr columnindex<56> col56;
inline constexpr columnindex<57> col57;
inline constexpr columnindex<58> col58;
inline constexpr columnindex<59> col59;
inline constexpr columnindex<60> col60;
inline constexpr columnindex<61> col61;
inline constexpr columnindex<62> col62;
inline constexpr columnindex<63> col63;
inline constexpr columnindex<64> col64;
inline constexpr columnindex<65> col65;
inline constexpr columnindex<66> col66;
inline constexpr columnindex<67> col67;
inline constexpr columnindex<68> col68;
inline constexpr columnindex<69> col69;
inline constexpr columnindex<70> col70;
inline constexpr columnindex<71> col71;
inline constexpr columnindex<72> col72;
inline constexpr columnindex<73> col73;
inline constexpr columnindex<74> col74;
inline constexpr columnindex<75> col75;
inline constexpr columnindex<76> col76;
inline constexpr columnindex<77> col77;
inline constexpr columnindex<78> col78;
inline constexpr columnindex<79> col79;
inline constexpr columnindex<80> col80;
inline constexpr columnindex<81> col81;
inline constexpr columnindex<82> col82;
inline constexpr columnindex<83> col83;
inline constexpr columnindex<84> col84;
inline constexpr columnindex<85> col85;
inline constexpr columnindex<86> col86;
inline constexpr columnindex<87> col87;
inline constexpr columnindex<88> col88;
inline constexpr columnindex<89> col89;
inline constexpr columnindex<90> col90;
inline constexpr columnindex<91> col91;
inline constexpr columnindex<92> col92;
inline constexpr columnindex<93> col93;
inline constexpr columnindex<94> col94;
inline constexpr columnindex<95> col95;
inline constexpr columnindex<96> col96;
inline constexpr columnindex<97> col97;
inline constexpr columnindex<98> col98;
inline constexpr columnindex<99> col99;

} // namespace placeholders

} // namespace mf


#endif // INCLUDED_mainframe_expression_hpp
