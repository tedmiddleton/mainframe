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

#include "mainframe/detail/base.hpp"
#include "mainframe/detail/frame.hpp"
#include "mainframe/detail/simd.hpp"
#include "mainframe/detail/uframe.hpp"
#include "mainframe/expression.hpp"
#include "mainframe/frame_iterator.hpp"
#include "mainframe/missing.hpp"
#include "mainframe/series.hpp"

namespace mf
{

template<size_t... Inds>
struct index_defn;

template<typename IndexDefn, typename... Ts>
class group;

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
    using pack_element = typename detail::pack_element<Ind, Us...>;

public:
    using iterator               = frame_iterator<Ts...>;
    using const_iterator         = const_frame_iterator<Ts...>;
    using reverse_iterator       = reverse_frame_iterator<Ts...>;
    using const_reverse_iterator = const_reverse_frame_iterator<Ts...>;
    using name_array             = std::array<std::string, sizeof...(Ts)>;
    using row_type               = frame_row<Ts...>;
    using value_type             = _row_proxy<false, Ts...>;
    using const_value_type       = _row_proxy<true, Ts...>;

    frame()             = default;
    frame(const frame&) = default;
    explicit frame(const series<typename detail::pack_element<0, Ts...>::type>&);
    frame(frame&&)      = default;
    frame&
    operator=(const frame&) = default;
    frame&
    operator=(frame&&) = default;

    /// Requests a row iterator pointing to the first row of the frame. Note
    /// that this is a nonconst-iterator which means that any underlying series
    /// with a reference count greater than 1 will be copied first.
    ///
    iterator
    begin();

    /// Requests a row iterator pointing just past the last row of the frame. Note
    /// that this is a nonconst-iterator which means that any underlying series
    /// with a reference count greater than 1 will be copied first.
    ///
    iterator
    end();

    /// Requests a row iterator pointing to the first row of the frame.
    ///
    const_iterator
    begin() const;

    /// Requests a row iterator pointing just past the last row of the frame.
    ///
    const_iterator
    end() const;

    /// Requests a row iterator pointing to the first row of the frame.
    ///
    const_iterator
    cbegin() const;

    /// Requests a row iterator pointing just past the last row of the frame.
    ///
    const_iterator
    cend() const;

    /// Requests a reverse (will increment from the last row to the first) row
    /// iterator that points to the last row. This operation will unref the
    /// dataframe.
    ///
    reverse_iterator
    rbegin();

    /// Requests a reverse (will increment from the last row to the first) row
    /// iterator that points to the place in memory just before the first row.
    /// This operation will unref the dataframe.
    ///
    reverse_iterator
    rend();

    /// Requests a reverse (will increment from the last row to the first) const
    /// row iterator that points to the last row.
    ///
    const_reverse_iterator
    rbegin() const;

    /// Requests a reverse (will increment from the last row to the first) row
    /// iterator that points to the place in memory just before the first row.
    /// This operation will unref the dataframe.
    ///
    const_reverse_iterator
    rend() const;

    /// Requests a reverse (will increment from the last row to the first) const
    /// row iterator that points to the last row.
    ///
    const_reverse_iterator
    crbegin() const;

    /// Requests a reverse (will increment from the last row to the first) row
    /// iterator that points to the place in memory just before the first row.
    ///
    const_reverse_iterator
    crend() const;

    template<size_t... Inds>
    using frame_with_missing_columns = typename detail::add_opt<frame<Ts...>, 0, Inds...>::type;
    
    /// convert a column to use the missing class mi<> to represent missing
    /// elements
    ///
    ///     frame<year_month, int, double> f1;
    ///
    ///     frame<mi<year_month>, int, mi<double>> f2 = f1.allow_missing(_0, _2);
    ///     f2.push_back(missing, 1, 3.14);
    ///     f2.push_back(2022_y/11, 2, missing);
    ///     f2.push_back(2022_y/12, 3, 0.007297);
    ///
    template<size_t... Inds>
    frame_with_missing_columns<Inds...>
    allow_missing(columnindex<Inds>... cols) const;

