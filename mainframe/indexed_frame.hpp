//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_indexed_frame_h
#define INCLUDED_mainframe_indexed_frame_h

#include "mainframe/frame.hpp"

namespace mf
{

template<size_t... Inds>
struct index_defn
{};

namespace detail
{

// Does the index def contain that column index?
// index_defn_contains<3, index_defn<0, 2, 1>>::value == false
// index_defn_contains<1, index_defn<0, 2, 1>>::value == true
template<size_t ColNum, typename IndexDefn>
struct index_defn_contains : std::false_type
{};

template<size_t ColNum, size_t... Inds>
struct index_defn_contains<ColNum, index_defn<ColNum, Inds...>> : std::true_type
{};

template<size_t ColNum, size_t Ind, size_t... Inds>
struct index_defn_contains<ColNum, index_defn<Ind, Inds...>>
    : index_defn_contains<ColNum, index_defn<Inds...>>
{};

// Get the ColNum'th index
// index_defn_deref<3, index_defn<32, 12, 11, 34>>::value == 34
// index_defn_deref<1, index_defn<32, 12, 11, 34>>::value == 12
template<size_t ColNum, typename IndexDefn>
struct index_defn_deref;

template<size_t ColNum, size_t Ind, size_t... Inds>
struct index_defn_deref<ColNum, index_defn<Ind, Inds...>>
{
    static const size_t value = index_defn_deref<ColNum - 1, index_defn<Inds...>>::value;
};

template<size_t Ind, size_t... Inds>
struct index_defn_deref<0, index_defn<Ind, Inds...>>
{
    static const size_t value = Ind;
};

// Make a Tpl with the index defn's columns/items
// index_defn_select<index_defn<0, 4>, tuple<char, short, int, long>>::type == tuple<char, long>
// index_defn_select<index_defn<2, 0>, frame<year_month, double, float>>::type == frame<float,
// year_month>
template<typename IndexDefn, typename Tpl>
struct index_defn_select;

template<size_t Ind, size_t... Inds, template<typename...> typename Tpl, typename... Ts>
struct index_defn_select<index_defn<Ind, Inds...>, Tpl<Ts...>>
{
    using ind_type  = typename pack_element<Ind, Ts...>::type;
    using remaining = typename index_defn_select<index_defn<Inds...>, Tpl<Ts...>>::type;
    using type      = typename prepend<ind_type, remaining>::type;
};

template<size_t Ind, template<typename...> typename Tpl, typename... Ts>
struct index_defn_select<index_defn<Ind>, Tpl<Ts...>>
{
    using ind_type = typename pack_element<Ind, Ts...>::type;
    using type     = Tpl<ind_type>;
};

// Prepend an index to an index defn
// index_defn_prepend<6, index_defn<0, 1, 2>>::type == index_defn<6, 0, 1, 2>
template<size_t Ind, typename IndexDefn>
struct index_defn_prepend;

template<size_t Ind, size_t... Inds>
struct index_defn_prepend<Ind, index_defn<Inds...>>
{
    using type = index_defn<Ind, Inds...>;
};

template<size_t CurrInd, size_t Num, typename IndexDefn>
struct index_defn_invert_impl
{
    using remaining = typename index_defn_invert_impl<CurrInd + 1, Num, IndexDefn>::type;
    using type      = typename std::conditional<index_defn_contains<CurrInd, IndexDefn>::value,
        remaining, typename index_defn_prepend<CurrInd, remaining>::type>::type;
};

template<size_t CurrInd, typename IndexDefn>
struct index_defn_invert_impl<CurrInd, CurrInd, IndexDefn>
{
    using type = index_defn<>;
};

// Invert the index definition for a Tpl
// index_defn_invert<index_defn<0, 3>, frame<char, short, int, long>>::type == index_defn<1, 2>
// index_defn_invert<index_defn<3, 0>, frame<char, short, int, long>>::type == index_defn<1, 2>
template<typename IndexDefn, typename Tpl>
struct index_defn_invert;

template<typename IndexDefn, template<typename...> typename Tpl, typename... Ts>
struct index_defn_invert<IndexDefn, Tpl<Ts...>>
{
    using type = typename index_defn_invert_impl<0, sizeof...(Ts), IndexDefn>::type;
};


// A frame with only the indexed columns in it.
// eg indexed_frame<index_defn<4, 0>, frame<short, double, double, int, year_month>>::type
// == frame<year_month, short>
// This is used to build the frame index
// This is very very similar to detail::rearrange
template<typename IndexDefn, typename Frame>
struct get_index_frame;

template<size_t Ind, size_t... Inds, template<typename...> typename Frame, typename... Ts>
struct get_index_frame<index_defn<Ind, Inds...>, Frame<Ts...>>
{
    using ind_type  = typename pack_element<Ind, Ts...>::type;
    using remaining = typename get_index_frame<index_defn<Inds...>, Frame<Ts...>>::type;
    using type      = typename prepend<ind_type, remaining>::type;

    static type
    op(Frame<Ts...>& f)
    {
        return f.columns(columnindex<Ind>{}, columnindex<Inds>{}...);
    }

