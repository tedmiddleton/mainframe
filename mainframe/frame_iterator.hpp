
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_frame_iterator_h
#define INCLUDED_mainframe_frame_iterator_h

#include "mainframe/series.hpp"
#include <iterator>

namespace mf
{

template<typename T>
struct pointerize;

template<typename... Ts>
struct pointerize<std::tuple<series<Ts>...>>
{
    using series_tuple_type = std::tuple<series<Ts>...>;
    using type = std::tuple<Ts*...>;
    using const_type = std::tuple<Ts*...>;

    static type
    op(series_tuple_type& sertup, ptrdiff_t offset = 0)
    {
        type out;
        assign_impl<0>(sertup, offset, out);
        return out;
    }

    template<size_t Ind>
    static void
    assign_impl(series_tuple_type& sertup, ptrdiff_t offset, type& out)
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        series<T>& sertupelem = std::get<Ind>(sertup);
        std::get<Ind>(out) = sertupelem.data() + offset;
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            assign_impl<Ind + 1>(sertup, offset, out);
        }
    }
};

template<size_t Ind>
struct expr_column;

template<typename T>
struct terminal;

template<size_t Ind>
using columnindex = terminal<expr_column<Ind>>;

template<bool IsConst, typename... Ts>
class _row_proxy;

// IsConstDummy is just for template-template params with _row_proxy
template<bool IsConstDummy, typename... Ts>
class _base_frame_row
{
public:
    using row_type = std::true_type;

    _base_frame_row() = default;

    _base_frame_row(const std::tuple<Ts...>& args) : data(args) {}

    _base_frame_row(std::tuple<Ts...>&& args) : data(std::move(args)) {}

    _base_frame_row(const _base_frame_row& other) : data(other.data) {}

    _base_frame_row(_base_frame_row&& other) : data(std::move(other.data)) {}

    template<bool IsConst>
    _base_frame_row(const _row_proxy<IsConst, Ts...>& refs)
    {
        init<0>(refs);
    }

    _base_frame_row&
    operator=(const _base_frame_row& other)
    {
        _base_frame_row<IsConstDummy, Ts...> out{ other };
        std::swap(data, other.data);
        return *this;
    }

    _base_frame_row&
    operator=(_base_frame_row&& other)
    {
        _base_frame_row<IsConstDummy, Ts...> out{ std::move(other) };
        std::swap(data, out.data);
        return *this;
    }

    template<bool IsConst>
    _base_frame_row&
    operator=(const _row_proxy<IsConst, Ts...>& refs)
    {
        _base_frame_row<IsConstDummy, Ts...> out{ refs };
        std::swap(data, out.data);
        return *this;
    }

    template<size_t Ind>
    const typename detail::pack_element<Ind, Ts...>::type&
    at() const
    {
        return std::get<Ind>(data);
    }

    template<size_t Ind>
    typename detail::pack_element<Ind, Ts...>::type&
    at()
    {
        return std::get<Ind>(data);
    }

    template<size_t Ind>
    const typename detail::pack_element<Ind, Ts...>::type&
    at(columnindex<Ind>) const
    {
        return std::get<Ind>(data);
    }

    template<size_t Ind>
    typename detail::pack_element<Ind, Ts...>::type&
    at(columnindex<Ind>)
    {
        return std::get<Ind>(data);
    }

private:
    template<size_t Ind>
    void
    init(const _base_frame_row& refs)
    {
        columnindex<Ind> ci;
        at(ci) = refs.at(ci);
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            init<Ind + 1>(refs);
        }
    }

    template<size_t Ind>
    void
    init(_base_frame_row&& refs)
    {
        columnindex<Ind> ci;
        at(ci) = std::move(refs.at(ci));
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            init<Ind + 1>(std::move(refs));
        }
    }

    template<size_t Ind, bool IsConst>
    void
    init(const _row_proxy<IsConst, Ts...>& refs)
    {
        columnindex<Ind> ci;
        std::get<Ind>(data) = refs.at(ci);
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            init<Ind + 1>(refs);
        }
    }

    std::tuple<Ts...> data;
};

template<typename... Ts>
using frame_row = _base_frame_row<false, Ts...>;

template<bool IsConst, typename... Ts>
class _row_proxy
{
public:
    using row_type = std::true_type;

    template<bool _IsConst = IsConst, std::enable_if_t<!_IsConst, bool> = true>
    _row_proxy&
    operator=(_row_proxy&& row)
    {
        init<0>(std::move(row));
        return *this;
    }

    template<bool _IsConst = IsConst, std::enable_if_t<!_IsConst, bool> = true>
    const _row_proxy&
    operator=(_row_proxy&& row) const
    {
        init<0>(std::move(row));
        return *this;
    }

