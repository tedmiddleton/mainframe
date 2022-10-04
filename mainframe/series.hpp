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

    // operator=
    series& operator=(series&& other);

    series& operator=(const series&) = default;

    series& operator=(std::initializer_list<T> init);

    template<typename U = T, std::enable_if_t<detail::is_missing<U>::value, bool> = true>
    series<T> allow_missing() const;

    template<typename U = T, std::enable_if_t<!detail::is_missing<U>::value, bool> = true>
    series<mi<T>> allow_missing() const;

    // This requires default-construction. Can we do better?
    template<typename U = T,
        std::enable_if_t<detail::is_missing<U>::value &&
                std::is_default_constructible<typename U::value_type>::value,
            bool>       = true>
    series<typename U::value_type> disallow_missing() const;

    template<typename U = T, std::enable_if_t<!detail::is_missing<U>::value, bool> = true>
    series<T> disallow_missing() const;

    // assign
    void assign(size_t count, const T& value);

    template<typename InputIt>
    void assign(InputIt inbegin, InputIt inend);

    void assign(std::initializer_list<T> init);

    // at
    reference at(size_t n);

    const_reference at(size_t n) const;

    // operator[]
    // It seems like this should really be an unref-ing operation, otherwise
    // there's the possibility that the user will damage multiple series.
    reference operator[](size_t n);

    const_reference operator[](size_t n) const;

    // front & back
    reference front();

    const_reference front() const;

    reference back();

    const_reference back() const;

    // data
    T* data();

    const T* data() const;

    // begin, cbegin, end, cend
    iterator begin();

    const_iterator begin() const;

    const_iterator cbegin() const;

    iterator end();

    const_iterator end() const;

    const_iterator cend() const;

    // rbegin, crbegin, rend, crend
    reverse_iterator rbegin();

    const_reverse_iterator rbegin() const;

    const_reverse_iterator crbegin() const;

    reverse_iterator rend();

    const_reverse_iterator rend() const;

    const_reverse_iterator crend() const;

    // capacity & modifiers
    bool empty() const;

    size_t size() const;

    size_t max_size() const;

    void reserve(size_t _size);

    size_t capacity() const;

    void shrink_to_fit();

    void clear();

    // insert & emplace
    iterator insert(const_iterator pos, const T& value);

    iterator insert(const_iterator pos, T&& value);

    iterator insert(const_iterator pos, size_t count, const T& value);

    template<typename InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last);

    iterator insert(const_iterator pos, std::initializer_list<T> init);

    template<class... Args>
    iterator emplace(const_iterator pos, Args&&... args);

    // erase
    iterator erase(const_iterator pos);

    iterator erase(const_iterator first, const_iterator last);

    // push_back, emplace_back, pop_back
    void push_back(const T& value);

    void push_back(T&& value);

    template<typename... Args>
    reference emplace_back(Args&&... args);

    void pop_back();

    // resize
    void resize(size_t newsize);

    void resize(size_t newsize, const T& value);

    const std::string& name() const;

    void set_name(const std::string& name);

    std::vector<std::string> to_string() const;

    series unique() const;

    size_t use_count() const;

    bool operator==(const series<T>& other) const;

    bool operator!=(const series<T>& other) const;

    void unref();

private:
    iterator unref(iterator it);

    const_iterator unref(const_iterator it);

    friend class useries;
    template<typename U>
    friend std::ostream& operator<<(std::ostream&, const series<U>&);

    std::string m_name;
    std::shared_ptr<series_vector<T>> m_sharedvec;
};

} // namespace mf


#endif // INCLUDED_mainframe_series_h
