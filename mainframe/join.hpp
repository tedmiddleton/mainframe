//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_join_h
#define INCLUDED_mainframe_join_h

#include "mainframe/detail/frame_indexer.hpp"

namespace mf
{

template<typename... Ts, size_t Ind1, typename... Us, size_t Ind2>
frame<Ts..., Us...>
innerjoin(frame<Ts...> left, columnindex<Ind1>, frame<Us...> right, columnindex<Ind2>)
{
    // These should be comparable
    using LT = typename detail::pack_element<Ind1, Ts...>::type;
    using RT = typename detail::pack_element<Ind2, Us...>::type;
    static_assert(detail::is_equality_comparable<LT, RT>::value,
        "Column types to join on must be equality comparable");

    const frame_indexer<index_defn<Ind1>, Ts...> ileft{ left };
    const frame_indexer<index_defn<Ind2>, Us...> iright{ right };
    ileft.build_index();
    iright.build_index();
    frame<Ts...> fleft;
    fleft.set_column_names(left.column_names());
    frame<Us...> fright;
    fright.set_column_names(right.column_names());

    // Iterator through left index keys
    for (auto liit = ileft.begin_index(); liit != ileft.end_index(); ++liit) {

        auto riit = iright.find_index(liit->first);
        if (riit != iright.end_index()) {

            for (auto lrit = ileft.begin_index_row(liit); lrit != ileft.end_index_row(liit);
                 ++lrit) {
                size_t leftind = *lrit;
                auto leftrow   = *(ileft.begin() + leftind);
                (void)leftrow;

                for (auto rrit = iright.begin_index_row(riit); rrit != iright.end_index_row(riit);
                     ++rrit) {
                    size_t rightind = *rrit;
                    auto rightrow   = *(iright.begin() + rightind);
                    (void)rightrow;

                    fleft.push_back(leftrow);
                    fright.push_back(rightrow);
                }
            }
        }
    }

    // hcat fleft and fright into out
    frame<Ts..., Us...> out = fleft.hcat(fright);

    return out;
}

template<typename... Ts, size_t Ind1, typename... Us, size_t Ind2>
frame<Ts..., Us...>
leftjoin(frame<Ts...> left, columnindex<Ind1>, frame<Us...> right, columnindex<Ind2>)
{
    // These should be comparable
    using LT = typename detail::pack_element<Ind1, Ts...>::type;
    using RT = typename detail::pack_element<Ind2, Us...>::type;
    static_assert(detail::is_equality_comparable<LT, RT>::value,
        "Column types to join on must be equality comparable");

    const frame_indexer<index_defn<Ind1>, Ts...> ileft{ left };
    const frame_indexer<index_defn<Ind2>, Us...> iright{ right };
    ileft.build_index();
    iright.build_index();
    frame<Ts...> fleft;
    fleft.set_column_names(left.column_names());
    frame<Us...> fright;
    fright.set_column_names(right.column_names());

    // Iterator through left index keys
    for (auto liit = ileft.begin_index(); liit != ileft.end_index(); ++liit) {

        auto riit = iright.find_index(liit->first);
        if (riit != iright.end_index()) {

            for (auto lrit = ileft.begin_index_row(liit); lrit != ileft.end_index_row(liit);
                 ++lrit) {
                size_t leftind = *lrit;
                auto leftrow   = *(ileft.begin() + leftind);
                (void)leftrow;

                for (auto rrit = iright.begin_index_row(riit); rrit != iright.end_index_row(riit);
                     ++rrit) {
                    size_t rightind = *rrit;
                    auto rightrow   = *(iright.begin() + rightind);
                    (void)rightrow;

                    fleft.push_back(leftrow);
                    fright.push_back(rightrow);
                }
            }
        }
        else {

            for (auto lrit = ileft.begin_index_row(liit); lrit != ileft.end_index_row(liit);
                 ++lrit) {
                size_t leftind = *lrit;
                auto leftrow   = *(ileft.begin() + leftind);
                (void)leftrow;

                fleft.push_back(leftrow);
                fright.resize(fright.size() + 1);
            }
        }
    }

    // hcat fleft and fright into out
    frame<Ts..., Us...> out = fleft.hcat(fright);

    return out;
}

template<typename... Ts, size_t Ind1, typename... Us, size_t Ind2>
frame<Ts..., Us...>
outerjoin(frame<Ts...> left, columnindex<Ind1>, frame<Us...> right, columnindex<Ind2>)
{
    // These should be comparable
    using LT = typename detail::pack_element<Ind1, Ts...>::type;
    using RT = typename detail::pack_element<Ind2, Us...>::type;
    static_assert(detail::is_equality_comparable<LT, RT>::value,
        "Column types to join on must be equality comparable");

    const frame_indexer<index_defn<Ind1>, Ts...> ileft{ left };
    const frame_indexer<index_defn<Ind2>, Us...> iright{ right };
    ileft.build_index();
    iright.build_index();
    frame<Ts...> fleft;
    fleft.set_column_names(left.column_names());
    frame<Us...> fright;
    fright.set_column_names(right.column_names());

    // Iterator through left index keys
    for (auto liit = ileft.begin_index(); liit != ileft.end_index(); ++liit) {

        auto riit = iright.find_index(liit->first);
        if (riit != iright.end_index()) {

            for (auto lrit = ileft.begin_index_row(liit); lrit != ileft.end_index_row(liit);
                 ++lrit) {
                size_t leftind = *lrit;
                auto leftrow   = *(ileft.begin() + leftind);
                (void)leftrow;

                for (auto rrit = iright.begin_index_row(riit); rrit != iright.end_index_row(riit);
                     ++rrit) {
                    size_t rightind = *rrit;
                    auto rightrow   = *(iright.begin() + rightind);
                    (void)rightrow;

                    fleft.push_back(leftrow);
                    fright.push_back(rightrow);
                }
            }
        }
        else {

            for (auto lrit = ileft.begin_index_row(liit); lrit != ileft.end_index_row(liit);
                 ++lrit) {
                size_t leftind = *lrit;
                auto leftrow   = *(ileft.begin() + leftind);
                (void)leftrow;

                fleft.push_back(leftrow);
                fright.resize(fright.size() + 1);
            }
        }
    }

    // Iterator through right index keys
    for (auto riit = iright.begin_index(); riit != iright.end_index(); ++riit) {

        auto liit = ileft.find_index(riit->first);
        if (liit == ileft.end_index()) {

            for (auto rlit = iright.begin_index_row(riit); rlit != iright.end_index_row(riit);
                ++rlit) {
                size_t rightind = *rlit;
                auto rightrow   = *(iright.begin() + rightind);
                (void)rightrow;
                fright.push_back(rightrow);
                fleft.resize(fleft.size() + 1);
            }
        }
    }

    // hcat fleft and fright into out
    frame<Ts..., Us...> out = fleft.hcat(fright);

    return out;
}

} // namespace mf
  //
#endif // INCLUDED_mainframe_join_h
