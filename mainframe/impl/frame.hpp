//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_frame_impl_h
#define INCLUDED_mainframe_frame_impl_h

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <list>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "mainframe/detail/base.hpp"
#include "mainframe/detail/frame.hpp"
#include "mainframe/detail/simd.hpp"
#include "mainframe/detail/uframe.hpp"
#include "mainframe/expression.hpp"
#include "mainframe/frame.hpp"
#include "mainframe/frame_iterator.hpp"
#include "mainframe/missing.hpp"
#include "mainframe/series.hpp"

namespace mf
{

template<typename... Ts>
frame<Ts...>::frame(const series<typename detail::pack_element<0, Ts...>::type>& s)
{
    std::get<0>(m_columns) = s;
}

template<typename... Ts>
typename frame<Ts...>::iterator
frame<Ts...>::begin()
{
    unref();
    return iterator{ m_columns, 0 };
}

template<typename... Ts>
typename frame<Ts...>::iterator
frame<Ts...>::end()
{
    unref();
    return iterator{ m_columns, static_cast<int>(size()) };
}

template<typename... Ts>
typename frame<Ts...>::const_iterator
frame<Ts...>::begin() const
{
    return const_iterator{ m_columns, 0 };
}

template<typename... Ts>
typename frame<Ts...>::const_iterator
frame<Ts...>::end() const
{
    return const_iterator{ m_columns, static_cast<int>(size()) };
}


template<typename... Ts>
typename frame<Ts...>::const_iterator
frame<Ts...>::cbegin() const
{
    return const_iterator{ m_columns, 0 };
}

template<typename... Ts>
typename frame<Ts...>::const_iterator
frame<Ts...>::cend() const
{
    return const_iterator{ m_columns, static_cast<int>(size()) };
}

template<typename... Ts>
typename frame<Ts...>::reverse_iterator
frame<Ts...>::rbegin()
{
    unref();
    return reverse_iterator{ m_columns, static_cast<int>(size()) - 1 };
}

template<typename... Ts>
typename frame<Ts...>::reverse_iterator
frame<Ts...>::rend()
{
    unref();
    return reverse_iterator{ m_columns, -1 };
}

template<typename... Ts>
typename frame<Ts...>::const_reverse_iterator
frame<Ts...>::rbegin() const
{
    unref();
    return const_reverse_iterator{ m_columns, static_cast<int>(size()) - 1 };
}

template<typename... Ts>
typename frame<Ts...>::const_reverse_iterator
frame<Ts...>::rend() const
{
    unref();
    return const_reverse_iterator{ m_columns, -1 };
}

template<typename... Ts>
typename frame<Ts...>::const_reverse_iterator
frame<Ts...>::crbegin() const
{
    return const_reverse_iterator{ m_columns, static_cast<int>(size()) - 1 };
}

template<typename... Ts>
typename frame<Ts...>::const_reverse_iterator
frame<Ts...>::crend() const
{
    return const_reverse_iterator{ m_columns, -1 };
}

template<typename... Ts>
template<size_t... Inds>
typename detail::add_opt<frame<Ts...>, 0, Inds...>::type
frame<Ts...>::allow_missing(columnindex<Inds>... cols) const
{
    uframe u(*this);
    allow_missing_impl<0, Inds...>(u, cols...);
    return u;
}

template<typename... Ts>
typename detail::add_all_opt<frame<Ts...>>::type
frame<Ts...>::allow_missing() const
{
    uframe u;
    allow_missing_impl<0, Ts...>(u);
    return u;
}

template<typename... Ts>
template<typename T>
frame<Ts..., T>
frame<Ts...>::append_column(const std::string& series_name) const
{
    uframe plust(*this);
    series<T> ns(size());
    ns.set_name(series_name);
    useries us(ns);
    plust.append_column(us);
    return plust;
}

template<typename... Ts>
template<typename T, typename Ex>
frame<Ts..., T>
frame<Ts...>::append_column(const std::string& series_name, Ex expr) const
{
    uframe plust(*this);
    series<T> ns(size());
    ns.set_name(series_name);
    useries us(ns);
    plust.append_column(us);
    frame<Ts..., T> out = plust;
    auto b              = out.begin();
    auto e              = out.end();
    auto it             = b;
    if constexpr (detail::is_missing<T>::value) {
        for (; it != e; ++it) {
            auto val                         = expr(b, it, e);
            it->template at<sizeof...(Ts)>() = val;
        }
    }
    else {
        for (; it != e; ++it) {
            auto val                         = expr(b, it, e);
            auto uval                        = detail::unwrap_missing<decltype(val)>::unwrap(val);
            it->template at<sizeof...(Ts)>() = uval;
        }
    }
    return out;
}

template<typename... Ts>
template<typename T>
frame<Ts..., T>
frame<Ts...>::append_series(const series<T>& s) const
{
    uframe plust(*this);
    useries us(s);
    plust.append_column(us);
    return plust;
}

template<typename... Ts>
void
frame<Ts...>::clear()
{
    clear_impl<0>();
}

template<typename... Ts>
useries
frame<Ts...>::column(const std::string& colname) const
{
    return column_impl<0, Ts...>(colname);
}

template<typename... Ts>
template<size_t Ind>
series<typename detail::pack_element<Ind, Ts...>::type>&
frame<Ts...>::column(columnindex<Ind>)
{
    return std::get<Ind>(m_columns);
}

template<typename... Ts>
template<size_t Ind>
const series<typename detail::pack_element<Ind, Ts...>::type>&
frame<Ts...>::column(columnindex<Ind>) const
{
    return std::get<Ind>(m_columns);
}

template<typename... Ts>
template<size_t Ind>
std::string
frame<Ts...>::column_name() const
{
    return std::get<Ind>(m_columns).name();
}

template<typename... Ts>
template<size_t Ind>
std::string
frame<Ts...>::column_name(columnindex<Ind>) const
{
    return std::get<Ind>(m_columns).name();
}

template<typename... Ts>
typename frame<Ts...>::name_array
frame<Ts...>::column_names() const
{
    std::array<std::string, sizeof...(Ts)> out;
    column_names_impl<0>(out);
    return out;
}

template<typename... Ts>
template<size_t... Inds>
typename detail::rearrange<frame<Ts...>, Inds...>::type
frame<Ts...>::columns(columnindex<Inds>... cols) const
{
    uframe f;
    columns_impl(f, cols...);
    return f;
}

template<typename... Ts>
template<typename... Us>
uframe
frame<Ts...>::columns(const Us&... us) const
{
    uframe f;
    columns_impl(f, us...);
    return f;
}

template<typename... Ts>
template<size_t Ind1, size_t Ind2>
double
frame<Ts...>::corr(terminal<expr_column<Ind1>>, terminal<expr_column<Ind2>>) const
{
    using T1             = typename detail::pack_element<Ind1, Ts...>::type;
    using T2             = typename detail::pack_element<Ind2, Ts...>::type;
    const series<T1>& s1 = std::get<Ind1>(m_columns);
    const series<T2>& s2 = std::get<Ind2>(m_columns);

    double c = detail::correlate_pearson(s1.data(), s2.data(), s1.size());
    return c;
}

template<typename... Ts>
template<size_t... Inds>
typename detail::remove_opt<frame<Ts...>, 0, Inds...>::type
frame<Ts...>::disallow_missing(columnindex<Inds>... cols) const
{
    uframe u(*this);
    disallow_missing_impl<0, Inds...>(u, cols...);
    return u;
}

template<typename... Ts>
typename detail::remove_all_opt<frame<Ts...>>::type
frame<Ts...>::disallow_missing() const
{
    uframe u;
    disallow_missing_impl<0, Ts...>(u);
    return u;
}

template<typename... Ts>
frame<Ts...>
frame<Ts...>::drop_missing() const
{
    frame<Ts...> out;
    out.set_column_names(column_names());

    auto b    = cbegin();
    auto curr = b;
    auto e    = cend();
    for (; curr != e; ++curr) {
        auto& row = *curr;
        if (!row.any_missing()) {
            out.push_back(row);
        }
    }
    return out;
}

template<typename... Ts>
bool
frame<Ts...>::empty() const
{
    return std::get<0>(m_columns).empty();
}


template<typename... Ts>
typename frame<Ts...>::iterator
frame<Ts...>::erase(typename frame<Ts...>::iterator first, typename frame<Ts...>::iterator last)
{
    std::tuple<Ts*...> ptrs;
    erase_impl<0>(ptrs, first, last);
    return typename frame<Ts...>::iterator{ ptrs };
}

template<typename... Ts>
typename frame<Ts...>::iterator
frame<Ts...>::erase(typename frame<Ts...>::iterator pos)
{
    std::tuple<Ts*...> ptrs;
    erase_impl<0>(ptrs, pos);
    return typename frame<Ts...>::iterator{ ptrs };
}

template<typename... Ts>
frame<Ts...>
frame<Ts...>::fill_forward() const
{
    frame<Ts...> out;
    out.set_column_names(column_names());

    auto b    = cbegin();
    auto curr = b;
    auto e    = cend();
    for (; curr != e; ++curr) {
        auto& row = *curr;
        if (out.size() == 0 || !row.any_missing()) {
            out.push_back(row);
        }
        else {
            frame_row<Ts...> fr{ row };
            fr.replace_missing(*(out.end() - 1));
            out.push_back(fr);
        }
    }
    return out;
}

template<typename... Ts>
frame<Ts...>
frame<Ts...>::fill_backward() const
{
    frame<Ts...> out;
    out.set_column_names(column_names());

    auto b    = crbegin();
    auto curr = b;
    auto e    = crend();
    for (; curr != e; ++curr) {
        auto& row = *curr;
        if (out.size() == 0 || !row.any_missing()) {
            out.push_back(row);
        }
        else {
            frame_row<Ts...> fr{ row };
            fr.replace_missing(*(out.end() - 1));
            out.push_back(fr);
        }
    }
    return out.reversed();
}

template<typename... Ts>
template<size_t... Idx>
group<index_defn<Idx...>, Ts...>
frame<Ts...>::groupby(columnindex<Idx>...) const
{
    return group<index_defn<Idx...>, Ts...>{ *this };
}

template<typename... Ts>
template<typename... Us>
frame<Ts..., Us...>
frame<Ts...>::hcat(frame<Us...> other) const
{
    frame<Ts...> self(*this);
    size_t max_size = std::max(self.size(), other.size());
    // Redundant resize will be handled at the series level to prevent an
    // unnecessary unref
    self.resize(max_size);
    other.resize(max_size);

    uframe out(self);
    uframe uother(other);
    for (size_t c = 0; c < other.num_columns(); ++c) {
        out.append_column(uother.column(c));
    }

    return out;
}

template<typename... Ts>
typename frame<Ts...>::iterator
frame<Ts...>::insert(typename frame<Ts...>::iterator pos,
    typename frame<Ts...>::const_iterator first, typename frame<Ts...>::const_iterator last)
{
    std::tuple<Ts*...> ptrs;
    insert_impl<0>(ptrs, pos, first, last);
    return typename frame<Ts...>::iterator{ ptrs };
}

template<typename... Ts>
typename frame<Ts...>::iterator
frame<Ts...>::insert(typename frame<Ts...>::iterator pos, const Ts&... ts)
{
    std::tuple<Ts*...> ptrs;
    insert_impl<0>(ptrs, pos, 1, ts...);
    return typename frame<Ts...>::iterator{ ptrs };
}

template<typename... Ts>
typename frame<Ts...>::iterator
frame<Ts...>::insert(typename frame<Ts...>::iterator pos, size_t count, const Ts&... ts)
{
    std::tuple<Ts*...> ptrs;
    insert_impl<0>(ptrs, pos, count, ts...);
    return typename frame<Ts...>::iterator{ ptrs };
}

template<typename... Ts>
template<size_t Ind>
double
frame<Ts...>::mean(columnindex<Ind>) const
{
    const auto& s = std::get<Ind>(m_columns);
    return s.mean();
}

template<typename... Ts>
template<size_t Ind>
typename frame<Ts...>::template pack_elem_pair<Ind>
frame<Ts...>::minmax(columnindex<Ind>) const
{
    const auto& s = std::get<Ind>(m_columns);
    return s.minmax();
}

template<typename... Ts>
size_t
frame<Ts...>::num_columns() const
{
    return sizeof...(Ts);
}

template<typename... Ts>
frame<Ts...>
frame<Ts...>::operator+(const frame<Ts...>& other) const
{
    frame<Ts...> out{ *this };
    out.insert(out.end(), other.cbegin(), other.cend());
    return out;
}

template<typename... Ts>
bool
frame<Ts...>::operator==(const frame<Ts...>& other) const
{
    return eq_impl<0>(other);
}

template<typename... Ts>
template<size_t Num>
typename std::enable_if<Num == 1, typename detail::pack_element<0, Ts...>::type>::type
frame<Ts...>::operator*() const
{
    if (size() == 0) {
        throw std::out_of_range("Cannot dereference empty frame");
    }
    return std::get<0>(m_columns).at(0);
}

template<typename... Ts>
template<typename Ex>
std::enable_if_t<is_complex_expression<Ex>::value, frame<Ts...>>
frame<Ts...>::operator[](Ex ex) const
{
    return rows(ex);
}

template<typename... Ts>
frame<Ts...>
frame<Ts...>::operator[](size_t ind) const
{
    frame<Ts...> out;
    out.push_back(*(cbegin() + ind));
    return out;
}

#if __cplusplus <= 202002L

template<typename... Ts>
template<size_t... Inds>
typename detail::rearrange<frame<Ts...>, Inds...>::type
frame<Ts...>::operator[](columnindexpack<Inds...> cols) const
{
    uframe u;
    columns_impl(u, cols);
    return u;
}

template<typename... Ts>
template<size_t Ind>
typename detail::rearrange<frame<Ts...>, Ind>::type
frame<Ts...>::operator[](columnindex<Ind>) const
{
    uframe u;
    columnindexpack<Ind> cols;
    columns_impl(u, cols);
    return u;
}

#else

template<typename... Ts>
template<size_t... Inds>
typename detail::rearrange<frame<Ts...>, Inds...>::type
frame<Ts...>::operator[](columnindex<Inds>... cols) const
{
    return columns(cols...);
}

#endif

template<typename... Ts>
void
frame<Ts...>::pop_back()
{
    pop_back_impl<0>();
}

template<typename... Ts>
template<typename T>
frame<T, Ts...>
frame<Ts...>::prepend_column(const std::string& series_name) const
{
    uframe plust(*this);
    series<T> ns(size());
    ns.set_name(series_name);
    useries us(ns);
    plust.prepend_column(us);
    return plust;
}

template<typename... Ts>
template<typename T, typename Ex>
frame<T, Ts...>
frame<Ts...>::prepend_column(const std::string& series_name, Ex expr) const
{
    uframe plust(*this);
    series<T> ns(size());
    ns.set_name(series_name);
    useries us(ns);
    plust.prepend_column(us);
    frame<T, Ts...> out = plust;
    auto ib             = begin();
    auto ie             = end();
    auto ob             = out.begin();
    auto oe             = out.end();
    auto iit            = ib;
    auto oit             = ob;
    if constexpr (detail::is_missing<T>::value) {
        for (; oit != oe; ++oit, ++iit) {
            auto val                         = expr(ib, iit, ie);
            oit->template at<0>() = val;
        }
    }
    else {
        for (; oit != oe; ++oit, ++iit) {
            auto val                         = expr(ib, iit, ie);
            auto uval                        = detail::unwrap_missing<decltype(val)>::unwrap(val);
            oit->template at<0>() = uval;
        }
    }
    return out;
}

template<typename... Ts>
template<typename T>
frame<T, Ts...>
frame<Ts...>::prepend_series(const series<T>& s) const
{
    uframe plust(*this);
    useries us(s);
    plust.prepend_column(us);
    return plust;
}

template<typename... Ts>
template<bool IsConst, typename... Us, typename... Vs>
void
frame<Ts...>::push_back(const _row_proxy<IsConst, Us...>& fr, const Vs&... args)
{
    push_back_multiple_row_impl<0>(fr, args...);
}

template<typename... Ts>
template<typename... Us, typename... Vs>
void
frame<Ts...>::push_back(const frame_row<Us...>& fr, const Vs&... args)
{
    push_back_multiple_row_impl<0>(fr, args...);
}

template<typename... Ts>
template<typename U, typename... Us>
void
frame<Ts...>::push_back(U first_arg, Us... args)
{
    push_back_impl<0, U, Us...>(first_arg, args...);
}

template<typename... Ts>
template<size_t Ind>
typename frame<Ts...>::template frame_without_indexed_column<Ind> frame<Ts...>::remove_column(columnindex<Ind>)
{
    uframe u(*this);
    u.remove_column(Ind);
    return u;
}

template<typename... Ts>
void
frame<Ts...>::reserve(size_t newsize)
{
    reserve_impl<0>(newsize);
}

template<typename... Ts>
void
frame<Ts...>::resize(size_t newsize)
{
    resize_impl<0>(newsize);
}

template<typename... Ts>
frame<Ts...>
frame<Ts...>::reversed() const
{
    frame<Ts...> out;
    out.set_column_names(column_names());
    for (auto it(crbegin()); it != crend(); ++it) {
        out.push_back(*it);
    }
    return out;
}

template<typename... Ts>
template<size_t... Inds>
void
frame<Ts...>::reverse_sort(columnindex<Inds>...)
{
    detail::build_gt<row_type, Inds...> op;
    std::sort(this->begin(), this->end(), op);
}

template<typename... Ts>
template<size_t... Inds>
frame<Ts...>
frame<Ts...>::reverse_sorted(columnindex<Inds>... ci)
{
    frame<Ts...> out(*this);
    out.set_column_names(column_names());
    out.reverse_sort(ci...);
    return out;
}

template<typename... Ts>
_row_proxy<false, Ts...>
frame<Ts...>::row(size_t ind)
{
    _row_proxy<false, Ts...> out{ m_columns, static_cast<ptrdiff_t>(ind) };
    return out;
}

template<typename... Ts>
_row_proxy<true, Ts...>
frame<Ts...>::row(size_t ind) const
{
    _row_proxy<true, Ts...> out{ m_columns, static_cast<ptrdiff_t>(ind) };
    return out;
}

template<typename... Ts>
template<typename Ex>
std::enable_if_t<is_expression<Ex>::value, frame<Ts...>>
frame<Ts...>::rows(Ex ex) const
{
    frame<Ts...> out;
    out.set_column_names(column_names());

    auto b    = cbegin();
    auto curr = b;
    auto e    = cend();
    for (; curr != e; ++curr) {
        auto exprval = ex(b, curr, e);
        if (exprval) {
            out.push_back(*curr);
        }
    }

    return out;
}

template<typename... Ts>
void
frame<Ts...>::set_column_names(const std::vector<std::string>& names)
{
    set_column_names_impl<0>(names);
}

template<typename... Ts>
void
frame<Ts...>::set_column_names(const name_array& names)
{
    set_column_names_impl<0>(names);
}

template<typename... Ts>
template<typename... Us>
void
frame<Ts...>::set_column_names(const Us&... colnames)
{
    set_column_names_impl<0, Us...>(colnames...);
}

template<typename... Ts>
size_t
frame<Ts...>::size() const
{
    return size_impl_with_check<0, Ts...>();
}

template<typename... Ts>
template<size_t... Inds>
void
frame<Ts...>::sort(columnindex<Inds>...)
{
    detail::build_lt<row_type, Inds...> op;
    std::sort(this->begin(), this->end(), op);
}

template<typename... Ts>
template<size_t... Inds>
frame<Ts...>
frame<Ts...>::sorted(columnindex<Inds>... ci)
{
    frame<Ts...> out(*this);
    out.set_column_names(column_names());
    out.sort(ci...);
    return out;
}

template<typename... Ts>
template<size_t Ind>
double
frame<Ts...>::stddev(columnindex<Ind>) const
{
    const auto& s = std::get<Ind>(m_columns);
    return s.stddev();
}

template<typename... Ts>
std::vector<std::vector<std::string>>
frame<Ts...>::to_string() const
{
    std::vector<std::vector<std::string>> out;
    to_string_impl<0, Ts...>(out);
    return out;
}

// ================ private =================

template<typename... Ts>
template<size_t Ind, size_t... Inds>
void
frame<Ts...>::allow_missing_impl(uframe& uf, columnindex<Inds>... cols) const
{
    if constexpr (detail::contains<Ind, Inds...>::value) {
        columnindex<Ind> ci;
        auto& s  = column(ci);
        auto ams = s.allow_missing();
        uf.set_column(Ind, ams);
    }
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        allow_missing_impl<Ind + 1, Inds...>(uf, cols...);
    }
}

