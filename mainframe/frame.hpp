//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_frame_h
#define INCLUDED_mainframe_frame_h

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

#include "mainframe/base.hpp"
#include "mainframe/expression.hpp"
#include "mainframe/frame_iterator.hpp"
#include "mainframe/missing.hpp"
#include "mainframe/series.hpp"
#include "mainframe/simd.hpp"

namespace mf
{

template<typename... Ts>
class frame;

class uframe
{
public:
    uframe()                         = default;
    uframe(const uframe&)            = default;
    uframe(uframe&&)                 = default;
    uframe& operator=(const uframe&) = default;
    uframe& operator=(uframe&&)      = default;

    template<typename... Ts>
    uframe(const frame<Ts...>& f)
    {
        init_impl<0>(f);
    }

    void
    add_series(const useries& s)
    {
        m_columns.push_back(s);
    }

    template<typename T>
    void
    add_series(const std::string& colname)
    {
        series<T> s;
        s.set_name(colname);
        s.resize(size());
        m_columns.push_back(s);
    }

    void
    clear()
    {
        for (auto& s : m_columns) {
            s.clear();
        }
    }

    useries&
    column(size_t i)
    {
        return m_columns.at(i);
    }

    const useries&
    column(size_t i) const
    {
        return m_columns.at(i);
    }

    size_t
    num_columns() const
    {
        return m_columns.size();
    }

    template<typename... Ts>
    operator frame<Ts...>() const
    {
        frame<Ts...> out;
        out.populate(m_columns);
        return out;
    }

    void
    set_series(size_t idx, const useries& s)
    {
        m_columns.at(idx) = s;
    }

    size_t
    size() const
    {
        if (m_columns.size() == 0) {
            return 0;
        }
        size_t sz = m_columns[0].size();
        for (auto& s : m_columns) {
            if (sz != s.size()) {
                throw std::logic_error{ "Inconsistent sizes!" };
            }
        }
        return sz;
    }

private:
    template<size_t Ind, typename... Ts>
    void
    init_impl(const frame<Ts...>& f)
    {
        useries s = f.template column<Ind>();
        m_columns.push_back(s);
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            init_impl<Ind + 1>(f);
        }
    }

    std::vector<useries> m_columns;
};

