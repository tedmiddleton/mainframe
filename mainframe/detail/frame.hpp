//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_detail_frame_h
#define INCLUDED_mainframe_detail_frame_h

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
#include "mainframe/expression.hpp"
#include "mainframe/frame_iterator.hpp"
#include "mainframe/missing.hpp"
#include "mainframe/series.hpp"
#include "mainframe/detail/simd.hpp"

namespace mf
{

template<typename... Ts>
class frame;

}

namespace mf::detail
{

template<typename T, typename U>
struct combine_frames;

template<typename... Ts, typename... Us>
struct combine_frames<frame<Ts...>, frame<Us...>>
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
    using type         = typename combine_frames<frame_type, add_opt_type>::type;
};

template<typename T, typename... Ts, size_t Curr, size_t... IndList>
struct add_opt<frame<T, Ts...>, Curr, IndList...>
{
    static const bool inds_contains = contains<Curr, IndList...>::value;
    using frame_type   = typename std::conditional<inds_contains, frame<mi<T>>, frame<T>>::type;
    using add_opt_type = typename add_opt<frame<Ts...>, Curr + 1, IndList...>::type;
    using type         = typename combine_frames<frame_type, add_opt_type>::type;
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
    using type            = typename combine_frames<frame_type, remove_opt_type>::type;
};

template<typename T, typename... Ts, size_t Curr, size_t... IndList>
struct remove_opt<frame<T, Ts...>, Curr, IndList...>
{
    using frame_type      = frame<T>;
    using remove_opt_type = typename remove_opt<frame<Ts...>, Curr + 1, IndList...>::type;
    using type            = typename combine_frames<frame_type, remove_opt_type>::type;
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

template<typename Row, size_t... Inds>
struct build_lt;

template<typename Row, size_t Ind, size_t... Inds>
struct build_lt<Row, Ind, Inds...>
{
    bool
    operator()(const Row& rowl, const Row& rowr) const
    {
        columnindex<Ind> ci;
        if (rowl.at(ci) == rowr.at(ci)) {
            build_lt<Row, Inds...> remaining;
            return remaining(rowl, rowr);
        }
        else {
            return rowl.at(ci) < rowr.at(ci);
        }
    };
};

template<typename Row, size_t Ind>
struct build_lt<Row, Ind>
{
    bool
    operator()(const Row& rowl, const Row& rowr) const
    {
        columnindex<Ind> ci;
        return rowl.at(ci) < rowr.at(ci);
    };
};

template<typename Row, size_t... Inds>
struct build_gt;

template<typename Row, size_t Ind, size_t... Inds>
struct build_gt<Row, Ind, Inds...>
{
    bool
    operator()(const Row& rowl, const Row& rowr) const
    {
        columnindex<Ind> ci;
        if (rowl.at(ci) == rowr.at(ci)) {
            build_gt<Row, Inds...> remaining;
            return remaining(rowl, rowr);
        }
        else {
            return rowl.at(ci) > rowr.at(ci);
        }
    };
};

template<typename Row, size_t Ind>
struct build_gt<Row, Ind>
{
    bool
    operator()(const Row& rowl, const Row& rowr) const
    {
        columnindex<Ind> ci;
        return rowl.at(ci) > rowr.at(ci);
    };
};

} // namespace mf::detail

#endif // INCLUDED_mainframe_detail_frame_h