    template<bool _IsConst = IsConst, std::enable_if_t<!_IsConst, bool> = true>
    _row_proxy&
    operator=(const _row_proxy& row)
    {
        init<0>(row);
        return *this;
    }

    template<bool _IsConst = IsConst, std::enable_if_t<!_IsConst, bool> = true>
    const _row_proxy&
    operator=(const _row_proxy& row) const
    {
        init<0>(row);
        return *this;
    }

    template<bool _IsConst = IsConst, std::enable_if_t<!_IsConst, bool> = true>
    _row_proxy&
    operator=(const frame_row<Ts...>& row)
    {
        init<0>(row);
        return *this;
    }

    template<bool _IsConst = IsConst, std::enable_if_t<!_IsConst, bool> = true>
    const _row_proxy&
    operator=(const frame_row<Ts...>& row) const
    {
        init<0>(row);
        return *this;
    }

    // by-value swap
    template<size_t Ind = 0, bool _IsConst = IsConst, std::enable_if_t<!_IsConst, bool> = true>
    void
    swap_values(_row_proxy& other) noexcept
    {
        using std::swap;
        columnindex<Ind> ci;
        swap(at(ci), other.at(ci));
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            swap_values<Ind + 1>(other);
        }
    }

    template<size_t Ind>
    typename std::conditional<IsConst, const typename detail::pack_element<Ind, Ts...>::type&,
        typename detail::pack_element<Ind, Ts...>::type&>::type
    at() const
    {
        return *std::get<Ind>(m_ptrs);
    }

    template<size_t Ind>
    typename std::conditional<IsConst, const typename detail::pack_element<Ind, Ts...>::type&,
        typename detail::pack_element<Ind, Ts...>::type&>::type
    at(columnindex<Ind>) const
    {
        return *std::get<Ind>(m_ptrs);
    }

    template<size_t Ind>
    typename std::conditional<IsConst, const typename detail::pack_element<Ind, Ts...>::type&,
        typename detail::pack_element<Ind, Ts...>::type&>::type
    operator[](columnindex<Ind>) const
    {
        return *std::get<Ind>(m_ptrs);
    }

    template<size_t Ind>
    typename std::conditional<IsConst, const typename detail::pack_element<Ind, Ts...>::type*,
        typename detail::pack_element<Ind, Ts...>::type*>::type
    data() const
    {
        return std::get<Ind>(m_ptrs);
    }

    template<size_t Ind>
    typename std::conditional<IsConst, const typename detail::pack_element<Ind, Ts...>::type*,
        typename detail::pack_element<Ind, Ts...>::type*>::type
    data(columnindex<Ind>) const
    {
        return std::get<Ind>(m_ptrs);
    }

    bool
    any_missing() const
    {
        return any_missing_impl<0>();
    }

private:
    template<bool IC, bool IR, typename... Us>
    friend class base_frame_iterator;

    template<size_t Ind = 0>
    bool
    any_missing_impl() const
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        const T& elem = at<Ind>();
        if constexpr (detail::is_missing<T>::value) {
            if (!elem.has_value()) {
                return true;
            }
        }
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            return any_missing_impl<Ind + 1>();
        }
        return false;
    }

    // Only base_frame_iterator should be able to create one of these
    _row_proxy(std::tuple<Ts*...> p) : m_ptrs(p) {}

    _row_proxy(const _row_proxy& other) : m_ptrs(other.m_ptrs) {}

    void
    swap_ptrs(_row_proxy& other) noexcept
    {
        std::swap(m_ptrs, other.m_ptrs);
    }

    ptrdiff_t
    addr_diff(const _row_proxy& other) const
    {
        using T = typename detail::pack_element<0, Ts...>::type;
        const T* ptr = std::get<0>(m_ptrs);
        const T* otherptr = std::get<0>(other.m_ptrs);
        return ptr - otherptr;
    }

    bool
    addr_eq(const _row_proxy& other) const
    {
        return std::get<0>(m_ptrs) == std::get<0>(other.m_ptrs);
    }

    bool
    addr_ne(const _row_proxy& other) const
    {
        return std::get<0>(m_ptrs) != std::get<0>(other.m_ptrs);
    }

    bool
    addr_le(const _row_proxy& other) const
    {
        return std::get<0>(m_ptrs) <= std::get<0>(other.m_ptrs);
    }

    bool
    addr_lt(const _row_proxy& other) const
    {
        return std::get<0>(m_ptrs) < std::get<0>(other.m_ptrs);
    }

    bool
    addr_ge(const _row_proxy& other) const
    {
        return std::get<0>(m_ptrs) >= std::get<0>(other.m_ptrs);
    }

    bool
    addr_gt(const _row_proxy& other) const
    {
        return std::get<0>(m_ptrs) > std::get<0>(other.m_ptrs);
    }

    template<size_t Ind = 0>
    void
    addr_modify(ptrdiff_t n)
    {
        std::get<Ind>(m_ptrs) += n;
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            addr_modify<Ind + 1>(n);
        }
    }

    _row_proxy
    new_row_plus_offset(ptrdiff_t off) const
    {
        _row_proxy out(m_ptrs);
        out.addr_modify(off);
        return out;
    }

    template<size_t Ind>
    void
    init(const _row_proxy& vals) const
    {
        columnindex<Ind> ci;
        at(ci) = vals.at(ci);
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            init<Ind + 1>(vals);
        }
    }

    template<size_t Ind>
    void
    init(_row_proxy&& vals) const
    {
        columnindex<Ind> ci;
        at(ci) = std::move(vals.at(ci));
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            init<Ind + 1>(std::move(vals));
        }
    }

    template<size_t Ind>
    void
    init(const frame_row<Ts...>& vals) const
    {
        columnindex<Ind> ci;
        at(ci) = vals.at(ci);
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            init<Ind + 1>(vals);
        }
    }

    std::tuple<Ts*...> m_ptrs;
};

