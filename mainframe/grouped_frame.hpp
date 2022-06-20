
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_grouped_frame_h
#define INCLUDED_mainframe_grouped_frame_h

#include "mainframe/frame.hpp"

namespace mf
{

template< size_t ... idx >
class group_index
{
};

} // namespace mf

#endif // INCLUDED_mainframe_grouped_frame_h
