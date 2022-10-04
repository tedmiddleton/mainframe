//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_impl_series_h
#define INCLUDED_mainframe_impl_series_h

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
series<T>::series()
    : m_sharedvec(std::make_shared<series_vector<T>>())
{}

template<typename T>
series<T>::series(size_t count, const T& value)
    : m_sharedvec(std::make_shared<series_vector<T>>(count, value))
{}

template<typename T>
series<T>::series(size_t count)
    : m_sharedvec(std::make_shared<series_vector<T>>(count))
{}

template<typename T>
template<typename InputIt>
series<T>::series(InputIt f, InputIt l)
    : m_sharedvec(std::make_shared<series_vector<T>>(f, l))
{}

template<typename T>
series<T>::series(series&& other)
    : m_name(std::move(other.m_name))
    , m_sharedvec(std::move(other.m_sharedvec))
{
    // Don't leave other with nullptr
    other.m_sharedvec = std::make_shared<series_vector<T>>();
}

template<typename T>
series<T>::series(std::initializer_list<T> init)
    : m_sharedvec(std::make_shared<series_vector<T>>(init))
{}

template<typename T>
series<T>&
series<T>::operator=(series&& other)
{
    m_name            = other.m_name;
    m_sharedvec       = std::move(other.m_sharedvec);
    other.m_sharedvec = std::make_shared<series_vector<T>>();
    return *this;
}

template<typename T>
series<T>&
series<T>::operator=(std::initializer_list<T> init)
{
    m_sharedvec = std::make_shared<series_vector<T>>(init);
    return *this;
}

template<typename T>
template<typename U, std::enable_if_t<detail::is_missing<U>::value, bool>>
series<T>
series<T>::allow_missing() const
{
    return *this;
}

template<typename T>
template<typename U, std::enable_if_t<!detail::is_missing<U>::value, bool>>
series<mi<T>>
series<T>::allow_missing() const
{
    series<mi<T>> os;
    for (auto& e : *m_sharedvec) {
        os.push_back(e);
    }
    os.set_name(name());
    return os;
}

template<typename T>
template<typename U,
    std::enable_if_t<detail::is_missing<U>::value &&
            std::is_default_constructible<typename U::value_type>::value,
        bool>>
series<typename U::value_type>
series<T>::disallow_missing() const
{
    using V = typename T::value_type;
    series<V> s;
    for (auto& e : *m_sharedvec) {
        if (e.has_value()) {
            s.push_back(*e);
        }
        else {
            s.push_back(V{});
        }
    }
    s.set_name(name());
    return s;
}

template<typename T>
template<typename U, std::enable_if_t<!detail::is_missing<U>::value, bool>>
series<T>
series<T>::disallow_missing() const
{
    return *this;
}

template<typename T>
void
series<T>::assign(size_t count, const T& value)
{
    m_sharedvec = std::make_shared<series_vector<T>>(count, value);
}

template<typename T>
template<typename InputIt>
void
series<T>::assign(InputIt inbegin, InputIt inend)
{
    m_sharedvec = std::make_shared<series_vector<T>>(inbegin, inend);
}

template<typename T>
void
series<T>::assign(std::initializer_list<T> init)
{
    m_sharedvec = std::make_shared<series_vector<T>>(init);
}

template<typename T>
typename series<T>::reference
series<T>::at(size_t n)
{
    unref();
    return m_sharedvec->at(n);
}

template<typename T>
typename series<T>::const_reference
series<T>::at(size_t n) const
{
    return m_sharedvec->at(n);
}

template<typename T>
typename series<T>::reference
series<T>::operator[](size_t n)
{
    unref();
    return (*m_sharedvec)[n];
}

template<typename T>
typename series<T>::const_reference
series<T>::operator[](size_t n) const
{
    return (*m_sharedvec)[n];
}

template<typename T>
typename series<T>::reference
series<T>::front()
{
    unref();
    return m_sharedvec->front();
}

template<typename T>
typename series<T>::const_reference
series<T>::front() const
{
    return m_sharedvec->front();
}

template<typename T>
typename series<T>::reference
series<T>::back()
{
    unref();
    return m_sharedvec->back();
}

template<typename T>
typename series<T>::const_reference
series<T>::back() const
{
    return m_sharedvec->back();
}

template<typename T>
T*
series<T>::data()
{
    unref();
    return m_sharedvec->data();
}

template<typename T>
const T*
series<T>::data() const
{
    return m_sharedvec->data();
}

template<typename T>
typename series<T>::iterator
series<T>::begin()
{
    unref();
    return m_sharedvec->begin();
}

template<typename T>
typename series<T>::const_iterator
series<T>::begin() const
{
    return m_sharedvec->begin();
}

