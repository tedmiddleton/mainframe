//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_frame_row_h
#define INCLUDED_mainframe_frame_row_h

#include <iterator>

#include "mainframe/series.hpp"
#include "mainframe/columnindex.hpp"
#include "mainframe/row_decl.hpp"

namespace mf
{
// IsConstDummy is just for template-template params with _row_proxy
template<bool IsConstDummy, typename... Ts>
class _base_frame_row
{
public:
    using row_type = std::true_type;

    _base_frame_row() = default;

    explicit _base_frame_row(const std::tuple<Ts...>& args)
        : data(args)
    {}

    explicit _base_frame_row(std::tuple<Ts...>&& args)
        : data(std::move(args))
    {}

    _base_frame_row(const _base_frame_row& other)
        : data(other.data)
    {}

    _base_frame_row(_base_frame_row&& other)
        : data(std::move(other.data))
    {}

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

    template<bool IsConst>
    void
    replace_missing(const _row_proxy<IsConst, Ts...>& refs)
    {
        replace_missing_impl<0>(refs);
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


    template<size_t Ind, bool IsConst>
    void
    replace_missing_impl(const _row_proxy<IsConst, Ts...>& refs)
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        if constexpr (detail::is_missing<T>::value) {
            columnindex<Ind> ci;
            T& t = at(ci);
            const T& rt = refs.at(ci);
            if (!t.has_value() && rt.has_value()) {
                t = rt;
            }
        }
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            replace_missing_impl<Ind+1>(refs);
        }
    }

    std::tuple<Ts...> data;
};

template<size_t Ind = 0, typename... Ts>
std::ostream&
operator<<(std::ostream& o, const frame_row<Ts...>& fr)
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

} // namespace mf

namespace std
{
template<bool IsConst, typename... Ts>
struct hash<mf::_base_frame_row<IsConst, Ts...>>
{
    template<size_t Ind = 0>
    size_t
    operator()(const mf::_base_frame_row<IsConst, Ts...>& fr) const
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

#endif // INCLUDED_mainframe_frame_row_h