    using frame_with_all_missing_columns = typename detail::add_all_opt<frame<Ts...>>::type;

    /// convert all columns to use the missing class mi<> to represent missing
    /// elements
    ///
    ///     frame<year_month_day, double, bool> f1;
    ///
    ///     frame<mi<year_month_day>, mi<double>, mi<bool>> f2 = f1.allow_missing();
    ///     f2.push_back(2022_y / January / 1, 8.9, false);
    ///     f2.push_back(missing, 10.0, false);
    ///     f2.push_back(2022_y / January / 3, missing, true);
    ///     f2.push_back(2022_y / January / 4, 12.2, missing);
    ///     f2.push_back(missing, missing, false);
    ///
    frame_with_all_missing_columns
    allow_missing() const;

    /// Add a new column to the end of the frame with type T
    ///
    ///     frame<year_month, int> f1;
    ///     f1.set_column_names({"month", "length"});
    ///     f1.push_back(2022_y/1, 1);
    ///     f1.push_back(2022_y/2, 2);
    ///     f1.push_back(2022_y/3, 3);
    ///     frame<year_month, int, double> f2 = f1.append_column<double>("depth");
    ///     // f2 is now 
    ///     //   |   month | length | depth
    ///     // __|_________|________|_______
    ///     //  0| 2022-01 |      1 |   0.0
    ///     //  1| 2022-02 |      2 |   0.0
    ///     //  2| 2022-03 |      3 |   0.0
    ///
    template<typename T>
    frame<Ts..., T>
    append_column(const std::string& column_name) const;

    template<typename T, typename Ex>
    frame<Ts..., T>
    append_column(const std::string& column_name, Ex expr) const;

    /// Add an existing series to the frame as a column to the end of the frame 
    ///
    ///     frame<year_month, int> f1;
    ///     f1.set_column_names({"month", "length"});
    ///     f1.push_back(2022_y/1, 1);
    ///     f1.push_back(2022_y/2, 2);
    ///     f1.push_back(2022_y/3, 3);
    ///     series<double> s1 = {1.1, 2.2};
    ///     s1.set_name("depth");
    ///     series<double> s2 = {3.3, 4.4, 5.5, 6.6};
    ///     s2.set_name("height");
    ///     frame<year_month, int, double> f2 = f1.append_series(s1);
    ///     frame<year_month, int, double, double> f3 = f1.append_series(s2);
    ///     // f3 is now
    ///     //   |   month | length | depth | height
    ///     // __|_________|________|_______|________
    ///     //  0| 2022-01 |      1 |   1.1 |    3.3
    ///     //  1| 2022-02 |      2 |   2.2 |    4.4
    ///     //  2| 2022-03 |      3 |   0.0 |    5.5
    ///     //  3| 0000-00 |      0 |   0.0 |    6.6
    ///
    template<typename T>
    frame<Ts..., T>
    append_series(const series<T>& s) const;

    /// Remove all rows/data from the dataframe
    ///
    void
    clear();

    /// Return the @ref series with colname
    ///
    useries
    column(const std::string& colname) const;

    template<size_t Ind>
    series<typename detail::pack_element<Ind, Ts...>::type>& column(columnindex<Ind>);

    template<size_t Ind>
    const series<typename detail::pack_element<Ind, Ts...>::type>& column(columnindex<Ind>) const;

    template<size_t Ind>
    std::string
    column_name() const;

    template<size_t Ind>
    std::string column_name(columnindex<Ind>) const;

    name_array
    column_names() const;

    template<size_t... Inds>
    typename detail::rearrange<frame<Ts...>, Inds...>::type
    columns(columnindex<Inds>... cols) const;

    template<typename... Us>
    uframe
    columns(const Us&... us) const;

    template<size_t Ind1, size_t Ind2>
    double corr(terminal<expr_column<Ind1>>, terminal<expr_column<Ind2>>) const;

    template<size_t... Inds>
    using frame_without_missing_columns = typename detail::remove_opt<frame<Ts...>, 0, Inds...>::type;

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
    frame_without_missing_columns<Inds...>
    disallow_missing(columnindex<Inds>... cols) const;

