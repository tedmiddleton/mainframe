//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_detail_row_proxy_h
#define INCLUDED_mainframe_detail_row_proxy_h

#include "mainframe/series.hpp"
#include "mainframe/columnindex.hpp"
#include "mainframe/row_decl.hpp"
#include <iterator>

namespace mf
{
template<bool IsConst, typename... Ts>
class _row_proxy
{
public:
    using row_type = std::true_type;
    using tuple_type =
        typename std::conditional<IsConst, std::tuple<const Ts*...>, std::tuple<Ts*...>>::type;

    _row_proxy(const _row_proxy& other)
        : m_ptrs(other.m_ptrs)
    {}

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
    template<typename... Us>
    friend class frame;

    template<size_t Ind = 0>
    bool
    any_missing_impl() const
    {
        using T       = typename detail::pack_element<Ind, Ts...>::type;
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

    // Only frame or base_frame_iterator should be able to create one of these
    template<bool _IsConst = IsConst, std::enable_if_t<_IsConst, bool> = true>
    _row_proxy(const std::tuple<series<Ts>...>& data, ptrdiff_t off = 0)
    {
        init<0>(data, off);
    }

    template<bool _IsConst = IsConst, std::enable_if_t<!_IsConst, bool> = true>
    _row_proxy(std::tuple<series<Ts>...>& data, ptrdiff_t off = 0)
    {
        init<0>(data, off);
    }

    template<size_t Ind>
    void
    init(std::tuple<series<Ts>...>& data, ptrdiff_t off)
    {
        using T               = typename detail::pack_element<Ind, Ts...>::type;
        series<T>& s          = std::get<Ind>(data);
        std::get<Ind>(m_ptrs) = s.data() + off;
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            init<Ind + 1>(data, off);
        }
    }

    template<size_t Ind>
    void
    init(const std::tuple<series<Ts>...>& data, ptrdiff_t off)
    {
        using T               = typename detail::pack_element<Ind, Ts...>::type;
        const series<T>& s    = std::get<Ind>(data);
        std::get<Ind>(m_ptrs) = s.data() + off;
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            init<Ind + 1>(data, off);
        }
    }

    _row_proxy(tuple_type p)
        : m_ptrs(p)
    {}

    void
    swap_ptrs(_row_proxy& other) noexcept
    {
        std::swap(m_ptrs, other.m_ptrs);
    }

    ptrdiff_t
    addr_diff(const _row_proxy& other) const
    {
        using T           = typename detail::pack_element<0, Ts...>::type;
        const T* ptr      = std::get<0>(m_ptrs);
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

    tuple_type m_ptrs;
};

template<size_t Ind = 0, bool IsConst, typename... Ts>
std::ostream&
operator<<(std::ostream& o, const _row_proxy<IsConst, Ts...>& fr)
{
    columnindex<Ind> ci;

    if constexpr (sizeof...(Ts) == 0) {
        o << "[]";
        return o;
    }

    if constexpr (Ind == 0) {
        o << "[ ";
    }

    detail::stringify(o, fr.at(ci), true);

    if constexpr (Ind + 1 < sizeof...(Ts)) {
        o << ", ";
        operator<< <Ind + 1>(o, fr);
    }
    else {
        o << " ]";
    }
    return o;
}

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
    const auto& leftval  = left.at(ci);
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
    const auto& leftval  = left.at(ci);
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
    const auto& leftval  = left.at(ci);
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
    const auto& leftval  = left.at(ci);
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
    const auto& leftval  = left.at(ci);
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

} // namespace mf

namespace std
{
template<bool IsConst, typename... Ts>
struct hash<mf::_row_proxy<IsConst, Ts...>>
{
    template<size_t Ind = 0>
    size_t
    operator()(const mf::_row_proxy<IsConst, Ts...>& fr) const
    {
        using T = typename mf::detail::pack_element<Ind, Ts...>::type;
        std::hash<T> hasher;
        mf::columnindex<Ind> ci;
        size_t out = hasher(fr.at(ci));
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            out ^= operator()<Ind + 1>(fr);
        }
        return out;
    }
};
} // namespace std

#endif // INCLUDED_mainframe_detail_row_proxy_h