template<typename T>
typename series<T>::const_iterator
series<T>::cbegin() const
{
    return m_sharedvec->cbegin();
}

template<typename T>
typename series<T>::iterator
series<T>::end()
{
    unref();
    return m_sharedvec->end();
}

template<typename T>
typename series<T>::const_iterator
series<T>::end() const
{
    return m_sharedvec->end();
}

template<typename T>
typename series<T>::const_iterator
series<T>::cend() const
{
    return m_sharedvec->cend();
}

template<typename T>
typename series<T>::reverse_iterator
series<T>::rbegin()
{
    unref();
    return m_sharedvec->rbegin();
}

template<typename T>
typename series<T>::const_reverse_iterator
series<T>::rbegin() const
{
    return m_sharedvec->rbegin();
}

template<typename T>
typename series<T>::const_reverse_iterator
series<T>::crbegin() const
{
    return m_sharedvec->crbegin();
}

template<typename T>
typename series<T>::reverse_iterator
series<T>::rend()
{
    unref();
    return m_sharedvec->rend();
}

template<typename T>
typename series<T>::const_reverse_iterator
series<T>::rend() const
{
    return m_sharedvec->rend();
}

template<typename T>
typename series<T>::const_reverse_iterator
series<T>::crend() const
{
    return m_sharedvec->crend();
}

template<typename T>
bool
series<T>::empty() const
{
    return m_sharedvec->empty();
}

template<typename T>
size_t
series<T>::size() const
{
    return m_sharedvec->size();
}

template<typename T>
size_t
series<T>::max_size() const
{
    return m_sharedvec->max_size();
}

template<typename T>
void
series<T>::reserve(size_t _size)
{
    m_sharedvec->reserve(_size);
}

template<typename T>
size_t
series<T>::capacity() const
{
    return m_sharedvec->capacity();
}

template<typename T>
void
series<T>::shrink_to_fit()
{
    m_sharedvec->shrink_to_fit();
}

template<typename T>
void
series<T>::clear()
{
    m_sharedvec = std::make_shared<series_vector<T>>();
}


template<typename T>
typename series<T>::iterator
series<T>::insert(typename series<T>::const_iterator pos, const T& value)
{
    if (m_sharedvec->cbegin() <= pos && pos <= m_sharedvec->cend()) {
        pos = unref(pos);
    }
    return m_sharedvec->insert(pos, value);
}

template<typename T>
typename series<T>::iterator
series<T>::insert(typename series<T>::const_iterator pos, T&& value)
{
    if (m_sharedvec->cbegin() <= pos && pos <= m_sharedvec->cend()) {
        pos = unref(pos);
    }
    return m_sharedvec->insert(pos, std::move(value));
}

template<typename T>
typename series<T>::iterator
series<T>::insert(typename series<T>::const_iterator pos, size_t count, const T& value)
{
    if (m_sharedvec->cbegin() <= pos && pos <= m_sharedvec->cend() && count > 0) {
        pos = unref(pos);
    }
    return m_sharedvec->insert(pos, count, value);
}

template<typename T>
template<typename InputIt>
typename series<T>::iterator
series<T>::insert(typename series<T>::const_iterator pos, InputIt first, InputIt last)
{
    if (m_sharedvec->cbegin() <= pos && pos <= m_sharedvec->cend() && last > first) {
        pos = unref(pos);
    }
    return m_sharedvec->insert(pos, first, last);
}

template<typename T>
typename series<T>::iterator
series<T>::insert(typename series<T>::const_iterator pos, std::initializer_list<T> init)
{
    if (m_sharedvec->cbegin() <= pos && pos <= m_sharedvec->cend() && !init.empty()) {
        pos = unref(pos);
    }
    return m_sharedvec->insert(pos, init);
}

template<typename T>
template<class... Args>
typename series<T>::iterator
series<T>::emplace(typename series<T>::const_iterator pos, Args&&... args)
{
    if (m_sharedvec->cbegin() <= pos && pos <= m_sharedvec->cend()) {
        pos = unref(pos);
    }
    return m_sharedvec->emplace(pos, std::forward<T>(args...));
}


template<typename T>
typename series<T>::iterator
series<T>::erase(typename series<T>::const_iterator pos)
{
    if (m_sharedvec->cbegin() <= pos && pos <= m_sharedvec->cend()) {
        pos = unref(pos);
    }
    return m_sharedvec->erase(pos);
}

template<typename T>
typename series<T>::iterator
series<T>::erase(typename series<T>::const_iterator first, typename series<T>::const_iterator last)
{
    if (m_sharedvec->cbegin() <= first && first < last && last <= m_sharedvec->cend()) {
        auto diff = last - first;
        first     = unref(first);
        last      = first + diff;
    }
    return m_sharedvec->erase(first, last);
}