namespace detail
{

template<typename T, typename U>
struct join_frames;

template<typename... Ts, typename... Us>
struct join_frames<frame<Ts...>, frame<Us...>>
{
    using type = frame<Ts..., Us...>;
};

template<size_t Ind, typename Frame>
struct frame_element;

template<size_t Ind, typename... Ts>
struct frame_element<Ind, frame<Ts...>> : pack_element<Ind, Ts...>
{};

template<typename T, size_t Curr, size_t... IndList>
struct add_opt;

template<typename T, typename... Ts, size_t Curr, size_t... IndList>
struct add_opt<frame<mi<T>, Ts...>, Curr, IndList...>
{
    using frame_type   = frame<mi<T>>;
    using add_opt_type = typename add_opt<frame<Ts...>, Curr + 1, IndList...>::type;
    using type         = typename join_frames<frame_type, add_opt_type>::type;
};

template<typename T, typename... Ts, size_t Curr, size_t... IndList>
struct add_opt<frame<T, Ts...>, Curr, IndList...>
{
    static const bool inds_contains = contains<Curr, IndList...>::value;
    using frame_type   = typename std::conditional<inds_contains, frame<mi<T>>, frame<T>>::type;
    using add_opt_type = typename add_opt<frame<Ts...>, Curr + 1, IndList...>::type;
    using type         = typename join_frames<frame_type, add_opt_type>::type;
};

template<typename T, size_t Curr, size_t... IndList>
struct add_opt<frame<mi<T>>, Curr, IndList...>
{
    using type = frame<mi<T>>;
};

template<typename T, size_t Curr, size_t... IndList>
struct add_opt<frame<T>, Curr, IndList...>
{
    static const bool inds_contains = contains<Curr, IndList...>::value;
    using type = typename std::conditional<inds_contains, frame<mi<T>>, frame<T>>::type;
};

template<typename T, size_t Curr, size_t... IndList>
struct remove_opt;

template<typename T, typename... Ts, size_t Curr, size_t... IndList>
struct remove_opt<frame<mi<T>, Ts...>, Curr, IndList...>
{
    static const bool inds_contains = contains<Curr, IndList...>::value;
    using frame_type      = typename std::conditional<inds_contains, frame<T>, frame<mi<T>>>::type;
    using remove_opt_type = typename remove_opt<frame<Ts...>, Curr + 1, IndList...>::type;
    using type            = typename join_frames<frame_type, remove_opt_type>::type;
};

template<typename T, typename... Ts, size_t Curr, size_t... IndList>
struct remove_opt<frame<T, Ts...>, Curr, IndList...>
{
    using frame_type      = frame<T>;
    using remove_opt_type = typename remove_opt<frame<Ts...>, Curr + 1, IndList...>::type;
    using type            = typename join_frames<frame_type, remove_opt_type>::type;
};

template<typename T, size_t Curr, size_t... IndList>
struct remove_opt<frame<mi<T>>, Curr, IndList...>
{
    static const bool inds_contains = contains<Curr, IndList...>::value;
    using type = typename std::conditional<inds_contains, frame<T>, frame<mi<T>>>::type;
};

template<typename T, size_t Curr, size_t... IndList>
struct remove_opt<frame<T>, Curr, IndList...>
{
    using type = frame<T>;
};

template<typename T>
struct add_all_opt;

template<typename T, typename... Ts>
struct add_all_opt<frame<mi<T>, Ts...>>
{
    using remaining_frame = typename add_all_opt<frame<Ts...>>::type;
    using type            = typename prepend<mi<T>, remaining_frame>::type;
};

template<typename T, typename... Ts>
struct add_all_opt<frame<T, Ts...>>
{
    using remaining_frame = typename add_all_opt<frame<Ts...>>::type;
    using type            = typename prepend<mi<T>, remaining_frame>::type;
};

template<typename T>
struct add_all_opt<frame<mi<T>>>
{
    using type = frame<mi<T>>;
};

template<typename T>
struct add_all_opt<frame<T>>
{
    using type = frame<mi<T>>;
};

template<>
struct add_all_opt<frame<>>
{
    using type = frame<>;
};

template<typename T>
struct remove_all_opt;

template<typename T, typename... Ts>
struct remove_all_opt<frame<T, Ts...>>
{
    using remaining_frame = typename remove_all_opt<frame<Ts...>>::type;
    using type            = typename prepend<T, remaining_frame>::type;
};

template<typename T, typename... Ts>
struct remove_all_opt<frame<mi<T>, Ts...>>
{
    using remaining_frame = typename remove_all_opt<frame<Ts...>>::type;
    using type            = typename prepend<T, remaining_frame>::type;
};

template<typename T>
struct remove_all_opt<frame<mi<T>>>
{
    using type = frame<T>;
};

template<typename T>
struct remove_all_opt<frame<T>>
{
    using type = frame<T>;
};

template<>
struct remove_all_opt<frame<>>
{
    using type = frame<>;
};

} // namespace detail

template<size_t... Inds>
struct index_defn;

template<typename IndexDefn, typename... Ts>
class grouped_frame;

///
/// dataframe class
///
/// frame is mainframe's dataframe class, a column-oriented table data structure 
/// with columns of different types. In a database this could be thought of as 
/// the table schema, but with column-oriented storage we can think of this more 
/// naturally as a collection of arrays. A frame must be declared with it's 
/// column types as in 
///
/// `frame<int, double, std::string> f;`
///
/// @ref frame is mostly a column/@ref series (the two terms are interchangeable 
/// in the mainframe library) container. The @ref frame `f` in the above declaration 
/// contains 3 @ref series objects. @ref series objects themselves point to 
/// reference-counted copy-on-write arrays which means a @ref frame, as a @ref series 
/// container, also points to reference-counted copy-on-write arrays and as such 
/// can be copied and moved cheaply.
///
/// Any mutating operation on a @ref frame - requesting a non-const iterator or 
/// calling push_back() or erase() for example - will trigger an "unref" where 
/// the underlying array is copied if it's ref-count is greater than 1. 
/// Depending on the size of the data set, this could be expensive. 
///
template<typename... Ts>
class frame
{
    template<size_t Ind, typename... Us>
    using pack_element = detail::pack_element<Ind, Us...>;

public:
    using iterator               = frame_iterator<Ts...>;
    using const_iterator         = const_frame_iterator<Ts...>;
    using reverse_iterator       = reverse_frame_iterator<Ts...>;
    using const_reverse_iterator = const_reverse_frame_iterator<Ts...>;
    using name_array             = std::array<std::string, sizeof...(Ts)>;
    using row_type               = frame_row<Ts...>;
    using value_type             = _row_proxy<false, Ts...>;
    using const_value_type       = _row_proxy<true, Ts...>;

