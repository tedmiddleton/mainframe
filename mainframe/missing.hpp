//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_missing_hpp
#define INCLUDED_mainframe_missing_hpp

#include <iostream>
#include <optional>

namespace mf
{

using missing_t = std::nullopt_t;
inline constexpr missing_t missing = std::nullopt;

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

template<typename T, typename U>
bool
operator==(const mi<T>& lhs, const mi<U>& rhs)
{
    if (!lhs.has_value() && !rhs.has_value()) {
        return true;
    }
    if (!lhs.has_value() || !rhs.has_value()) {
        return false;
    }
    return *lhs == *rhs;
}

template<typename T, typename U>
bool
operator==(const mi<T>& lhs, const U& rhs)
{
    if (!lhs.has_value()) {
        return false;
    }
    return *lhs == rhs;
}

template<typename T, typename U>
bool
operator==(const T& lhs, const mi<U>& rhs)
{
    if (!rhs.has_value()) {
        return false;
    }
    return lhs == *rhs;
}

template<typename T>
bool
operator==(const mi<T>& lhs, missing_t)
{
    return !lhs.has_value();
}

template<typename U>
bool
operator==(missing_t, const mi<U>& rhs)
{
    return !rhs.has_value();
}

template<typename T, typename U>
bool
operator!=(const mi<T>& lhs, const mi<U>& rhs)
{
    return !(lhs == rhs);
}

template<typename T, typename U>
bool
operator!=(const mi<T>& lhs, const U& rhs)
{
    if (!lhs.has_value()) {
        return true;
    }
    return *lhs != rhs;
}

template<typename T, typename U>
bool
operator!=(const T& lhs, const mi<U>& rhs)
{
    if (!rhs.has_value()) {
        return true;
    }
    return lhs != *rhs;
}

template<typename T>
bool
operator!=(const mi<T>& lhs, missing_t)
{
    return lhs.has_value();
}

template<typename U>
bool
operator!=(missing_t, const mi<U>& rhs)
{
    return rhs.has_value();
}

template<typename T, typename U>
bool
operator<(const mi<T>& lhs, const mi<U>& rhs)
{
    return rhs.has_value() && (!lhs.has_value() || (*lhs < *rhs));
}

template<typename T>
bool
operator<(const mi<T>&, missing_t)
{
    return false;
}

template<typename U>
bool
operator<(missing_t, const mi<U>& rhs)
{
    return rhs.has_value();
}

template<typename T, typename U>
bool
operator<(const mi<T>& lhs, const U& rhs)
{
    return !lhs.has_value() || (*lhs < rhs);
}

template<typename T, typename U>
bool
operator<(const T& lhs, const mi<U>& rhs)
{
    return rhs.has_value() && (lhs < *rhs);
}

template<typename T, typename U>
bool
operator>(const mi<T>& lhs, const mi<U>& rhs)
{
    return lhs.has_value() && (!rhs.has_value() || (*lhs > *rhs));
}

template<typename T>
bool
operator>(const mi<T>& lhs, missing_t)
{
    return lhs.has_value();
}

template<typename U>
bool
operator>(missing_t, const mi<U>&)
{
    return false;
}

template<typename T, typename U>
bool
operator>(const mi<T>& lhs, const U& rhs)
{
    return lhs.has_value() && (*lhs > rhs);
}

template<typename T, typename U>
bool
operator>(const T& lhs, const mi<U>& rhs)
{
    return !rhs.has_value() || (lhs > *rhs);
}

template<typename T, typename U>
bool
operator<=(const mi<T>& lhs, const mi<U>& rhs)
{
    return !lhs.has_value() || (rhs.has_value() && (*lhs <= *rhs));
}

template<typename T>
bool
operator<=(const mi<T>& lhs, missing_t)
{
    return !lhs.has_value();
}

template<typename U>
bool
operator<=(missing_t, const mi<U>&)
{
    return true;
}

template<typename T, typename U>
bool
operator<=(const mi<T>& lhs, const U& rhs)
{
    return !lhs.has_value() || (*lhs <= rhs);
}

template<typename T, typename U>
bool
operator<=(const T& lhs, const mi<U>& rhs)
{
    return rhs.has_value() && (lhs <= *rhs);
}

template<typename T, typename U>
bool
operator>=(const mi<T>& lhs, const mi<U>& rhs)
{
    return !rhs.has_value() || (lhs.has_value() && (*lhs >= *rhs));
}

template<typename T>
bool
operator>=(const mi<T>&, missing_t)
{
    return true;
}

template<typename U>
bool
operator>=(missing_t, const mi<U>& rhs)
{
    return !rhs.has_value();
}

template<typename T, typename U>
bool
operator>=(const mi<T>& lhs, const U& rhs)
{
    return lhs.has_value() && (*lhs >= rhs);
}

template<typename T, typename U>
bool
operator>=(const T& lhs, const mi<U>& rhs)
{
    return !rhs.has_value() || (lhs >= *rhs);
}

template<typename T>
mi(T) -> mi<T>;

} // namespace mf

namespace std
{
    template<typename T>
    struct hash<mf::mi<T>>
    {
        size_t 
        operator()(const mf::mi<T>& mit) const noexcept
        {
            if (!mit.has_value()) {
                return 0;
            }
            std::hash<T> hasher;
            return hasher(*mit);
        }
    };
}


#endif // INCLUDED_mainframe_missing_hpp