template<typename... Ts>
template<size_t Ind, typename U, typename... Us>
void
frame<Ts...>::allow_missing_impl(uframe& uf) const
{
    const series<U>& s = std::get<Ind>(m_columns);
    auto os            = s.allow_missing();
    uf.append_column(os);
    if constexpr (sizeof...(Us) > 0) {
        allow_missing_impl<Ind + 1, Us...>(uf);
    }
}

template<typename... Ts>
template<size_t Ind>
void
frame<Ts...>::clear_impl()
{
    std::get<Ind>(m_columns).clear();
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        clear_impl<Ind + 1>();
    }
}

template<typename... Ts>
template<size_t Ind, typename U, typename... Us>
useries
frame<Ts...>::column_impl(const std::string& colname) const
{
    const series<U>& s = std::get<Ind>(m_columns);
    if (s.name() == colname) {
        return s;
    }
    if constexpr (sizeof...(Us) > 0) {
        return column_impl<Ind + 1, Us...>(colname);
    }
    else {
        throw std::out_of_range{ "out of range" };
    }
}

template<typename... Ts>
template<size_t Ind>
void
frame<Ts...>::column_names_impl(std::array<std::string, sizeof...(Ts)>& out) const
{
    auto& s  = std::get<Ind>(m_columns);
    out[Ind] = s.name();
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        column_names_impl<Ind + 1>(out);
    }
}