    using frame_without_any_missing_columns = typename detail::remove_all_opt<frame<Ts...>>::type;

    /// convert all columns to NOT use the missing class mi<> to represent
    /// missing elements
    ///
    ///     frame_row<mi<year_month>, int, mi<double>> fr1;
    ///
    ///     // fr2 is frame_row<year_month, int, double>
    ///     auto fr2 = fr1.disallow_missing();
    ///     fr1.push_back(2022_y/10, 1, 3.14);
    ///     //fr2.push_back(missing, 2, missing); this won't compile
    ///     fr2.push_back(2022_y/12, 3, 0.007297);
    ///
    frame_without_any_missing_columns
    disallow_missing() const;

    frame<Ts...>
    drop_missing() const;

    bool
    empty() const;

    iterator
    erase(iterator first, iterator last);

    iterator
    erase(iterator pos);

    frame<Ts...>
    fill_forward() const;

    frame<Ts...>
    fill_backward() const;

    template<size_t... Idx>
    group<index_defn<Idx...>, Ts...>
    groupby(columnindex<Idx>...) const;

    template<typename... Us>
    frame<Ts..., Us...>
    hcat(frame<Us...> other) const;

    iterator
    insert(iterator pos, const_iterator first, const_iterator last);

    iterator
    insert(iterator pos, const Ts&... ts);

    iterator
    insert(iterator pos, size_t count, const Ts&... ts);

    template<size_t Ind>
    double mean(columnindex<Ind>) const;

    template<size_t Ind>
    using pack_elem_pair =
        std::pair<typename pack_element<Ind, Ts...>::type, typename pack_element<Ind, Ts...>::type>;

    /// Calculate the minimum and maximum value in a column, and return them in
    /// a std::pair<>
    ///
    /// If the frame is empty and the type of the column doesn't support default
    /// construction, this will throw std::out_of_range
    ///
    template<size_t Ind>
    pack_elem_pair<Ind> minmax(columnindex<Ind>) const;

    size_t
    num_columns() const;

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
    operator+(const frame<Ts...>& other) const;

    bool
    operator==(const frame<Ts...>& other) const;

    /// Cast to series<> if the frame has only one column
    ///
    /// This is a convenience operator for the common case of a single-column
    /// frame. It allows you to write
    ///
    ///    void takes_a_series(const series<double>& s);
    ///
    ///    frame_row<year_month, int, double> fr1;
    ///    fr1.push_back(2022_y/11, 1, 3.14);
    ///    takes_a_series(fr1[_2]);
    ///
    template<size_t NumColumns = sizeof...(Ts)>
    operator std::enable_if_t<NumColumns == 1, series<typename pack_element<0, Ts...>::type>>() const;

    /// Dereference the first row of a single-column frame
    ///
    /// This is a convenience operator for the common case of a single-column
    /// frame. It allows you to write
    ///
    ///     frame_row<year_month, int, double> fr1;
    ///     fr1.push_back(2022_y/11, 1, 3.14);
    ///     year_month x = *(fr1[_0]);
    ///
    template<size_t Num=sizeof...(Ts)>
    typename std::enable_if<Num == 1, typename detail::pack_element<0, Ts...>::type>::type
    operator*() const;

    // row-selector operator[]. One tricky bit is that rows(Ex) which it calls
    // only requires is_expression<Ex> whereas here we require
    // is_complex_expression<Ex>. This means that if we have a column (say
    // column _1) that is bool-convertible, we can call
    //   fr.rows(_1);
    // ...and that will select all rows where column _1 is 'true' but we CANNOT
    // call
    //   fr[_1];
    // ...because this is a column-selector call that returns a frame with a
    // single column. Instead we either need to call rows() or call
    //   fr[_1 == true];
    //
    template<typename Ex>
    std::enable_if_t<is_complex_expression<Ex>::value, frame<Ts...>>
    operator[](Ex ex) const;

    frame<Ts...>
    operator[](size_t ind) const;

#if __cplusplus <= 202002L
    // Note that this will only work up to c++23, where we can be more sensible
    // about multiple arguments to operator[]
    template<size_t... Inds>
    typename detail::rearrange<frame<Ts...>, Inds...>::type
    operator[](columnindexpack<Inds...> cols) const;