template<typename T>
void
series<T>::push_back(const T& value)
{
    unref();
    m_sharedvec->push_back(value);
}

template<typename T>
void
series<T>::push_back(T&& value)
{
    unref();
    m_sharedvec->push_back(std::move(value));
}

template<typename T>
template<typename... Args>
typename series<T>::reference
series<T>::emplace_back(Args&&... args)
{
    unref();
    return m_sharedvec->emplace_back(std::forward<T>(args...));
}

template<typename T>
void
series<T>::pop_back()
{
    unref();
    m_sharedvec->pop_back();
}

template<typename T>
void
series<T>::resize(size_t newsize)
{
    if (newsize != m_sharedvec->size()) {
        unref();
        m_sharedvec->resize(newsize);
    }
}

template<typename T>
void
series<T>::resize(size_t newsize, const T& value)
{
    if (newsize != m_sharedvec->size()) {
        unref();
        m_sharedvec->resize(newsize, value);
    }
}

template<typename T>
const std::string&
series<T>::name() const
{
    return m_name;
}


template<typename T>
void
series<T>::set_name(const std::string& name)
{
    m_name = name;
}


template<typename T>
std::vector<std::string>
series<T>::to_string() const
{
    std::vector<std::string> s;
    s.reserve(size());

    for (const T& t : *m_sharedvec) {
        std::stringstream ss;
        ss << std::boolalpha;
        detail::stringify(ss, t, true);
        s.push_back(ss.str());
    }
    return s;
}

template<typename T>
series<T>
series<T>::unique() const
{
    series out;
    out.m_name      = m_name;
    out.m_sharedvec = m_sharedvec->unique();
    return out;
}

template<typename T>
size_t
series<T>::use_count() const
{
    return m_sharedvec.use_count();
}

template<typename T>
bool
series<T>::operator==(const series<T>& other) const
{
    return m_name == other.m_name && *m_sharedvec == *(other.m_sharedvec);
}

template<typename T>
bool
series<T>::operator!=(const series<T>& other) const
{
    return m_name != other.m_name || *m_sharedvec != *(other.m_sharedvec);
}

template<typename T>
void
series<T>::unref()
{
    if (m_sharedvec.use_count() > 1) {
        std::shared_ptr<series_vector<T>> n = std::make_shared<series_vector<T>>(*m_sharedvec);
        m_sharedvec                         = n;
    }
}

// ================= private =================

template<typename T>
typename series<T>::iterator
series<T>::unref(typename series<T>::iterator it)
{
    typename series<T>::iterator newit = it;
    if (m_sharedvec.use_count() > 1) {
        auto oldbegin                       = m_sharedvec->begin();
        std::shared_ptr<series_vector<T>> n = std::make_shared<series_vector<T>>(*m_sharedvec);
        m_sharedvec                         = n;
        newit += (m_sharedvec->begin() - oldbegin);
    }
    return newit;
}

template<typename T>
typename series<T>::const_iterator
series<T>::unref(typename series<T>::const_iterator it)
{
    typename series<T>::const_iterator newit = it;
    if (m_sharedvec.use_count() > 1) {
        auto oldbegin                       = m_sharedvec->cbegin();
        std::shared_ptr<series_vector<T>> n = std::make_shared<series_vector<T>>(*m_sharedvec);
        m_sharedvec                         = n;
        newit += (m_sharedvec->cbegin() - oldbegin);
    }
    return newit;
}


template<typename T>
std::ostream&
operator<<(std::ostream& o, const series<T>& s)
{
    std::ios_base::fmtflags fl(o.flags());

    std::vector<std::string> us = s.to_string();
    auto name                   = s.name();
    auto width                  = std::max(name.size(), detail::get_max_string_length(us));
    const size_t num_rows       = s.size();

    size_t gutter_width =
        num_rows > 0 ? static_cast<size_t>(std::ceil(std::log10(num_rows))) + 1 : 1;

    o << std::boolalpha;
    o << detail::get_emptyspace(gutter_width) << "| " << std::setw(width) << name << "\n";
    o << detail::get_horzrule(gutter_width) << "|" << detail::get_horzrule(width + 2) << "\n";

    for (size_t rowind = 0; rowind < num_rows; ++rowind) {
        std::string& datum = us[rowind];
        o << std::setw(gutter_width) << rowind << "| " << std::setw(width) << datum << "\n";
    }

    o.flags(fl);
    return o;
}

} // namespace mf


#endif // INCLUDED_mainframe_impl_series_h