template<bool IsConst, typename... Ts>
void
swap(mf::_row_proxy<IsConst, Ts...>& left, mf::_row_proxy<IsConst, Ts...>& right) noexcept
{
    left.swap_values(right);
}

template<size_t Ind = 0, template<bool, typename...> typename LRow,
    template<bool, typename...> typename RRow, bool LC, bool RC, typename... Ts>
bool
operator==(const LRow<LC, Ts...>& left, const RRow<RC, Ts...>& right)
{
    columnindex<Ind> ci;
    const auto& leftval = left.at(ci);
    const auto& rightval = right.at(ci);
    if (leftval == rightval) {
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            return operator==<Ind + 1>(left, right);
        }
        else {
            return true;
        }
    }
    else {
        return false;
    }
}

template<size_t Ind = 0, template<bool, typename...> typename LRow,
    template<bool, typename...> typename RRow, bool LC, bool RC, typename... Ts>
bool
operator!=(const LRow<LC, Ts...>& left, const RRow<RC, Ts...>& right)
{
    return !(left == right);
}

template<size_t Ind = 0, template<bool, typename...> typename LRow,
    template<bool, typename...> typename RRow, bool LC, bool RC, typename... Ts>
bool
operator<=(const LRow<LC, Ts...>& left, const RRow<RC, Ts...>& right)
{
    columnindex<Ind> ci;
    const auto& leftval = left.at(ci);
    const auto& rightval = right.at(ci);
    if (leftval < rightval) {
        return true;
    }
    else if (leftval == rightval) {
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            return operator<=<Ind + 1>(left, right);
        }
        else {
            return true;
        }
    }
    else {
        return false;
    }
}

template<size_t Ind = 0, template<bool, typename...> typename LRow,
    template<bool, typename...> typename RRow, bool LC, bool RC, typename... Ts>
bool
operator>=(const LRow<LC, Ts...>& left, const RRow<RC, Ts...>& right)
{
    columnindex<Ind> ci;
    const auto& leftval = left.at(ci);
    const auto& rightval = right.at(ci);
    if (leftval > rightval) {
        return true;
    }
    else if (leftval == rightval) {
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            return operator>=<Ind + 1>(left, right);
        }
        else {
            return true;
        }
    }
    else {
        return false;
    }
}

template<size_t Ind = 0, template<bool, typename...> typename LRow,
    template<bool, typename...> typename RRow, bool LC, bool RC, typename... Ts>