    frame()                        = default;
    frame(const frame&)            = default;
    frame(frame&&)                 = default;
    frame& operator=(const frame&) = default;
    frame& operator=(frame&&)      = default;

    /// Append one frame to this one and return a new frame non-destructively. 
    /// This will necessarily trigger an "unref" (copy) of this @ref frame's 
    /// data.
    ///
    ///     frame_row<year_month, int, double> fr1; 
    ///     frame_row<year_month, int, double> fr2; 
    ///     fr1.push_back(2022_y/11, 1, 3.14);
    ///     fr2.push_back(2022_y/12, 2, 0.007297);
    ///
    ///     auto fr3 = fr1 + fr2;
    ///
    frame<Ts...>
    operator+(const frame<Ts...>& other) const
    {
        frame<Ts...> out{ *this };
        out.insert(out.end(), other.cbegin(), other.cend());
        return out;
    }

    /// Requests a row iterator pointing to the first row of the frame. Note 
    /// that this is a nonconst-iterator which means that any underlying series 
    /// with a reference count greater than 1 will be copied first. 
    ///
    iterator
    begin()
    {
        unref();
        return iterator{ m_columns, 0 };
    }
    /// Requests a row iterator pointing just past the last row of the frame. Note 
    /// that this is a nonconst-iterator which means that any underlying series 
    /// with a reference count greater than 1 will be copied first. 
    ///
    iterator
    end()
    {
        unref();
        return iterator{ m_columns, static_cast<int>(size()) };
    }
    /// Requests a row iterator pointing to the first row of the frame.  
    ///
    const_iterator
    begin() const
    {
        return const_iterator{ m_columns, 0 };
    }
    /// Requests a row iterator pointing just past the last row of the frame.  
    ///
    const_iterator
    end() const
    {
        return const_iterator{ m_columns, static_cast<int>(size()) };
    }
    /// Requests a row iterator pointing to the first row of the frame.  
    ///
    const_iterator
    cbegin() const
    {
        return const_iterator{ m_columns, 0 };
    }
    /// Requests a row iterator pointing just past the last row of the frame.  
    ///
    const_iterator
    cend() const
    {
        return const_iterator{ m_columns, static_cast<int>(size()) };
    }
    /// Requests a reverse (will increment from the last row to the first) row 
    /// iterator that points to the last row. This operation will unref the 
    /// dataframe.
    ///
    reverse_iterator 
    rbegin()
    {
        unref();
        return reverse_iterator{ m_columns, static_cast<int>(size()) - 1 };
    }
    /// Requests a reverse (will increment from the last row to the first) row 
    /// iterator that points to the place in memory just before the first row. 
    /// This operation will unref the dataframe.
    ///
    reverse_iterator
    rend()
    {
        unref();
        return reverse_iterator{ m_columns, -1 };
    }
    /// Requests a reverse (will increment from the last row to the first) const 
    /// row iterator that points to the last row. 
    ///
    const_reverse_iterator
    rbegin() const
    {
        unref();
        return const_reverse_iterator{ m_columns, static_cast<int>(size()) - 1 };
    }
    /// Requests a reverse (will increment from the last row to the first) row 
    /// iterator that points to the place in memory just before the first row. 
    /// This operation will unref the dataframe.
    ///
    const_reverse_iterator
    rend() const
    {
        unref();
        return const_reverse_iterator{ m_columns, -1 };
    }
    /// Requests a reverse (will increment from the last row to the first) const 
    /// row iterator that points to the last row. 
    ///
    const_reverse_iterator
    crbegin() const
    {
        return const_reverse_iterator{ m_columns, static_cast<int>(size()) - 1 };
    }
    /// Requests a reverse (will increment from the last row to the first) row 
    /// iterator that points to the place in memory just before the first row. 
    ///
    const_reverse_iterator
    crend() const
    {
        return const_reverse_iterator{ m_columns, -1 };
    }

