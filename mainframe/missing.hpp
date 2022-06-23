
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_missing_hpp
#define INCLUDED_mainframe_missing_hpp

#include <optional>

namespace mf
{

inline constexpr std::nullopt_t missing = std::nullopt;

template<typename T>
class mi : public std::optional<T>
{
public:
    using std::optional<T>::optional;

    operator bool() const noexcept = delete;

private:
    template<typename U>
    friend std::ostream& operator<<(std::ostream& o, const mi<U>& u);
};

template<typename U>
std::ostream&
operator<<(std::ostream& o, const mi<U>& u)
{
    if (u.has_value()) {
        o << "mi<" << typeid(U).name() << ">{" << *u << "}";
    }
    else {
        o << "missing";
    }
    return o;
}

template<typename T>
constexpr mi<std::decay_t<T>>
make_missing(T&& t)
{
    return mi<std::decay_t<T>>{ std::forward<T>(t) };
}

template<typename T, typename... Args>
constexpr mi<T>
make_missing(Args&&... args)
{
    return mi<T>{ std::in_place, std::forward<Args>(args)... };
}

template<typename T, typename U, typename... Args>
constexpr mi<T>
make_missing(std::initializer_list<U> il, Args&&... args)
{
    return mi<T>{ std::in_place, il, std::forward<Args>(args)... };
}

template<typename T, typename U>
auto
operator+(const mi<T>& t, const mi<U>& u) -> mi<decltype(*t + *u)>
{
    if (!t.has_value() || !u.has_value()) {
        return missing;
    }
    return mi<decltype(*t + *u)>(*t + *u);
}

template<typename T, typename U>
auto
operator+(const mi<T>& t, const U& u) -> mi<decltype(*t + u)>
{
    if (!t.has_value()) {
        return missing;
    }
    return mi<decltype(*t + u)>{ *t + u };
}

template<typename T, typename U>
auto
operator+(const T& t, const mi<U>& u) -> mi<decltype(t + *u)>
{
    if (!u.has_value()) {
        return missing;
    }
    return mi<decltype(t + *u)>{ t + *u };
}

template<typename T, typename U>
auto
operator-(const mi<T>& t, const mi<U>& u) -> mi<decltype(*t - *u)>
{
    if (!t.has_value() || !u.has_value()) {
        return missing;
    }
    return mi<decltype(*t - *u)>(*t - *u);
}

template<typename T, typename U>
auto
operator-(const mi<T>& t, const U& u) -> mi<decltype(*t - u)>
{
    if (!t.has_value()) {
        return missing;
    }
    return mi<decltype(*t - u)>{ *t - u };
}

template<typename T, typename U>
auto
operator-(const T& t, const mi<U>& u) -> mi<decltype(t - *u)>
{
    if (!u.has_value()) {
        return missing;
    }
    return mi<decltype(t - *u)>{ t - *u };
}

template<typename T, typename U>
auto
operator*(const mi<T>& t, const mi<U>& u) -> mi<decltype(*t * *u)>
{
    if (!t.has_value() || !u.has_value()) {
        return missing;
    }
    return mi<decltype(*t * *u)>(*t * *u);
}

template<typename T, typename U>
auto
operator*(const mi<T>& t, const U& u) -> mi<decltype(*t * u)>
{
    if (!t.has_value()) {
        return missing;
    }
    return mi<decltype(*t * u)>{ *t * u };
}

template<typename T, typename U>
auto
operator*(const T& t, const mi<U>& u) -> mi<decltype(t * *u)>
{
    if (!u.has_value()) {
        return missing;
    }
    return mi<decltype(t * *u)>{ t * *u };
}

template<typename T, typename U>
auto
operator/(const mi<T>& t, const mi<U>& u) -> mi<decltype(*t / *u)>
{
    if (!t.has_value() || !u.has_value()) {
        return missing;
    }
    return mi<decltype(*t / *u)>(*t / *u);
}

template<typename T, typename U>
auto
operator/(const mi<T>& t, const U& u) -> mi<decltype(*t / u)>
{
    if (!t.has_value()) {
        return missing;
    }
    return mi<decltype(*t / u)>{ *t / u };
}

template<typename T, typename U>
auto
operator/(const T& t, const mi<U>& u) -> mi<decltype(t / *u)>
{
    if (!u.has_value()) {
        return missing;
    }
    return mi<decltype(t / *u)>{ t / *u };
}

template<typename T, typename U>
auto
operator%(const mi<T>& t, const mi<U>& u) -> mi<decltype(*t % *u)>
{
    if (!t.has_value() || !u.has_value()) {
        return missing;
    }
    return mi<decltype(*t % *u)>(*t % *u);
}

template<typename T, typename U>
auto
operator%(const mi<T>& t, const U& u) -> mi<decltype(*t % u)>
{
    if (!t.has_value()) {
        return missing;
    }
    return mi<decltype(*t % u)>{ *t % u };
}

template<typename T, typename U>
auto
operator%(const T& t, const mi<U>& u) -> mi<decltype(t % *u)>
{
    if (!u.has_value()) {
        return missing;
    }
    return mi<decltype(t % *u)>{ t % *u };
}

template<typename T>
auto
operator~(const mi<T>& t) -> mi<decltype(~*t)>
{
    if (!t.has_value()) {
        return missing;
    }
    return mi<decltype(~*t)>{ ~*t };
}

template<typename T>
auto
operator!(const mi<T>& t) -> mi<decltype(!*t)>
{
    if (!t.has_value()) {
        return missing;
    }
    return mi<decltype(!*t)>{ !*t };
}

template<typename _Tp>
mi(_Tp) -> mi<_Tp>;

} // namespace mf

#endif // INCLUDED_mainframe_missing_hpp