bool
operator<(const LRow<LC, Ts...>& left, const RRow<RC, Ts...>& right)
{
    columnindex<Ind> ci;
    const auto& leftval = left.at(ci);
    const auto& rightval = right.at(ci);
    if (leftval < rightval) {
        return true;
    }
    else if (leftval == rightval) {
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            return operator< <Ind + 1>(left, right);
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

template<size_t Ind = 0, template<bool, typename...> typename LRow,
    template<bool, typename...> typename RRow, bool LC, bool RC, typename... Ts>
bool
operator>(const LRow<LC, Ts...>& left, const RRow<RC, Ts...>& right)
{
    columnindex<Ind> ci;
    const auto& leftval = left.at(ci);
    const auto& rightval = right.at(ci);
    if (leftval > rightval) {
        return true;
    }
    else if (leftval == rightval) {
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            return operator><Ind + 1>(left, right);
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

template<bool IsConst, bool IsReverse, typename... Ts>
class base_frame_iterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = ptrdiff_t;
    using value_type = frame_row<Ts...>;
    using reference = _row_proxy<IsConst, Ts...>&;
    using pointer = _row_proxy<IsConst, Ts...>*;
    using const_reference = const _row_proxy<IsConst, Ts...>&;
    using const_pointer = const _row_proxy<IsConst, Ts...>*;

    base_frame_iterator() = default;

    base_frame_iterator(const std::tuple<series<Ts>...>& data, difference_type off = 0)
        : m_row(pointerize<std::tuple<series<Ts>...>>::op(
              const_cast<std::tuple<series<Ts>...>&>(data), off))
    {}

    base_frame_iterator(std::tuple<Ts*...> ptrs) : m_row(ptrs) {}

    base_frame_iterator(_row_proxy<IsConst, Ts...> r) : m_row(r) {}

    base_frame_iterator(const base_frame_iterator& other) : m_row(other.m_row) {}

    base_frame_iterator&
    operator=(const base_frame_iterator& other)
    {
        base_frame_iterator<IsConst, IsReverse, Ts...> out{ other };
        swap(out);
        return *this;
    }

    base_frame_iterator&
    operator++()
    {
        return operator+=(1);
    }
    base_frame_iterator&
    operator++(int)
    {
        return operator+=(1);
    }
    base_frame_iterator&
    operator--()
    {
        return operator-=(1);
    }
    base_frame_iterator&
    operator--(int)
    {
        return operator-=(1);
    }

    base_frame_iterator&
    operator+=(ptrdiff_t n)
    {
        if constexpr (IsReverse) {
            m_row.addr_modify(-n);
        }
        else {
            m_row.addr_modify(n);
        }
        return *this;
    }
    base_frame_iterator&
    operator-=(ptrdiff_t n)
    {
        if constexpr (IsReverse) {
            m_row.addr_modify(n);
        }
        else {
            m_row.addr_modify(-n);
        }
        return *this;
    }

    reference
    operator*()
    {
        return m_row;
    }
    const_reference
    operator*() const
    {
        return m_row;
    }
    pointer
    operator->()
    {
        return &m_row;
    }
    const_pointer
    operator->() const
    {
        return &m_row;
    }

    template<size_t Ind>
    base_sv_iterator<typename detail::pack_element<Ind, Ts...>::type, IsConst, IsReverse>
    column_iterator(columnindex<Ind> ci) const
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        base_sv_iterator<T, IsConst, IsReverse> it{ m_row.data(ci) };
        return it;
    }

    void
    swap(base_frame_iterator& other) noexcept
    {
        m_row.swap_ptrs(other.m_row);
    }

    bool
    operator==(const base_frame_iterator& other) const
    {
        return m_row.addr_eq(other.m_row);
    }

    bool
    operator!=(const base_frame_iterator& other) const
    {
        return m_row.addr_ne(other.m_row);
    }

    bool
    operator<(const base_frame_iterator& other) const
    {
        if constexpr (IsReverse) {
            return m_row.addr_gt(other.m_row);
        }
        else {
            return m_row.addr_lt(other.m_row);
        }
    }

    bool
    operator>(const base_frame_iterator& other) const
    {
        if constexpr (IsReverse) {
            return m_row.addr_lt(other.m_row);
        }
        else {
            return m_row.addr_gt(other.m_row);
        }
    }

    bool
    operator<=(const base_frame_iterator& other) const
    {
        if constexpr (IsReverse) {
            return m_row.addr_ge(other.m_row);
        }
        else {
            return m_row.addr_le(other.m_row);
        }
    }

    bool
    operator>=(const base_frame_iterator& other) const
    {
        if constexpr (IsReverse) {
            return m_row.addr_le(other.m_row);
        }
        else {
            return m_row.addr_ge(other.m_row);
        }
    }

    difference_type
    operator-(const base_frame_iterator& other) const
    {
        if constexpr (IsReverse) {
            return -m_row.addr_diff(other.m_row);
        }
        else {
            return m_row.addr_diff(other.m_row);
        }
    }

    base_frame_iterator
    operator+(ptrdiff_t off) const
    {
        if constexpr (IsReverse) {
            return base_frame_iterator(m_row.new_row_plus_offset(-off));
        }
        else {
            return base_frame_iterator(m_row.new_row_plus_offset(off));
        }
    }

    base_frame_iterator
    operator-(ptrdiff_t off) const
    {
        if constexpr (IsReverse) {
            return base_frame_iterator(m_row.new_row_plus_offset(off));
        }
        else {
            return base_frame_iterator(m_row.new_row_plus_offset(-off));
        }
    }

private:
    _row_proxy<IsConst, Ts...> m_row;
};

template<typename... Ts>
using frame_iterator = base_frame_iterator<false, false, Ts...>;

template<typename... Ts>
using const_frame_iterator = base_frame_iterator<true, false, Ts...>;

template<typename... Ts>
using reverse_frame_iterator = base_frame_iterator<false, true, Ts...>;

template<typename... Ts>
using const_reverse_frame_iterator = base_frame_iterator<true, true, Ts...>;

} // namespace mf

#endif // INCLUDED_mainframe_frame_iterator_h