template<typename... Ts>
template<typename U, typename... Us>
void
frame<Ts...>::columns_impl(uframe& f, const U& u, const Us&... us) const
{
    useries s = column(u);
    f.append_column(s);
    if constexpr (sizeof...(Us) > 0) {
        columns_impl(f, us...);
    }
}

template<typename... Ts>
template<size_t Ind, size_t... RemInds>
void
frame<Ts...>::columns_impl(uframe& f, columnindexpack<Ind, RemInds...>) const
{
    columnindex<Ind> ci;
    useries s = column(ci);
    f.append_column(s);
    if constexpr (sizeof...(RemInds) > 0) {
        columnindexpack<RemInds...> remcis;
        columns_impl(f, remcis);
    }
}

template<typename... Ts>
template<size_t Ind, size_t... Inds>
void
frame<Ts...>::disallow_missing_impl(uframe& uf, columnindex<Inds>... cols) const
{
    if constexpr (detail::contains<Ind, Inds...>::value) {
        columnindex<Ind> ci;
        auto& s  = column(ci);
        auto ams = s.disallow_missing();
        uf.set_column(Ind, ams);
    }
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        disallow_missing_impl<Ind + 1, Inds...>(uf, cols...);
    }
}

template<typename... Ts>
template<size_t Ind, typename U, typename... Us>
void
frame<Ts...>::disallow_missing_impl(uframe& uf) const
{
    const series<U>& s = std::get<Ind>(m_columns);
    auto os            = s.disallow_missing();
    uf.append_column(os);
    if constexpr (sizeof...(Us) > 0) {
        disallow_missing_impl<Ind + 1, Us...>(uf);
    }
}