    template<size_t Ind>
    typename detail::rearrange<frame<Ts...>, Ind>::type operator[](columnindex<Ind>) const;

#else

    template<size_t... Inds>
    typename detail::rearrange<frame<Ts...>, Inds...>::type
    operator[](columnindex<Inds>... cols) const;

#endif

    void
    pop_back();

    /// Add a new column to the beginning of the frame with type T
    ///
    ///     frame<year_month, int> f1;
    ///     f1.set_column_names({"month", "length"});
    ///     f1.push_back(2022_y/1, 1);
    ///     f1.push_back(2022_y/2, 2);
    ///     f1.push_back(2022_y/3, 3);
    ///     frame<double, year_month, int> f2 = f1.prepend_column<double>("depth");
    ///     // f2 is now 
    ///     //   | depth |   month | length 
    ///     // __|_______|_________|________
    ///     //  0|   0.0 | 2022-01 |      1 
    ///     //  1|   0.0 | 2022-02 |      2 
    ///     //  2|   0.0 | 2022-03 |      3 
    ///
    template<typename T>
    frame<T, Ts...>
    prepend_column(const std::string& column_name) const;

    template<typename T, typename Ex>
    frame<T, Ts...>
    prepend_column(const std::string& column_name, Ex expr) const;

    /// Add an existing series to the frame as a column to the beginning of the frame 
    ///
    ///     frame<year_month, int> f1;
    ///     f1.set_column_names({"month", "length"});
    ///     f1.push_back(2022_y/1, 1);
    ///     f1.push_back(2022_y/2, 2);
    ///     f1.push_back(2022_y/3, 3);
    ///     series<double> s1 = {1.1, 2.2};
    ///     s1.set_name("depth");
    ///     series<double> s2 = {3.3, 4.4, 5.5, 6.6};
    ///     s2.set_name("height");
    ///     frame<double, year_month, int> f2 = f1.prepend_series(s1);
    ///     frame<double, double, year_month, int> f3 = f1.prepend_series(s2);
    ///     // f3 is now
    ///     //   | height | depth |   month | length 
    ///     // __|________|_______|_________|________
    ///     //  0|    3.3 |   1.1 | 2022-01 |      1 
    ///     //  1|    4.4 |   2.2 | 2022-02 |      2 
    ///     //  2|    5.5 |   0.0 | 2022-03 |      3 
    ///     //  3|    6.6 |   0.0 | 0000-00 |      0 
    ///
    template<typename T>
    frame<T, Ts...>
    prepend_series(const series<T>& s) const;

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
    push_back(const _row_proxy<IsConst, Us...>& fr, const Vs&... args);

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
    push_back(const frame_row<Us...>& fr, const Vs&... args);

    ///
    /// Append a flexible list of objects to the end of this frame as a new row
    ///
    ///     frame_row<year_month, int, double> fr;
    ///     fr.push_back(2022_y/12, 1, 3.14);
    ///
    template<typename U, typename... Us>
    void
    push_back(U first_arg, Us... args);

    template<size_t Ind>
    using frame_without_indexed_column = typename detail::pack_remove<Ind, frame<Ts...>>::type;

    /// Return a frame without the column at Ind index
    template<size_t Ind>
    frame_without_indexed_column<Ind> remove_column(columnindex<Ind>);

    void
    reserve(size_t newsize);

    void
    resize(size_t newsize);

    frame<Ts...>
    reversed() const;

    template<size_t... Inds>
    void
    reverse_sort(columnindex<Inds>...);

    template<size_t... Inds>
    frame<Ts...>
    reverse_sorted(columnindex<Inds>... ci);

    _row_proxy<false, Ts...>
    row(size_t ind);

    _row_proxy<true, Ts...>
    row(size_t ind) const;

    template<typename Ex>
    std::enable_if_t<is_expression<Ex>::value, frame<Ts...>>
    rows(Ex ex) const;

    void
    set_column_names(const std::vector<std::string>& names);