    static const type
    op(const Frame<Ts...>& f)
    {
        return f.columns(columnindex<Ind>{}, columnindex<Inds>{}...);
    }
};

template<size_t Ind, template<typename...> typename Frame, typename... Ts>
struct get_index_frame<index_defn<Ind>, Frame<Ts...>>
{
    using ind_type = typename pack_element<Ind, Ts...>::type;
    using type     = frame<ind_type>;

    static type
    op(Frame<Ts...>& f)
    {
        return f.columns(columnindex<Ind>{});
    }

    static const type
    op(const Frame<Ts...>& f)
    {
        return f.columns(columnindex<Ind>{});
    }
};

// A frame with all of the indexed columns removed
// eg unindexed_frame<index_defn<4, 0>, frame<short, double, double, int, year_month>>::type
// is frame<double, double, int>
template<typename IndexDefn, typename Frame>
using get_unindex_frame =
    get_index_frame<typename index_defn_invert<IndexDefn, Frame>::type, Frame>;

} // namespace detail
 
template<typename IndexDefn, typename... Ts>
class indexed_frame;

template<size_t... GroupInds, typename... Ts>
class indexed_frame<index_defn<GroupInds...>, Ts...>
{
protected:
    // The index frame is a frame made of the indexed columns
    using get_index_frame = detail::get_index_frame<index_defn<GroupInds...>, frame<Ts...>>;
    using index_frame     = typename get_index_frame::type;

    using key_type       = typename index_frame::const_value_type; // _row_proxy
    using map_value_type = std::vector<size_t>;
    using map_type       = std::unordered_map<key_type, map_value_type>;

public:
    indexed_frame(frame<Ts...> f)
        : m_frame(f)
    {}

    using iterator = typename frame<Ts...>::iterator;
    using const_iterator = typename frame<Ts...>::const_iterator;
    using index_iterator = typename map_type::iterator;
    using const_index_iterator = typename map_type::const_iterator;
    using index_row_iterator = typename map_value_type::iterator;
    using const_index_row_iterator = typename map_value_type::const_iterator;

    iterator
    begin()
    {
        return m_frame.begin();
    }

    iterator
    end()
    {
        return m_frame.end();
    }

    const_iterator
    begin() const
    {
        return m_frame.begin();
    }

    const_iterator
    end() const
    {
        return m_frame.end();
    }

    const_iterator
    cbegin() const
    {
        return m_frame.cbegin();
    }

    const_iterator
    cend() const
    {
        return m_frame.cend();
    }

    index_iterator
    begin_index()
    {
        return m_idx.begin();
    }

    index_iterator
    end_index()
    {
        return m_idx.end();
    }

    const_index_iterator
    begin_index() const
    {
        return m_idx.begin();
    }

    const_index_iterator
    end_index() const
    {
        return m_idx.end();
    }

    const_index_iterator
    cbegin_index() const
    {
        return m_idx.cbegin();
    }

    const_index_iterator
    cend_index() const
    {
        return m_idx.cend();
    }

    index_row_iterator
    begin_index_row(index_iterator it)
    {
        return it->second.begin();
    }

    index_row_iterator
    end_index_row(index_iterator it)
    {
        return it->second.end();
    }

    const_index_row_iterator
    begin_index_row(const_index_iterator it) const
    {
        return it->second.begin();
    }

    const_index_row_iterator
    end_index_row(const_index_iterator it) const
    {
        return it->second.end();
    }

    const_index_row_iterator
    cbegin_index_row(const_index_iterator it) const
    {
        return it->second.cbegin();
    }

    const_index_row_iterator
    cend_index_row(const_index_iterator it) const
    {
        return it->second.cend();
    }

    index_iterator
    find_index(const key_type& key)
    {
        return m_idx.find(key);
    }

    const_index_iterator
    find_index(const key_type& key) const
    {
        return m_idx.find(key);
    }

    void
    build_index() const
    {
        if (this->m_idx.size() != 0) {
            return;
        }

        const index_frame ifr{ get_index_frame::op(this->m_frame) };
        this->m_idx.reserve(ifr.size());

        for (size_t i = 0; i < ifr.size(); ++i) {
            auto row    = ifr.row(i);
            auto findit = this->m_idx.find(row);
            if (findit == this->m_idx.end()) {
                map_value_type arr;
                arr.push_back(i);
                this->m_idx[row] = arr;
            }
            else {
                findit->second.push_back(i);
            }
        }
    }

    void
    debug_index() const
    {
        std::cout << "Index:\n";
        for (auto& [key, val] : this->m_idx) {
            std::cout << "key " << key << ": ";
            std::cout << "[ ";
            int i = 0;
            for (auto& ind : val) {
                if (i++ != 0) {
                    std::cout << ", ";
                }
                std::cout << ind;
            }
            std::cout << " ]\n";
        }
    }
protected:

    mutable map_type m_idx;
    frame<Ts...> m_frame;
};

} // namespace mf
 
#endif // INCLUDED_mainframe_indexed_frame_h