template<typename... Ts>
template<size_t Ind>
void
frame<Ts...>::erase_impl(std::tuple<Ts*...>& ptrs, iterator pos)
{
    columnindex<Ind> ci;
    auto& s             = std::get<Ind>(m_columns);
    auto column_pos     = pos.column_iterator(ci);
    auto newcpos        = s.erase(column_pos);
    std::get<Ind>(ptrs) = newcpos.data();
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        erase_impl<Ind + 1>(ptrs, pos);
    }
}

template<typename... Ts>
template<size_t Ind>
void
frame<Ts...>::erase_impl(std::tuple<Ts*...>& ptrs, iterator first, iterator last)
{
    columnindex<Ind> ci;
    auto& s             = std::get<Ind>(m_columns);
    auto column_first   = first.column_iterator(ci);
    auto column_last    = last.column_iterator(ci);
    auto newcpos        = s.erase(column_first, column_last);
    std::get<Ind>(ptrs) = newcpos.data();
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        erase_impl<Ind + 1>(ptrs, first, last);
    }
}

template<typename... Ts>
template<size_t Ind>
bool
frame<Ts...>::eq_impl(const frame<Ts...>& other) const
{
    auto& s  = std::get<Ind>(m_columns);
    auto& os = std::get<Ind>(other.m_columns);
    if (s != os) {
        return false;
    }
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        return eq_impl<Ind + 1>(other);
    }
    return true;
}