    /// convert a column to use the missing class mi<> to represent missing 
    /// elements
    ///
    ///     frame_row<year_month, int, double> fr1; 
    ///
    ///     // fr2 is frame_row<year_month, int, mi<double>>
    ///     auto fr2 = fr1.allow_missing(_2);
    ///     fr1.push_back(2022_y/10, 1, 3.14);
    ///     fr2.push_back(2022_y/11, 2, missing);
    ///     fr2.push_back(2022_y/12, 3, 0.007297);
    ///
    template<size_t... Inds>
    typename detail::add_opt<frame<Ts...>, 0, Inds...>::type
    allow_missing(columnindex<Inds>... cols) const
    {
        uframe u(*this);
        allow_missing_impl<0, Inds...>(u, cols...);
        return u;
    }

    /// convert all columns to use the missing class mi<> to represent missing 
    /// elements
    ///
    typename detail::add_all_opt<frame<Ts...>>::type
    allow_missing() const
    {
        uframe u;
        allow_missing_impl<0, Ts...>(u);
        return u;
    }

    /// Remove all rows/data from the dataframe
    ///
    void
    clear()
    {
        clear_impl<0>();
    }

    /// Return the @ref series with colname
    ///
    useries
    column(const std::string& colname) const
    {
        return column_impl<0, Ts...>(colname);
    }

    template<size_t Ind>
    series<typename detail::pack_element<Ind, Ts...>::type>&
    column()
    {
        return std::get<Ind>(m_columns);
    }

    template<size_t Ind>
    const series<typename detail::pack_element<Ind, Ts...>::type>&
    column() const
    {
        return std::get<Ind>(m_columns);
    }

    template<size_t Ind>
    series<typename detail::pack_element<Ind, Ts...>::type>&
    column(columnindex<Ind>)
    {
        return std::get<Ind>(m_columns);
    }

    template<size_t Ind>
    const series<typename detail::pack_element<Ind, Ts...>::type>&
    column(columnindex<Ind>) const
    {
        return std::get<Ind>(m_columns);
    }

    template<size_t Ind>
    std::string
    column_name() const
    {
        return std::get<Ind>(m_columns).name();
    }

    template<size_t Ind>
    std::string
    column_name(columnindex<Ind>) const
    {
        return std::get<Ind>(m_columns).name();
    }

    name_array
    column_names() const
    {
        std::array<std::string, sizeof...(Ts)> out;
        column_names_impl<0>(out);
        return out;
    }

    template<size_t... Inds>
    typename detail::rearrange<frame<Ts...>, Inds...>::type
    columns(columnindex<Inds>... cols) const
    {
        uframe f;
        columns_impl(f, cols...);
        return f;
    }

    template<typename... Us>
    uframe
    columns(const Us&... us) const
    {
        uframe f;
        columns_impl(f, us...);
        return f;
    }

    template<size_t Ind1, size_t Ind2>
    double
    corr(terminal<expr_column<Ind1>>, terminal<expr_column<Ind2>>) const
    {
        using T1             = typename detail::pack_element<Ind1, Ts...>::type;
        using T2             = typename detail::pack_element<Ind2, Ts...>::type;
        const series<T1>& s1 = std::get<Ind1>(m_columns);
        const series<T2>& s2 = std::get<Ind2>(m_columns);

        double c = detail::correlate_pearson(s1.data(), s2.data(), s1.size());
        return c;
    }

    /// convert a column to NOT use the missing class mi<> to represent missing 
    /// elements
    ///
    ///     frame_row<year_month, int, mi<double>> fr1; 
    ///
    ///     // fr2 is frame_row<year_month, int, double>
    ///     auto fr2 = fr1.disallow_missing(_2);
    ///     fr1.push_back(2022_y/10, 1, 3.14);
    ///     //fr2.push_back(2022_y/11, 2, missing); this won't compile
    ///     fr2.push_back(2022_y/12, 3, 0.007297);
    ///
    template<size_t... Inds>
    typename detail::remove_opt<frame<Ts...>, 0, Inds...>::type
    disallow_missing(columnindex<Inds>... cols) const
    {
        uframe u(*this);
        disallow_missing_impl<0, Inds...>(u, cols...);
        return u;
    }

    /// convert all columns to NOT use the missing class mi<> to represent 
    /// missing elements
    ///
    typename detail::remove_all_opt<frame<Ts...>>::type
    disallow_missing() const
    {
        uframe u;
        disallow_missing_impl<0, Ts...>(u);
        return u;
    }

