//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_series_h
#define INCLUDED_mainframe_series_h

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "mainframe/detail/base.hpp"
#include "mainframe/detail/series_vector.hpp"
#include "mainframe/detail/useries.hpp"
#include "mainframe/missing.hpp"

namespace mf
{

///
/// series class
///
/// The series class is the column type for the @ref frame class. series
/// features a sort of copy-on-write semantics - series instances can be copied
/// cheaply because they contain a reference to an underlying ref-counted array
/// which isn't copied. When a series instance needs to be mutated, however, the
/// underlying array will be copied if it is also referenced by other series
/// instances. For example
///
///     series<int> s1;
///     auto s2 = s1; // s2 and s1 now reference the same underlying array
///     assert( s1.use_count() == 2 );
///     assert( s2.use_count() == 2 );
///     // iterator can mutate, so the array is copied or "unref"'ed
///     series<int>::iterator it = s2.begin();
///     assert( s1.use_count() == 1 );
///     assert( s2.use_count() == 1 );
///
template<typename T>
class series
{
    using iseries_vector = detail::iseries_vector;
    template<typename U>
    using series_vector = detail::series_vector<U>;

public:
    // types
    using value_type             = T;
    using difference_type        = typename series_vector<T>::difference_type;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    using iterator               = typename series_vector<T>::iterator;
    using const_iterator         = typename series_vector<T>::const_iterator;
    using reverse_iterator       = typename series_vector<T>::reverse_iterator;
    using const_reverse_iterator = typename series_vector<T>::const_reverse_iterator;

    // ctors
    series();

    series(size_t count, const T& value);

    explicit series(size_t count);

    template<typename InputIt>
    series(InputIt f, InputIt l);

    series(const series& other) = default;

    series(series&& other);

    explicit series(std::initializer_list<T> init);

    virtual ~series() = default;

    // begin, cbegin, end, cend
    iterator
    begin();

    const_iterator
    begin() const;

    const_iterator
    cbegin() const;

    iterator
    end();

    const_iterator
    end() const;

    const_iterator
    cend() const;

    // rbegin, crbegin, rend, crend
    reverse_iterator
    rbegin();

    const_reverse_iterator
    rbegin() const;

    const_reverse_iterator
    crbegin() const;

    reverse_iterator
    rend();

    const_reverse_iterator
    rend() const;

    const_reverse_iterator
    crend() const;

    template<typename _U = T, std::enable_if_t<detail::is_missing<_U>::value, bool> = true>
    series<T>
    allow_missing() const;

    template<typename _U = T, std::enable_if_t<!detail::is_missing<_U>::value, bool> = true>
    series<mi<T>>
    allow_missing() const;

    // assign
    void
    assign(std::initializer_list<T> init);

    void
    assign(size_t count, const T& value);

    template<typename InputIt>
    void
    assign(InputIt inbegin, InputIt inend);

    // at
    reference
    at(size_t n);

    const_reference
    at(size_t n) const;

    reference
    back();

    const_reference
    back() const;

    size_t
    capacity() const;

    void
    clear();

    // data
    T*
    data();

    const T*
    data() const;

    // This requires default-construction. Can we do better?
    template<typename _U = T,
        std::enable_if_t<detail::is_missing<_U>::value &&
                std::is_default_constructible<typename _U::value_type>::value,
            bool>        = true>
    series<typename _U::value_type>
    disallow_missing() const;

    template<typename _U = T, std::enable_if_t<!detail::is_missing<_U>::value, bool> = true>
    series<T>
    disallow_missing() const;

    template<class... Args>
    iterator
    emplace(const_iterator pos, Args&&... args);

    template<typename... Args>
    reference
    emplace_back(Args&&... args);

    // capacity & modifiers
    bool
    empty() const;

    // erase
    iterator
    erase(const_iterator pos);

    iterator
    erase(const_iterator first, const_iterator last);

    // front & back
    reference
    front();

    const_reference
    front() const;