template<typename... Ts>
template<size_t Ind, typename U, typename... Us>
void
frame<Ts...>::insert_impl(
    std::tuple<Ts*...>& ptrs, iterator pos, size_t count, const U& u, const Us&... us)
{
    columnindex<Ind> ci;
    auto& s             = std::get<Ind>(m_columns);
    auto column_pos     = pos.column_iterator(ci);
    auto newcpos        = s.insert(column_pos, count, u);
    std::get<Ind>(ptrs) = newcpos.data();
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        insert_impl<Ind + 1>(ptrs, pos, count, us...);
    }
}

template<typename... Ts>
template<size_t Ind>
void
frame<Ts...>::insert_impl(
    std::tuple<Ts*...>& ptrs, iterator pos, const_iterator first, const_iterator last)
{
    columnindex<Ind> ci;
    auto& s             = std::get<Ind>(m_columns);
    auto column_pos     = pos.column_iterator(ci);
    auto column_first   = first.column_iterator(ci);
    auto column_last    = last.column_iterator(ci);
    auto newcpos        = s.insert(column_pos, column_first, column_last);
    std::get<Ind>(ptrs) = newcpos.data();
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        insert_impl<Ind + 1>(ptrs, pos, first, last);
    }
}

template<typename... Ts>
template<size_t Ind>
void
frame<Ts...>::pop_back_impl()
{
    auto& s = std::get<Ind>(m_columns);
    s.pop_back();
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        pop_back_impl<Ind + 1>();
    }
}

