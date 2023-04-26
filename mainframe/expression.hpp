//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_expression_hpp
#define INCLUDED_mainframe_expression_hpp

#include <complex>
#include <string>

#include "mainframe/frame_iterator.hpp"
#include "mainframe/series.hpp"
#include "mainframe/detail/expression.hpp"

namespace mf::placeholders
{

inline constexpr terminal<row_number> rownum;
inline constexpr terminal<frame_length> framelen;
inline constexpr terminal<expr_column<0>> _0;
inline constexpr terminal<expr_column<1>> _1;
inline constexpr terminal<expr_column<2>> _2;
inline constexpr terminal<expr_column<3>> _3;
inline constexpr terminal<expr_column<4>> _4;
inline constexpr terminal<expr_column<5>> _5;
inline constexpr terminal<expr_column<6>> _6;
inline constexpr terminal<expr_column<7>> _7;
inline constexpr terminal<expr_column<8>> _8;
inline constexpr terminal<expr_column<9>> _9;
inline constexpr terminal<expr_column<10>> _10;
inline constexpr terminal<expr_column<11>> _11;
inline constexpr terminal<expr_column<12>> _12;
inline constexpr terminal<expr_column<13>> _13;
inline constexpr terminal<expr_column<14>> _14;
inline constexpr terminal<expr_column<15>> _15;
inline constexpr terminal<expr_column<16>> _16;
inline constexpr terminal<expr_column<17>> _17;
inline constexpr terminal<expr_column<18>> _18;
inline constexpr terminal<expr_column<19>> _19;
inline constexpr terminal<expr_column<20>> _20;
inline constexpr terminal<expr_column<21>> _21;
inline constexpr terminal<expr_column<22>> _22;
inline constexpr terminal<expr_column<23>> _23;
inline constexpr terminal<expr_column<24>> _24;
inline constexpr terminal<expr_column<25>> _25;
inline constexpr terminal<expr_column<26>> _26;
inline constexpr terminal<expr_column<27>> _27;
inline constexpr terminal<expr_column<28>> _28;
inline constexpr terminal<expr_column<29>> _29;
inline constexpr terminal<expr_column<30>> _30;
inline constexpr terminal<expr_column<31>> _31;
inline constexpr terminal<expr_column<32>> _32;
inline constexpr terminal<expr_column<33>> _33;
inline constexpr terminal<expr_column<34>> _34;
inline constexpr terminal<expr_column<35>> _35;
inline constexpr terminal<expr_column<36>> _36;
inline constexpr terminal<expr_column<37>> _37;
inline constexpr terminal<expr_column<38>> _38;
inline constexpr terminal<expr_column<39>> _39;
inline constexpr terminal<expr_column<40>> _40;
inline constexpr terminal<expr_column<41>> _41;
inline constexpr terminal<expr_column<42>> _42;
inline constexpr terminal<expr_column<43>> _43;
inline constexpr terminal<expr_column<44>> _44;
inline constexpr terminal<expr_column<45>> _45;
inline constexpr terminal<expr_column<46>> _46;
inline constexpr terminal<expr_column<47>> _47;
inline constexpr terminal<expr_column<48>> _48;
inline constexpr terminal<expr_column<49>> _49;
inline constexpr terminal<expr_column<50>> _50;
inline constexpr terminal<expr_column<51>> _51;
inline constexpr terminal<expr_column<52>> _52;
inline constexpr terminal<expr_column<53>> _53;
inline constexpr terminal<expr_column<54>> _54;
inline constexpr terminal<expr_column<55>> _55;
inline constexpr terminal<expr_column<56>> _56;
inline constexpr terminal<expr_column<57>> _57;
inline constexpr terminal<expr_column<58>> _58;
inline constexpr terminal<expr_column<59>> _59;
inline constexpr terminal<expr_column<60>> _60;
inline constexpr terminal<expr_column<61>> _61;
inline constexpr terminal<expr_column<62>> _62;
inline constexpr terminal<expr_column<63>> _63;
inline constexpr terminal<expr_column<64>> _64;
inline constexpr terminal<expr_column<65>> _65;
inline constexpr terminal<expr_column<66>> _66;
inline constexpr terminal<expr_column<67>> _67;
inline constexpr terminal<expr_column<68>> _68;
inline constexpr terminal<expr_column<69>> _69;
inline constexpr terminal<expr_column<70>> _70;
inline constexpr terminal<expr_column<71>> _71;
inline constexpr terminal<expr_column<72>> _72;
inline constexpr terminal<expr_column<73>> _73;
inline constexpr terminal<expr_column<74>> _74;
inline constexpr terminal<expr_column<75>> _75;
inline constexpr terminal<expr_column<76>> _76;
inline constexpr terminal<expr_column<77>> _77;
inline constexpr terminal<expr_column<78>> _78;
inline constexpr terminal<expr_column<79>> _79;
inline constexpr terminal<expr_column<80>> _80;
inline constexpr terminal<expr_column<81>> _81;
inline constexpr terminal<expr_column<82>> _82;
inline constexpr terminal<expr_column<83>> _83;
inline constexpr terminal<expr_column<84>> _84;
inline constexpr terminal<expr_column<85>> _85;
inline constexpr terminal<expr_column<86>> _86;
inline constexpr terminal<expr_column<87>> _87;
inline constexpr terminal<expr_column<88>> _88;
inline constexpr terminal<expr_column<89>> _89;
inline constexpr terminal<expr_column<90>> _90;
inline constexpr terminal<expr_column<91>> _91;
inline constexpr terminal<expr_column<92>> _92;
inline constexpr terminal<expr_column<93>> _93;
inline constexpr terminal<expr_column<94>> _94;
inline constexpr terminal<expr_column<95>> _95;
inline constexpr terminal<expr_column<96>> _96;
inline constexpr terminal<expr_column<97>> _97;
inline constexpr terminal<expr_column<98>> _98;
inline constexpr terminal<expr_column<99>> _99;
inline constexpr terminal<expr_column<0>> col0;
inline constexpr terminal<expr_column<1>> col1;
inline constexpr terminal<expr_column<2>> col2;
inline constexpr terminal<expr_column<3>> col3;
inline constexpr terminal<expr_column<4>> col4;
inline constexpr terminal<expr_column<5>> col5;
inline constexpr terminal<expr_column<6>> col6;
inline constexpr terminal<expr_column<7>> col7;
inline constexpr terminal<expr_column<8>> col8;
inline constexpr terminal<expr_column<9>> col9;
inline constexpr terminal<expr_column<10>> col10;
inline constexpr terminal<expr_column<11>> col11;
inline constexpr terminal<expr_column<12>> col12;
inline constexpr terminal<expr_column<13>> col13;
inline constexpr terminal<expr_column<14>> col14;
inline constexpr terminal<expr_column<15>> col15;
inline constexpr terminal<expr_column<16>> col16;
inline constexpr terminal<expr_column<17>> col17;
inline constexpr terminal<expr_column<18>> col18;
inline constexpr terminal<expr_column<19>> col19;
inline constexpr terminal<expr_column<20>> col20;
inline constexpr terminal<expr_column<21>> col21;
inline constexpr terminal<expr_column<22>> col22;
inline constexpr terminal<expr_column<23>> col23;
inline constexpr terminal<expr_column<24>> col24;
inline constexpr terminal<expr_column<25>> col25;
inline constexpr terminal<expr_column<26>> col26;
inline constexpr terminal<expr_column<27>> col27;
inline constexpr terminal<expr_column<28>> col28;
inline constexpr terminal<expr_column<29>> col29;
inline constexpr terminal<expr_column<30>> col30;
inline constexpr terminal<expr_column<31>> col31;
inline constexpr terminal<expr_column<32>> col32;
inline constexpr terminal<expr_column<33>> col33;
inline constexpr terminal<expr_column<34>> col34;
inline constexpr terminal<expr_column<35>> col35;
inline constexpr terminal<expr_column<36>> col36;
inline constexpr terminal<expr_column<37>> col37;
inline constexpr terminal<expr_column<38>> col38;
inline constexpr terminal<expr_column<39>> col39;
inline constexpr terminal<expr_column<40>> col40;
inline constexpr terminal<expr_column<41>> col41;
inline constexpr terminal<expr_column<42>> col42;
inline constexpr terminal<expr_column<43>> col43;
inline constexpr terminal<expr_column<44>> col44;
inline constexpr terminal<expr_column<45>> col45;
inline constexpr terminal<expr_column<46>> col46;
inline constexpr terminal<expr_column<47>> col47;
inline constexpr terminal<expr_column<48>> col48;
inline constexpr terminal<expr_column<49>> col49;
inline constexpr terminal<expr_column<50>> col50;
inline constexpr terminal<expr_column<51>> col51;
inline constexpr terminal<expr_column<52>> col52;
inline constexpr terminal<expr_column<53>> col53;
inline constexpr terminal<expr_column<54>> col54;
inline constexpr terminal<expr_column<55>> col55;
inline constexpr terminal<expr_column<56>> col56;
inline constexpr terminal<expr_column<57>> col57;
inline constexpr terminal<expr_column<58>> col58;
inline constexpr terminal<expr_column<59>> col59;
inline constexpr terminal<expr_column<60>> col60;
inline constexpr terminal<expr_column<61>> col61;
inline constexpr terminal<expr_column<62>> col62;
inline constexpr terminal<expr_column<63>> col63;
inline constexpr terminal<expr_column<64>> col64;
inline constexpr terminal<expr_column<65>> col65;
inline constexpr terminal<expr_column<66>> col66;
inline constexpr terminal<expr_column<67>> col67;
inline constexpr terminal<expr_column<68>> col68;
inline constexpr terminal<expr_column<69>> col69;
inline constexpr terminal<expr_column<70>> col70;
inline constexpr terminal<expr_column<71>> col71;
inline constexpr terminal<expr_column<72>> col72;
inline constexpr terminal<expr_column<73>> col73;
inline constexpr terminal<expr_column<74>> col74;
inline constexpr terminal<expr_column<75>> col75;
inline constexpr terminal<expr_column<76>> col76;
inline constexpr terminal<expr_column<77>> col77;
inline constexpr terminal<expr_column<78>> col78;
inline constexpr terminal<expr_column<79>> col79;
inline constexpr terminal<expr_column<80>> col80;
inline constexpr terminal<expr_column<81>> col81;
inline constexpr terminal<expr_column<82>> col82;
inline constexpr terminal<expr_column<83>> col83;
inline constexpr terminal<expr_column<84>> col84;
inline constexpr terminal<expr_column<85>> col85;
inline constexpr terminal<expr_column<86>> col86;
inline constexpr terminal<expr_column<87>> col87;
inline constexpr terminal<expr_column<88>> col88;
inline constexpr terminal<expr_column<89>> col89;
inline constexpr terminal<expr_column<90>> col90;
inline constexpr terminal<expr_column<91>> col91;
inline constexpr terminal<expr_column<92>> col92;
inline constexpr terminal<expr_column<93>> col93;
inline constexpr terminal<expr_column<94>> col94;
inline constexpr terminal<expr_column<95>> col95;
inline constexpr terminal<expr_column<96>> col96;
inline constexpr terminal<expr_column<97>> col97;
inline constexpr terminal<expr_column<98>> col98;
inline constexpr terminal<expr_column<99>> col99;

} // namespace mf::placeholders


#endif // INCLUDED_mainframe_expression_hpp