    void
    set_column_names(const name_array& names);

    template<typename... Us>
    void
    set_column_names(const Us&... colnames);

    size_t
    size() const;

    template<size_t... Inds>
    void
    sort(columnindex<Inds>...);

    template<size_t... Inds>
    frame<Ts...>
    sorted(columnindex<Inds>... ci);

    template<size_t Ind>
    double stddev(columnindex<Ind>) const;

    template<typename T, typename Ex>
    series<T>
    to_series(const std::string& column_name, Ex expr) const;

    std::vector<std::vector<std::string>>
    to_string() const;

private:
    template<size_t Ind, size_t... Inds>
    void
    allow_missing_impl(uframe& uf, columnindex<Inds>... cols) const;

    template<size_t Ind, typename U, typename... Us>
    void
    allow_missing_impl(uframe& uf) const;

    template<size_t Ind>
    void
    clear_impl();

    template<size_t Ind, typename U, typename... Us>
    useries
    column_impl(const std::string& colname) const;

    template<size_t Ind>
    void
    column_names_impl(std::array<std::string, sizeof...(Ts)>& out) const;

    template<typename U, typename... Us>
    void
    columns_impl(uframe& f, const U& u, const Us&... us) const;

    template<size_t Ind, size_t... RemInds>
    void
    columns_impl(uframe& f, columnindexpack<Ind, RemInds...>) const;

    template<size_t Ind, size_t... Inds>
    void
    disallow_missing_impl(uframe& uf, columnindex<Inds>... cols) const;

    template<size_t Ind, typename U, typename... Us>
    void
    disallow_missing_impl(uframe& uf) const;

    template<size_t Ind>
    void
    erase_impl(std::tuple<Ts*...>& ptrs, iterator pos);

    template<size_t Ind>
    void
    erase_impl(std::tuple<Ts*...>& ptrs, iterator first, iterator last);

    template<size_t Ind>
    bool
    eq_impl(const frame<Ts...>& other) const;

    template<size_t Ind, typename U, typename... Us>
    void
    insert_impl(std::tuple<Ts*...>& ptrs, iterator pos, size_t count, const U& u, const Us&... us);

    template<size_t Ind>
    void
    insert_impl(std::tuple<Ts*...>& ptrs, iterator pos, const_iterator first, const_iterator last);

    template<size_t Ind>
    void
    pop_back_impl();

    void
    populate(const std::vector<useries>& columns);

    template<size_t Ind, typename U, typename... Us>
    void
    populate_impl(const std::vector<useries>& columns);

    template<size_t Ind>
    void
    push_back_multiple_empty_impl();

    template<size_t Ind, typename V, typename... Vs>
    void
    push_back_multiple_args_impl(V arg, const Vs&... args);

    template<size_t Ind, bool IsConst, template<bool, typename...> typename Row, typename... Us,
        typename... Vs>
    void
    push_back_multiple_row_impl(const Row<IsConst, Us...>& fr, const Vs&... args);

    template<size_t Ind, typename U, typename... Us>
    void
    push_back_impl(const U& u, const Us&... us);

    template<size_t Ind>
    void
    reserve_impl(size_t newsize);

    template<size_t Ind>
    void
    resize_impl(size_t newsize);

    template<size_t Ind>
    void
    set_column_names_impl(const std::vector<std::string>& names);

    template<size_t Ind>
    void
    set_column_names_impl(const std::array<std::string, sizeof...(Ts)>& names);

    template<size_t Ind, typename U, typename... Us>
    void
    set_column_names_impl(const U& colname, const Us&... colnames);

    template<size_t Ind, typename U, typename... Us>
    size_t
    size_impl_with_check() const;

    template<size_t Ind, typename U, typename... Us>
    void
    to_string_impl(std::vector<std::vector<std::string>>& strs) const;

    template<size_t Ind = 0>
    void
    unref();

    template<typename... Us>
    friend std::ostream&
    operator<<(std::ostream&, const frame<Us...>&);
    friend class uframe;

    std::tuple<series<Ts>...> m_columns;
};

} // namespace mf


#endif // INCLUDED_mainframe_frame_h