template<typename... Ts>
void
frame<Ts...>::populate(const std::vector<useries>& columns)
{
    populate_impl<0, Ts...>(columns);
}

template<typename... Ts>
template<size_t Ind, typename U, typename... Us>
void
frame<Ts...>::populate_impl(const std::vector<useries>& columns)
{
    const useries& su = columns.at(Ind);
    series<U>& s      = std::get<Ind>(m_columns);
    s                 = series<U>(su);
    if constexpr (sizeof...(Us) > 0) {
        populate_impl<Ind + 1, Us...>(columns);
    }
}

template<typename... Ts>
template<size_t Ind>
void
frame<Ts...>::push_back_multiple_empty_impl()
{
    using T = typename pack_element<Ind, Ts...>::type;
    // Default ctor
    std::get<Ind>(m_columns).push_back(T{});
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        push_back_multiple_empty_impl<Ind + 1>();
    }
}

template<typename... Ts>
template<size_t Ind, typename V, typename... Vs>
void
frame<Ts...>::push_back_multiple_args_impl(V arg, const Vs&... args)
{
    // Should really try some kind of conversion here
    std::get<Ind>(m_columns).push_back(arg);
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        if constexpr (sizeof...(Vs) > 0) {
            push_back_multiple_args_impl<Ind + 1>(args...);
        }
        else {
            push_back_multiple_empty_impl<Ind + 1>();
        }
    }
}

