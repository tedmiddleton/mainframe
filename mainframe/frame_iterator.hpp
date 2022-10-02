//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_frame_iterator_h
#define INCLUDED_mainframe_frame_iterator_h

#include <iterator>
#include "mainframe/series.hpp"
#include "mainframe/columnindex.hpp"
#include "mainframe/row_decl.hpp"
#include "mainframe/detail/row_proxy.hpp"

namespace mf
{
template<bool IsConst, bool IsReverse, typename... Ts>
class base_frame_iterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = frame_row<Ts...>;
    using difference_type   = ptrdiff_t;
    using reference         = _row_proxy<IsConst, Ts...>&;
    using pointer           = _row_proxy<IsConst, Ts...>*;
    using const_reference   = const _row_proxy<IsConst, Ts...>&;
    using const_pointer     = const _row_proxy<IsConst, Ts...>*;

    base_frame_iterator() = default;

    template<bool _IsConst = IsConst, std::enable_if_t<_IsConst, bool> = true>
    explicit base_frame_iterator(const std::tuple<series<Ts>...>& data, ptrdiff_t off = 0)
        : m_row(data, off)
    {}

    template<bool _IsConst = IsConst, std::enable_if_t<!_IsConst, bool> = true>
    explicit base_frame_iterator(std::tuple<series<Ts>...>& data, ptrdiff_t off = 0)
        : m_row(data, off)
    {}

    explicit base_frame_iterator(std::tuple<Ts*...> ptrs)
        : m_row(ptrs)
    {}

    base_frame_iterator(_row_proxy<IsConst, Ts...> r)
        : m_row(r)
    {}

    base_frame_iterator(const base_frame_iterator& other)
        : m_row(other.m_row)
    {}

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
    detail::base_sv_iterator<typename detail::pack_element<Ind, Ts...>::type, IsConst, IsReverse>
    column_iterator(columnindex<Ind> ci) const
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        detail::base_sv_iterator<T, IsConst, IsReverse> it{ m_row.data(ci) };
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

    ptrdiff_t
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

} // namespace mf

#endif // INCLUDED_mainframe_frame_iterator_h