    frame<Ts...>
    drop_missing() const
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

    bool
    empty() const
    {
        return std::get<0>(m_columns).empty();
    }

    iterator
    erase(iterator first, iterator last)
    {
        std::tuple<Ts*...> ptrs;
        erase_impl<0>(ptrs, first, last);
        return iterator{ ptrs };
    }

    iterator
    erase(iterator pos)
    {
        std::tuple<Ts*...> ptrs;
        erase_impl<0>(ptrs, pos);
        return iterator{ ptrs };
    }

    template<size_t... Idx>
    grouped_frame<index_defn<Idx...>, Ts...>
    groupby(columnindex<Idx>...) const
    {
        return grouped_frame<index_defn<Idx...>, Ts...>{ *this };
    }

    iterator
    insert(iterator pos, const_iterator first, const_iterator last)
    {
        std::tuple<Ts*...> ptrs;
        insert_impl<0>(ptrs, pos, first, last);
        return iterator{ ptrs };
    }

    iterator
    insert(iterator pos, const Ts&... ts)
    {
        std::tuple<Ts*...> ptrs;
        insert_impl<0>(ptrs, pos, 1, ts...);
        return iterator{ ptrs };
    }

    iterator
    insert(iterator pos, size_t count, const Ts&... ts)
    {
        std::tuple<Ts*...> ptrs;
        insert_impl<0>(ptrs, pos, count, ts...);
        return iterator{ ptrs };
    }

    template<size_t Ind>
    double
    mean(columnindex<Ind>) const
    {
        using T            = typename detail::pack_element<Ind, Ts...>::type;
        const series<T>& s = std::get<Ind>(m_columns);
        double m           = detail::mean(s.data(), s.size());
        return m;
    }

    template<typename T>
    frame<Ts..., T>
    new_series(const std::string& series_name) const
    {
        uframe plust(*this);
        series<T> ns(size());
        ns.set_name(series_name);
        useries us(ns);
        plust.add_series(us);
        return plust;
    }

    template<typename T, typename Ex>
    frame<Ts..., T>
    new_series(const std::string& series_name, Ex expr) const
    {
        uframe plust(*this);
        series<T> ns(size());
        ns.set_name(series_name);
        useries us(ns);
        plust.add_series(us);
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
                auto val  = expr(b, it, e);
                auto uval = detail::unwrap_missing<decltype(val)>::unwrap(val);
                it->template at<sizeof...(Ts)>() = uval;
            }
        }
        return out;
    }

    size_t
    num_columns() const
    {
        return sizeof...(Ts);
    }

    bool
    operator==(const frame<Ts...>& other) const
    {
        return eq_impl<0>(other);
    }

    _row_proxy<false, Ts...>
    operator[](size_t ind)
    {
        _row_proxy<false, Ts...> out{ m_columns, static_cast<ptrdiff_t>(ind) };
        return out;
    }

    _row_proxy<true, Ts...>
    operator[](size_t ind) const
    {
        _row_proxy<true, Ts...> out{ m_columns, static_cast<ptrdiff_t>(ind) };
        return out;
    }

    void
    pop_back()
    {
        pop_back_impl<0>();
    }

    ///
    /// Append a row from another frame to the end of this frame.
    ///
    ///     void do_something( const frame_row<year_month, int, double>& fr_other )
    ///     {
    ///       frame_row<year_month, int, double> fr; 
    ///       fr.push_back( fr_other[0] );
    ///
    template<bool IsConst, typename... Us, typename... Vs>
    void
    push_back(const _row_proxy<IsConst, Us...>& fr, const Vs&... args)
    {
        push_back_multiple_row_impl<0>(fr, args...);
    }
    ///
    /// Append an explicitly declared frame_row to the end of the frame be 
    /// an explicitly 
    /// 
    ///     frame_row<year_month, int, double> fr; 
    ///     fr.at(_0) = 2022_y/12; fr.at(_1) = 1; fr.at(_2) = 3.14;
    ///
    ///     frame<year_month, int, double> f;
    ///     f.push_back(fr);
    ///
    template<typename... Us, typename... Vs>
    void
    push_back(const frame_row<Us...>& fr, const Vs&... args)
    {
        push_back_multiple_row_impl<0>(fr, args...);
    }
    ///
    /// Append a flexible list of objects to the end of this frame as a new row
    /// 
    ///     frame_row<year_month, int, double> fr; 
    ///     fr.push_back(2022_y/12, 1, 3.14);
    ///
    template<typename U, typename... Us>
    void
    push_back(U first_arg, Us... args)
    {
        push_back_impl<0, U, Us...>(first_arg, args...);
    }

    void
    reserve(size_t newsize)
    {
        reserve_impl<0>(newsize);
    }

    void
    resize(size_t newsize)
    {
        resize_impl<0>(newsize);
    }

    template<typename Ex>
    frame<Ts...>
    rows(Ex ex) const
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

    void
    set_column_names(const std::vector<std::string>& names)
    {
        set_column_names_impl<0>(names);
    }

    void
    set_column_names(const name_array& names)
    {
        set_column_names_impl<0>(names);
    }

    template<typename... Us>
    void
    set_column_names(const Us&... colnames)
    {
        set_column_names_impl<0, Us...>(colnames...);
    }

    size_t
    size() const
    {
        return size_impl_with_check<0, Ts...>();
    }

    std::vector<std::vector<std::string>>
    to_string() const
    {
        std::vector<std::vector<std::string>> out;
        to_string_impl<0, Ts...>(out);
        return out;
    }