template<typename... Ts>
template<size_t Ind, bool IsConst, template<bool, typename...> typename Row, typename... Us,
    typename... Vs>
void
frame<Ts...>::push_back_multiple_row_impl(const Row<IsConst, Us...>& fr, const Vs&... args)
{
    // Should really try some kind of conversion here
    columnindex<Ind> ci;
    std::get<Ind>(m_columns).push_back(fr.at(ci));
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        if constexpr (Ind + 1 < sizeof...(Us)) {
            push_back_multiple_row_impl<Ind + 1>(fr, args...);
        }
        else if constexpr (sizeof...(Vs) > 0) {
            push_back_multiple_args_impl<Ind + 1>(args...);
        }
        else {
            push_back_multiple_empty_impl<Ind + 1>();
        }
    }
}

template<typename... Ts>
template<size_t Ind, typename U, typename... Us>
void
frame<Ts...>::push_back_impl(const U& u, const Us&... us)
{
    using T = typename detail::pack_element<Ind, Ts...>::type;
    std::get<Ind>(m_columns).push_back(static_cast<T>(u));
    if constexpr (sizeof...(Us) > 0) {
        push_back_impl<Ind + 1>(us...);
    }
}

template<typename... Ts>
template<size_t Ind>
void
frame<Ts...>::reserve_impl(size_t newsize)
{
    auto& s = std::get<Ind>(m_columns);
    s.reserve(newsize);
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        reserve_impl<Ind + 1>(newsize);
    }
}

