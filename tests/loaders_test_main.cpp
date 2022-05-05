
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sstream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mainframe/csv.hpp>

using namespace std;
using namespace mf;

TEST_CASE( "load_csv( istream )", "[frame]" )
{
    stringstream ss;
    ss << "date,temperature,rain\n";
    ss << "2022/January/1,8.9,10\n";
    ss << "2022/January/2,10.0,10\n";
    ss << "2022/January/3,11.1,7\n";
    ss << "2022/January/4,12.2,10\n";
    ss << "2022/January/5,13.3,10\n";
    ss << "2022/January/6,14.4,7\n";
    ss << "2022/January/7,15.5,10\n";
    ss << "2022/January/8,9.1,7\n";
    ss << "2022/January/9,9.3,10\n";

    frame<string, string, string> f1 = load_csv( ss, true );
    //REQUIRE( f1.size() == 9 );
    (void)f1;
}