    // insert & emplace
    iterator
    insert(const_iterator pos, const T& value);

    iterator
    insert(const_iterator pos, T&& value);

    iterator
    insert(const_iterator pos, size_t count, const T& value);

    template<typename InputIt>
    iterator
    insert(const_iterator pos, InputIt first, InputIt last);

    iterator
    insert(const_iterator pos, std::initializer_list<T> init);

    size_t
    max_size() const;

    double
    mean() const;

    /// Calculate the minimum and maximum value in the series, and return them in
    /// a std::pair<>
    ///
    /// If the series is empty and the series type doesn't support default
    /// construction, this will throw std::out_of_range
    ///
    std::pair<T, T>
    minmax() const;

    const std::string&
    name() const;

    // operator=
    series&
    operator=(series&& other);

    series&
    operator=(const series&) = default;

    series&
    operator=(std::initializer_list<T> init);

    // operator[]
    // It seems like this should really be an unref-ing operation, otherwise
    // there's the possibility that the user will damage multiple series.
    reference
    operator[](size_t n);

    const_reference
    operator[](size_t n) const;

    bool
    operator==(const series<T>& other) const;

    bool
    operator!=(const series<T>& other) const;

    template<typename U>
    void
    operator+=(const U& value);

    template<typename U>
    void
    operator-=(const U& value);

    template<typename U>
    void
    operator*=(const U& value);

    template<typename U>
    void
    operator/=(const U& value);

    template<typename U>
    void
    operator%=(const U& value);

    template<typename U>
    void
    operator+=(const series<U>& other);

    template<typename U>
    void
    operator-=(const series<U>& other);

    template<typename U>
    void
    operator*=(const series<U>& other);

    template<typename U>
    void
    operator/=(const series<U>& other);

    template<typename U>
    void
    operator%=(const series<U>& other);

    template<typename U>
    series<decltype(std::declval<T>() + std::declval<U>())>
    operator+(const U& value) const;

    template<typename U>
    series<decltype(std::declval<T>() - std::declval<U>())>
    operator-(const U& value) const;

    template<typename U>
    series<decltype(std::declval<T>() * std::declval<U>())>
    operator*(const U& value) const;

    template<typename U>
    series<decltype(std::declval<T>() / std::declval<U>())>
    operator/(const U& value) const;

    template<typename U>
    series<decltype(std::declval<T>() % std::declval<U>())>
    operator%(const U& value) const;

    template<typename U>
    series<decltype(std::declval<T>() + std::declval<U>())>
    operator+(const series<U>& other) const;

    template<typename U>
    series<decltype(std::declval<T>() - std::declval<U>())>
    operator-(const series<U>& other) const;

    template<typename U>
    series<decltype(std::declval<T>() * std::declval<U>())>
    operator*(const series<U>& other) const;

    template<typename U>
    series<decltype(std::declval<T>() / std::declval<U>())>
    operator/(const series<U>& other) const;

    template<typename U>
    series<decltype(std::declval<T>() % std::declval<U>())>
    operator%(const series<U>& other) const;

    // push_back, emplace_back, pop_back
    void
    push_back(const T& value);

    void
    push_back(T&& value);

    void
    pop_back();

    void
    reserve(size_t _size);

    // resize
    void
    resize(size_t newsize);

    void
    resize(size_t newsize, const T& value);

    void
    set_name(const std::string& name);

    void
    shrink_to_fit();

    size_t
    size() const;

    double
    stddev() const;

    std::vector<std::string>
    to_string() const;

    series
    unique() const;

    size_t
    use_count() const;

    void
    unref();

private:
    iterator
    unref(iterator it);

    const_iterator
    unref(const_iterator it);

    friend class useries;
    template<typename U>
    friend std::ostream&
    operator<<(std::ostream&, const series<U>&);

    std::string m_name;
    std::shared_ptr<series_vector<T>> m_sharedvec;
};

} // namespace mf


#endif // INCLUDED_mainframe_series_h
