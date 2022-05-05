
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_csv_hpp
#define INCLUDED_mainframe_csv_hpp

#include <filesystem>
#include <mainframe/frame.hpp>

namespace mf
{

uframe
load_csv( const std::filesystem::path & path, bool header_line );

uframe
load_csv( std::istream& csv, bool header_line );

} // namespace mf
  //
#endif // INCLUDED_mainframe_csv_hpp