template<typename... Ts>
template<size_t Ind>
void
frame<Ts...>::resize_impl(size_t newsize)
{
    auto& s = std::get<Ind>(m_columns);
    s.resize(newsize);
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        resize_impl<Ind + 1>(newsize);
    }
}

template<typename... Ts>
template<size_t Ind>
void
frame<Ts...>::set_column_names_impl(const std::vector<std::string>& names)
{
    if (Ind < names.size()) {
        auto& s = std::get<Ind>(m_columns);
        s.set_name(names.at(Ind));
    }
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        set_column_names_impl<Ind + 1>(names);
    }
}

template<typename... Ts>
template<size_t Ind>
void
frame<Ts...>::set_column_names_impl(const std::array<std::string, sizeof...(Ts)>& names)
{
    auto& s = std::get<Ind>(m_columns);
    s.set_name(names[Ind]);
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        set_column_names_impl<Ind + 1>(names);
    }
}

template<typename... Ts>
template<size_t Ind, typename U, typename... Us>
void
frame<Ts...>::set_column_names_impl(const U& colname, const Us&... colnames)
{
    auto& s = std::get<Ind>(m_columns);
    s.set_name(colname);
    if constexpr (sizeof...(Us) > 0) {
        set_column_names_impl<Ind + 1, Us...>(colnames...);
    }
}

template<typename... Ts>
template<size_t Ind, typename U, typename... Us>
size_t
frame<Ts...>::size_impl_with_check() const
{
    const series<U>& series = std::get<Ind>(m_columns);
    size_t s                = series.size();
    if constexpr (sizeof...(Us) > 0) {
        size_t si = size_impl_with_check<Ind + 1, Us...>();
        if (si != s) {
            throw std::logic_error{ "Inconsistent sizes!" };
        }
    }
    return s;
}

template<typename... Ts>
template<size_t Ind, typename U, typename... Us>
void
frame<Ts...>::to_string_impl(std::vector<std::vector<std::string>>& strs) const
{
    const series<U>& s               = std::get<Ind>(m_columns);
    std::vector<std::string> colstrs = s.to_string();
    strs.push_back(colstrs);
    if constexpr (sizeof...(Us) > 0) {
        to_string_impl<Ind + 1, Us...>(strs);
    }
}

template<typename... Ts>
template<size_t Ind>
void
frame<Ts...>::unref()
{
    using T      = typename detail::pack_element<Ind, Ts...>::type;
    series<T>& s = std::get<Ind>(m_columns);
    s.unref();
    if constexpr (Ind + 1 < sizeof...(Ts)) {
        unref<Ind + 1>();
    }
}

template<typename... Ts>
std::ostream&
operator<<(std::ostream& o, const frame<Ts...>& f)
{
    std::ios_base::fmtflags fl(o.flags());

    std::vector<std::vector<std::string>> uf = f.to_string();
    auto names                               = f.column_names();
    auto widths                              = detail::get_max_string_lengths(uf);
    constexpr size_t num_columns             = sizeof...(Ts);
    const size_t num_rows                    = f.size();

    auto gutter_width = num_rows > 0 ? static_cast<size_t>(std::ceil(std::log10(num_rows))) + 1 : 1;

    o << std::boolalpha;
    o << detail::get_emptyspace(gutter_width);
    for (size_t i = 0; i < num_columns; ++i) {
        auto& name  = names[i];
        auto& width = widths[i];
        width       = std::max(width, name.size());
        o << "| " << std::setw(width) << name << " ";
    }
    o << "\n";

    o << detail::get_horzrule(gutter_width);
    for (size_t i = 0; i < num_columns; ++i) {
        auto& width = widths[i];
        o << "|" << detail::get_horzrule(width + 2);
    }
    o << "\n";

    for (size_t rowind = 0; rowind < num_rows; ++rowind) {
        o << std::setw(gutter_width) << rowind;
        for (size_t colind = 0; colind < num_columns; ++colind) {

            std::string& datum = uf[colind][rowind];
            auto width         = widths[colind];

            o << "| " << std::setw(width) << datum << " ";
        }
        o << "\n";
    }

    o.flags(fl);
    return o;
}

} // namespace mf

#endif // INCLUDED_mainframe_frame_h