private:
    template<size_t Ind, size_t... Inds>
    void
    allow_missing_impl(uframe& uf, columnindex<Inds>... cols) const
    {
        if constexpr (detail::contains<Ind, Inds...>::value) {
            auto& s  = column<Ind>();
            auto ams = s.allow_missing();
            uf.set_series(Ind, ams);
        }
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            allow_missing_impl<Ind + 1, Inds...>(uf, cols...);
        }
    }

    template<size_t Ind, typename U, typename... Us>
    void
    allow_missing_impl(uframe& uf) const
    {
        const series<U>& s = std::get<Ind>(m_columns);
        auto os            = s.allow_missing();
        uf.add_series(os);
        if constexpr (sizeof...(Us) > 0) {
            allow_missing_impl<Ind + 1, Us...>(uf);
        }
    }

    template<size_t Ind>
    void
    clear_impl()
    {
        std::get<Ind>(m_columns).clear();
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            clear_impl<Ind + 1>();
        }
    }

    template<size_t Ind, typename U, typename... Us>
    useries
    column_impl(const std::string& colname) const
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

    template<size_t Ind>
    void
    column_names_impl(std::array<std::string, sizeof...(Ts)>& out) const
    {
        auto& s  = std::get<Ind>(m_columns);
        out[Ind] = s.name();
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            column_names_impl<Ind + 1>(out);
        }
    }

    template<typename U, typename... Us>
    void
    columns_impl(uframe& f, const U& u, const Us&... us) const
    {
        useries s = column(u);
        f.add_series(s);
        if constexpr (sizeof...(Us) > 0) {
            columns_impl(f, us...);
        }
    }

    template<size_t Ind, size_t... Inds>
    void
    disallow_missing_impl(uframe& uf, columnindex<Inds>... cols) const
    {
        if constexpr (detail::contains<Ind, Inds...>::value) {
            auto& s  = column<Ind>();
            auto ams = s.disallow_missing();
            uf.set_series(Ind, ams);
        }
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            disallow_missing_impl<Ind + 1, Inds...>(uf, cols...);
        }
    }

    template<size_t Ind, typename U, typename... Us>
    void
    disallow_missing_impl(uframe& uf) const
    {
        const series<U>& s = std::get<Ind>(m_columns);
        auto os            = s.disallow_missing();
        uf.add_series(os);
        if constexpr (sizeof...(Us) > 0) {
            disallow_missing_impl<Ind + 1, Us...>(uf);
        }
    }

    template<size_t Ind>
    void
    erase_impl(std::tuple<Ts*...>& ptrs, iterator pos)
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

    template<size_t Ind>
    void
    erase_impl(std::tuple<Ts*...>& ptrs, iterator first, iterator last)
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

    template<size_t Ind>
    bool
    eq_impl(const frame<Ts...>& other) const
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

    template<size_t Ind, typename U, typename... Us>
    void
    insert_impl(std::tuple<Ts*...>& ptrs, iterator pos, size_t count, const U& u, const Us&... us)
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

    template<size_t Ind>
    void
    insert_impl(std::tuple<Ts*...>& ptrs, iterator pos, const_iterator first, const_iterator last)
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

    template<size_t Ind>
    void
    pop_back_impl()
    {
        auto& s = std::get<Ind>(m_columns);
        s.pop_back();
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            pop_back_impl<Ind + 1>();
        }
    }

    void
    populate(const std::vector<useries>& columns)
    {
        populate_impl<0, Ts...>(columns);
    }

    template<size_t Ind, typename U, typename... Us>
    void
    populate_impl(const std::vector<useries>& columns)
    {
        const useries& su = columns.at(Ind);
        series<U>& s      = std::get<Ind>(m_columns);
        s                 = series<U>(su);
        if constexpr (sizeof...(Us) > 0) {
            populate_impl<Ind + 1, Us...>(columns);
        }
    }

    template<size_t Ind>
    void
    push_back_multiple_empty_impl()
    {
        using T = typename pack_element<Ind, Ts...>::type;
        // Default ctor
        std::get<Ind>(m_columns).push_back(T{});
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            push_back_multiple_empty_impl<Ind + 1>();
        }
    }

    template<size_t Ind, typename V, typename... Vs>
    void
    push_back_multiple_args_impl(V arg, const Vs&... args)
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

    template<size_t Ind, bool IsConst, template<bool, typename...> typename Row, typename... Us,
        typename... Vs>
    void
    push_back_multiple_row_impl(const Row<IsConst, Us...>& fr, const Vs&... args)
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

    template<size_t Ind, typename U, typename... Us>
    void
    push_back_impl(const U& u, const Us&... us)
    {
        using T = typename detail::pack_element<Ind, Ts...>::type;
        std::get<Ind>(m_columns).push_back(static_cast<T>(u));
        if constexpr (sizeof...(Us) > 0) {
            push_back_impl<Ind + 1>(us...);
        }
    }

    template<size_t Ind>
    void
    reserve_impl(size_t newsize)
    {
        auto& s = std::get<Ind>(m_columns);
        s.reserve(newsize);
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            reserve_impl<Ind + 1>(newsize);
        }
    }

    template<size_t Ind>
    void
    resize_impl(size_t newsize)
    {
        auto& s = std::get<Ind>(m_columns);
        s.resize(newsize);
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            resize_impl<Ind + 1>(newsize);
        }
    }

    template<size_t Ind>
    void
    set_column_names_impl(const std::vector<std::string>& names)
    {
        if (Ind < names.size()) {
            auto& s = std::get<Ind>(m_columns);
            s.set_name(names.at(Ind));
        }
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            set_column_names_impl<Ind + 1>(names);
        }
    }

    template<size_t Ind>
    void
    set_column_names_impl(const std::array<std::string, sizeof...(Ts)>& names)
    {
        auto& s = std::get<Ind>(m_columns);
        s.set_name(names[Ind]);
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            set_column_names_impl<Ind + 1>(names);
        }
    }

    template<size_t Ind, typename U, typename... Us>
    void
    set_column_names_impl(const U& colname, const Us&... colnames)
    {
        auto& s = std::get<Ind>(m_columns);
        s.set_name(colname);
        if constexpr (sizeof...(Us) > 0) {
            set_column_names_impl<Ind + 1, Us...>(colnames...);
        }
    }

    template<size_t Ind, typename U, typename... Us>
    size_t
    size_impl_with_check() const
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

    template<size_t Ind, typename U, typename... Us>
    void
    to_string_impl(std::vector<std::vector<std::string>>& strs) const
    {
        const series<U>& s               = std::get<Ind>(m_columns);
        std::vector<std::string> colstrs = s.to_string();
        strs.push_back(colstrs);
        if constexpr (sizeof...(Us) > 0) {
            to_string_impl<Ind + 1, Us...>(strs);
        }
    }

    template<size_t Ind = 0>
    void
    unref()
    {
        using T      = typename detail::pack_element<Ind, Ts...>::type;
        series<T>& s = std::get<Ind>(m_columns);
        s.unref();
        if constexpr (Ind + 1 < sizeof...(Ts)) {
            unref<Ind + 1>();
        }
    }

    template<typename... Us>
    friend std::ostream& operator<<(std::ostream&, const frame<Us...>&);
    friend class uframe;

    std::tuple<series<Ts>...> m_columns;
};

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
