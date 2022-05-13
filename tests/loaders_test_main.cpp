
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sstream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "date.h"

#include <mainframe/base.hpp>
#include <mainframe/csv.hpp>

using namespace std;
using namespace mf;

template<typename T>
class TD;

TEST_CASE( "load_csv( istream )", "[frame]" )
{
    stringstream ss;
    ss << "date,temperature,rain,air movement\n";
    ss << "2022/January/1,8.9,10,windy\n";
    ss << "2022/January/2,10.0,10,windy\n";
    ss << "2022/January/3,11.1,7,windy\n";
    ss << "2022/January/4,12.2,10,windy\n";
    ss << "2022/January/5,13.3,10,windy\n";
    ss << "2022/January/6,14.4,7,windy\n";
    ss << "2022/January/7,15.5,10,windy\n";
    ss << "2022/January/8,9.1,7,windy\n";
    ss << "2022/January/9,9.3,10,windy\n";
    ss << "2022/January/10,,10,windy\n";
    ss << "2022/January/11,8.2,11,\n";
    ss << ",8.7,12,windy\n";
    ss << "2022/January/13,9.3,10,\"really, not so windy\"\n";
    ss << "2022/January/14,9.2,7,\"my friend said, \"\"crazy, crazy windy!\"\"\"\n";

#define PRINT(x) cout << #x << " = " << x << "\n"
    cout << std::boolalpha;
    PRINT((variant_contains<int, variant<double, int>>::value));
    PRINT((variant_contains<int, variant<int>>::value));
    PRINT((variant_contains<int, variant<double>>::value));
    PRINT((variant_contains<int, variant<double, string, float>>::value));
    PRINT((variant_contains<int, variant<double, float, int>>::value));
    PRINT((variant_contains<int, variant<int, double, float>>::value));
    PRINT((variant_contains<int, variant<float, int, double>>::value));

    using F = variant_unique<date::year_month_day, double, double, int, bool>::type;
    F fff;

    auto f1 = load_csv( 4, ss, true );
    (void)f1;
    cout << f1;
    //REQUIRE( f1.size() == 9 );
}

