
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <map>
#include <ostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "date.h"
#include "debug_cout.hpp"
#include <mainframe/base.hpp>
#include <mainframe/frame.hpp>
#include <mainframe/grouped_frame.hpp>

using namespace std;
using namespace std::chrono;
using namespace std::literals;
using namespace date::literals;
using namespace date;
using namespace mf;
using namespace mf::placeholders;


namespace std
{
template<>
struct hash<date::year_month_day>
{
    size_t
    operator()(const date::year_month_day& ymd) const noexcept
    {
        auto hi = std::hash<int>{};
        auto hu = std::hash<unsigned>{};
        auto y  = hi(static_cast<int>(ymd.year()));
        auto m  = hu(static_cast<unsigned>(ymd.month()));
        auto d  = hu(static_cast<unsigned>(ymd.day()));
        return y ^ m ^ d;
    }
};

template<>
struct hash<date::year_month>
{
    size_t
    operator()(const date::year_month& ym) const noexcept
    {
        auto hi = std::hash<int>{};
        auto hu = std::hash<unsigned>{};
        auto y  = hi(static_cast<int>(ym.year()));
        auto m  = hu(static_cast<unsigned>(ym.month()));
        return y ^ m;
    }
};

}

struct HasNoStreamOp
{
    double d{ -9999.99 };
    int i{ -9999 };
};

struct HasStreamOp
{
    double d{ -9999.99 };
    int i{ -9999 };
    friend ostream&
    operator<<(ostream& o, const HasStreamOp& tc)
    {
        o << "HasStreamOp{d=" << tc.d << ",i=" << tc.i << "}";
        return o;
    }
};

class TestType
{
public:
    TestType() = default;
    TestType(int x)
        : f1((double)x)
        , f2(x)
    {}
    double
    getf1()
    {
        return f1;
    }
    int
    getf2()
    {
        return f2;
    }

private:
    double f1;
    int f2;

    friend ostream& operator<<(ostream&, const TestType&);
};

ostream&
operator<<(ostream& o, const TestType& tt)
{
    o << "TestType{ f1=" << tt.f1 << ", f2=" << tt.f2 << " }";
    return o;
}

template<typename X>
class TestTypeX
{
public:
    TestTypeX() = default;
    TestTypeX(X x)
        : f1(x)
    {}
    X
    getf1()
    {
        return f1;
    }

private:
    X f1;

    template<typename Y>
    friend ostream& operator<<(ostream&, const TestTypeX<Y>&);
};

template<typename X>
ostream&
operator<<(ostream& o, const TestTypeX<X>& tt)
{
    o << "TestTypeX{ f1=" << tt.f1 << " }";
    return o;
}

TEST_CASE("ctor"
          "[frame]")
{
    frame<year_month_day, double, bool> f1;
    REQUIRE(f1.size() == 0);
}

TEST_CASE("ctor( const& )"
          "[frame]")
{
    frame<year_month_day, double, bool> f1;
    REQUIRE(f1.size() == 0);
    f1.push_back(2022_y / January / 2, 10.9, true);
    f1.push_back(2022_y / January / 3, 11.0, true);
    f1.push_back(2022_y / January / 4, 11.1, false);
    f1.push_back(2022_y / January / 5, 11.2, true);
    REQUIRE(f1.size() == 4);
    frame<year_month_day, double, bool> f2(f1);
    REQUIRE(f1.size() == 4);
    REQUIRE(f2.size() == 4);
}

TEST_CASE("ctor( && )"
          "[frame]")
{
    frame<year_month_day, double, bool> f1;
    REQUIRE(f1.size() == 0);
    f1.push_back(2022_y / January / 2, 10.9, true);
    f1.push_back(2022_y / January / 3, 11.0, true);
    f1.push_back(2022_y / January / 4, 11.1, false);
    f1.push_back(2022_y / January / 5, 11.2, true);
    REQUIRE(f1.size() == 4);
    frame<year_month_day, double, bool> f2(std::move(f1));
    REQUIRE(f1.size() == 0);
    REQUIRE(f2.size() == 4);
}

TEST_CASE("begin()/end()", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.9, true);
    f1.push_back(2022_y / January / 3, 11.0, true);
    f1.push_back(2022_y / January / 4, 11.1, false);
    f1.push_back(2022_y / January / 5, 11.2, true);
    REQUIRE(f1.column_name<0>() == "date");
    REQUIRE(f1.column_name<1>() == "temperature");
    REQUIRE(f1.column_name<2>() == "rain");
    REQUIRE(f1.size() == 4);

    auto f1i = f1.begin();
    auto f1e = f1.end();
    REQUIRE((f1e - f1i) == 4);
    REQUIRE(f1i->at(_0) == 2022_y / January / 2);
    REQUIRE(f1i->at(_1) == 10.9);
    REQUIRE(f1i->at(_2) == true);
    f1i += 2;
    REQUIRE(f1i->at(_0) == 2022_y / January / 4);
    REQUIRE(f1i->at(_1) == 11.1);
    REQUIRE(f1i->at(_2) == false);
    ++f1i;
    REQUIRE(f1i->at(_0) == 2022_y / January / 5);
    REQUIRE(f1i->at(_1) == 11.2);
    REQUIRE(f1i->at(_2) == true);
    f1i++;
    REQUIRE(f1i == f1e);
    f1i--;
    REQUIRE(f1i->at(_0) == 2022_y / January / 5);
    REQUIRE(f1i->at(_1) == 11.2);
    REQUIRE(f1i->at(_2) == true);
    f1i -= 2;
    REQUIRE(f1i->at(_0) == 2022_y / January / 3);
    REQUIRE(f1i->at(_1) == 11.0);
    REQUIRE(f1i->at(_2) == true);
    --f1i;
    REQUIRE(f1i->at(_0) == 2022_y / January / 2);
    REQUIRE(f1i->at(_1) == 10.9);
    REQUIRE(f1i->at(_2) == true);
}

TEST_CASE("rbegin()/rend()", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.9, true);
    f1.push_back(2022_y / January / 3, 11.0, true);
    f1.push_back(2022_y / January / 4, 11.1, false);
    f1.push_back(2022_y / January / 5, 11.2, true);
    REQUIRE(f1.column_name<0>() == "date");
    REQUIRE(f1.column_name<1>() == "temperature");
    REQUIRE(f1.column_name<2>() == "rain");
    REQUIRE(f1.size() == 4);

    auto f1i = f1.rbegin();
    auto f1e = f1.rend();
    REQUIRE((f1e - f1i) == 4);
    REQUIRE(f1i->at(_0) == 2022_y / January / 5);
    REQUIRE(f1i->at(_1) == 11.2);
    REQUIRE(f1i->at(_2) == true);
    f1i += 2;
    REQUIRE(f1i->at(_0) == 2022_y / January / 3);
    REQUIRE(f1i->at(_1) == 11.0);
    REQUIRE(f1i->at(_2) == true);
    ++f1i;
    REQUIRE(f1i->at(_0) == 2022_y / January / 2);
    REQUIRE(f1i->at(_1) == 10.9);
    REQUIRE(f1i->at(_2) == true);
    f1i++;
    REQUIRE(f1i == f1e);
    f1i--;
    REQUIRE(f1i->at(_0) == 2022_y / January / 2);
    REQUIRE(f1i->at(_1) == 10.9);
    REQUIRE(f1i->at(_2) == true);
    f1i -= 2;
    REQUIRE(f1i->at(_0) == 2022_y / January / 4);
    REQUIRE(f1i->at(_1) == 11.1);
    REQUIRE(f1i->at(_2) == false);
    --f1i;
    REQUIRE(f1i->at(_0) == 2022_y / January / 5);
    REQUIRE(f1i->at(_1) == 11.2);
    REQUIRE(f1i->at(_2) == true);
}

TEST_CASE("empty()", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    REQUIRE(f1.empty());
    f1.set_column_names("date", "temperature", "rain");
    REQUIRE(f1.empty());
    f1.push_back(2022_y / January / 2, 10.9, true);
    REQUIRE(!f1.empty());
    f1.push_back(2022_y / January / 3, 11.0, true);
    REQUIRE(!f1.empty());
}

TEST_CASE("size()/clear()", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    REQUIRE(f1.size() == 0);
    f1.push_back(2022_y / January / 2, 10.9, true);
    REQUIRE(f1.size() == 1);
    f1.clear();
    REQUIRE(f1.size() == 0);
    f1.set_column_names("date", "temperature", "rain");
    REQUIRE(f1.size() == 0);
    f1.push_back(2022_y / January / 2, 10.9, true);
    REQUIRE(f1.size() == 1);
    f1.push_back(2022_y / January / 3, 11.0, true);
    REQUIRE(f1.size() == 2);
}

TEST_CASE("insert( pos first last )", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, false);
    f1.push_back(2022_y / January / 4, 12.2, false);
    REQUIRE(f1.size() == 3);
    frame<year_month_day, double, bool> f2;
    f2.push_back(2021_y / March / 12, -110.9, true);
    f2.push_back(2021_y / March / 13, -111.0, true);
    f2.push_back(2021_y / March / 14, -112.1, true);
    REQUIRE(f2.size() == 3);

    auto resit = f1.insert(f1.begin() + 1, f2.cbegin(), f2.cend());
    REQUIRE(f1.size() == 6);
    auto b = f1.begin();
    REQUIRE(resit == b + 1);
    REQUIRE(b->at(_0) == 2022_y / January / 2);
    REQUIRE(b->at(_1) == 10.0);
    REQUIRE(b->at(_2) == false);
    b++;
    REQUIRE(b->at(_0) == 2021_y / March / 12);
    REQUIRE(b->at(_1) == -110.9);
    REQUIRE(b->at(_2) == true);
    ++b;
    REQUIRE(b->at(_0) == 2021_y / March / 13);
    REQUIRE(b->at(_1) == -111.0);
    REQUIRE(b->at(_2) == true);
    b += 1;
    REQUIRE(b->at(_0) == 2021_y / March / 14);
    REQUIRE(b->at(_1) == -112.1);
    REQUIRE(b->at(_2) == true);
    b++;
    REQUIRE(b->at(_0) == 2022_y / January / 3);
    REQUIRE(b->at(_1) == 11.1);
    REQUIRE(b->at(_2) == false);
    b++;
    REQUIRE(b->at(_0) == 2022_y / January / 4);
    REQUIRE(b->at(_1) == 12.2);
    REQUIRE(b->at(_2) == false);
    b -= 3;
    REQUIRE(b->at(_0) == 2021_y / March / 13);
    REQUIRE(b->at(_1) == -111.0);
    REQUIRE(b->at(_2) == true);
}

TEST_CASE("insert( pos value )", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, false);
    f1.push_back(2022_y / January / 4, 12.2, false);
    REQUIRE(f1.size() == 3);
    auto resit = f1.insert(f1.begin() + 1, 2021_y / March / 14, -112.1, true);
    REQUIRE(resit == f1.begin() + 1);
    REQUIRE(f1.size() == 4);
    resit = f1.insert(f1.begin() + 1, 2021_y / March / 13, -111.0, true);
    REQUIRE(resit == f1.begin() + 1);
    REQUIRE(f1.size() == 5);
    resit = f1.insert(f1.begin() + 1, 2021_y / March / 12, -110.9, true);
    REQUIRE(resit == f1.begin() + 1);
    REQUIRE(f1.size() == 6);
    auto b = f1.begin();
    REQUIRE(resit == b + 1);
    REQUIRE(b->at(_0) == 2022_y / January / 2);
    REQUIRE(b->at(_1) == 10.0);
    REQUIRE(b->at(_2) == false);
    b++;
    REQUIRE(b->at(_0) == 2021_y / March / 12);
    REQUIRE(b->at(_1) == -110.9);
    REQUIRE(b->at(_2) == true);
    ++b;
    REQUIRE(b->at(_0) == 2021_y / March / 13);
    REQUIRE(b->at(_1) == -111.0);
    REQUIRE(b->at(_2) == true);
    b += 1;
    REQUIRE(b->at(_0) == 2021_y / March / 14);
    REQUIRE(b->at(_1) == -112.1);
    REQUIRE(b->at(_2) == true);
    b++;
    REQUIRE(b->at(_0) == 2022_y / January / 3);
    REQUIRE(b->at(_1) == 11.1);
    REQUIRE(b->at(_2) == false);
    b++;
    REQUIRE(b->at(_0) == 2022_y / January / 4);
    REQUIRE(b->at(_1) == 12.2);
    REQUIRE(b->at(_2) == false);
    b -= 3;
    REQUIRE(b->at(_0) == 2021_y / March / 13);
    REQUIRE(b->at(_1) == -111.0);
    REQUIRE(b->at(_2) == true);
}

TEST_CASE("insert( pos count value )", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, false);
    f1.push_back(2022_y / January / 4, 12.2, false);
    REQUIRE(f1.size() == 3);
    auto resit = f1.insert(f1.begin() + 1, 2, 2021_y / March / 14, -112.1, true);
    REQUIRE(resit == f1.begin() + 1);
    REQUIRE(f1.size() == 5);
    resit = f1.insert(f1.begin() + 1, 1, 2021_y / March / 13, -111.0, true);
    REQUIRE(resit == f1.begin() + 1);
    REQUIRE(f1.size() == 6);
    resit = f1.insert(f1.begin() + 1, 2, 2021_y / March / 12, -110.9, true);
    REQUIRE(resit == f1.begin() + 1);
    REQUIRE(f1.size() == 8);
    auto b = f1.begin();
    REQUIRE(resit == b + 1);
    REQUIRE(b->at(_0) == 2022_y / January / 2);
    REQUIRE(b->at(_1) == 10.0);
    REQUIRE(b->at(_2) == false);
    b++;
    REQUIRE(b->at(_0) == 2021_y / March / 12);
    REQUIRE(b->at(_1) == -110.9);
    REQUIRE(b->at(_2) == true);
    ++b;
    REQUIRE(b->at(_0) == 2021_y / March / 12);
    REQUIRE(b->at(_1) == -110.9);
    REQUIRE(b->at(_2) == true);
    ++b;
    REQUIRE(b->at(_0) == 2021_y / March / 13);
    REQUIRE(b->at(_1) == -111.0);
    REQUIRE(b->at(_2) == true);
    b += 1;
    REQUIRE(b->at(_0) == 2021_y / March / 14);
    REQUIRE(b->at(_1) == -112.1);
    REQUIRE(b->at(_2) == true);
    b++;
    REQUIRE(b->at(_0) == 2021_y / March / 14);
    REQUIRE(b->at(_1) == -112.1);
    REQUIRE(b->at(_2) == true);
    b++;
    REQUIRE(b->at(_0) == 2022_y / January / 3);
    REQUIRE(b->at(_1) == 11.1);
    REQUIRE(b->at(_2) == false);
    b++;
    REQUIRE(b->at(_0) == 2022_y / January / 4);
    REQUIRE(b->at(_1) == 12.2);
    REQUIRE(b->at(_2) == false);
    b -= 4;
    REQUIRE(b->at(_0) == 2021_y / March / 13);
    REQUIRE(b->at(_1) == -111.0);
    REQUIRE(b->at(_2) == true);
}

TEST_CASE("erase( first last )", "[frame]")
{
    SECTION("begin")
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 2, 10.0, false);
        f1.push_back(2022_y / January / 3, 11.1, false);
        f1.push_back(2022_y / January / 4, 12.2, false);
        REQUIRE(f1.size() == 3);
        auto resit = f1.erase(f1.begin(), f1.begin() + 1);
        REQUIRE(resit == f1.begin());
        REQUIRE(f1.size() == 2);
        REQUIRE(f1.begin()->at(_0) == 2022_y / January / 3);
        REQUIRE(f1.begin()->at(_1) == 11.1);
        REQUIRE(f1.begin()->at(_2) == false);
        REQUIRE((f1.begin() + 1)->at(_0) == 2022_y / January / 4);
        REQUIRE((f1.begin() + 1)->at(_1) == 12.2);
        REQUIRE((f1.begin() + 1)->at(_2) == false);
    }

    SECTION("begin multiple")
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 2, 10.0, false);
        f1.push_back(2022_y / January / 3, 11.1, false);
        f1.push_back(2022_y / January / 4, 12.2, false);
        REQUIRE(f1.size() == 3);
        auto resit = f1.erase(f1.begin(), f1.begin() + 2);
        REQUIRE(resit == f1.begin());
        REQUIRE(f1.size() == 1);
        REQUIRE(f1.begin()->at(_0) == 2022_y / January / 4);
        REQUIRE(f1.begin()->at(_1) == 12.2);
        REQUIRE(f1.begin()->at(_2) == false);
    }

    SECTION("end 1")
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 2, 10.0, false);
        f1.push_back(2022_y / January / 3, 11.1, false);
        f1.push_back(2022_y / January / 4, 12.2, false);
        REQUIRE(f1.size() == 3);
        auto resit = f1.erase(f1.end() - 1, f1.end());
        REQUIRE(resit == f1.end());
        REQUIRE(f1.size() == 2);
        REQUIRE(f1.begin()->at(_0) == 2022_y / January / 2);
        REQUIRE(f1.begin()->at(_1) == 10.0);
        REQUIRE(f1.begin()->at(_2) == false);
        REQUIRE((f1.begin() + 1)->at(_0) == 2022_y / January / 3);
        REQUIRE((f1.begin() + 1)->at(_1) == 11.1);
        REQUIRE((f1.begin() + 1)->at(_2) == false);
    }
}

TEST_CASE("erase( pos )", "[frame]")
{
    SECTION("begin")
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 2, 10.0, false);
        f1.push_back(2022_y / January / 3, 11.1, false);
        f1.push_back(2022_y / January / 4, 12.2, false);
        REQUIRE(f1.size() == 3);
        auto resit = f1.erase(f1.begin());
        REQUIRE(resit == f1.begin());
        REQUIRE(f1.size() == 2);
        REQUIRE(f1.begin()->at(_0) == 2022_y / January / 3);
        REQUIRE(f1.begin()->at(_1) == 11.1);
        REQUIRE(f1.begin()->at(_2) == false);
        REQUIRE((f1.begin() + 1)->at(_0) == 2022_y / January / 4);
        REQUIRE((f1.begin() + 1)->at(_1) == 12.2);
        REQUIRE((f1.begin() + 1)->at(_2) == false);
    }

    SECTION("end -1")
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 2, 10.0, false);
        f1.push_back(2022_y / January / 3, 11.1, false);
        f1.push_back(2022_y / January / 4, 12.2, false);
        REQUIRE(f1.size() == 3);
        auto resit = f1.erase(f1.end() - 1);
        REQUIRE(resit == f1.end());
        REQUIRE(f1.size() == 2);
        REQUIRE(f1.begin()->at(_0) == 2022_y / January / 2);
        REQUIRE(f1.begin()->at(_1) == 10.0);
        REQUIRE(f1.begin()->at(_2) == false);
        REQUIRE((f1.begin() + 1)->at(_0) == 2022_y / January / 3);
        REQUIRE((f1.begin() + 1)->at(_1) == 11.1);
        REQUIRE((f1.begin() + 1)->at(_2) == false);
    }
}

TEST_CASE("push_back() pop_back()", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, false);
    f1.push_back(2022_y / January / 4, 12.2, false);
    REQUIRE(f1.size() == 3);
    REQUIRE(f1.begin()->at(_0) == 2022_y / January / 2);
    REQUIRE(f1.begin()->at(_1) == 10.0);
    REQUIRE(f1.begin()->at(_2) == false);
    REQUIRE((f1.begin() + 1)->at(_0) == 2022_y / January / 3);
    REQUIRE((f1.begin() + 1)->at(_1) == 11.1);
    REQUIRE((f1.begin() + 1)->at(_2) == false);
    REQUIRE((f1.begin() + 2)->at(_0) == 2022_y / January / 4);
    REQUIRE((f1.begin() + 2)->at(_1) == 12.2);
    REQUIRE((f1.begin() + 2)->at(_2) == false);
    f1.pop_back();
    f1.pop_back();
    REQUIRE(f1.size() == 1);
    REQUIRE(f1.begin()->at(_0) == 2022_y / January / 2);
    REQUIRE(f1.begin()->at(_1) == 10.0);
    REQUIRE(f1.begin()->at(_2) == false);
}

// TEST_CASE( "push_back( variant )", "[frame]" )
//{
//     SECTION( "no missing" )
//     {
//         frame<year_month_day, double, bool> f1;
//         f1.set_column_names( "date", "temperature", "rain" );
//
//         using row_type = decltype(f1)::row_type;
//         row_type row;
//         row.push_back( 2022_y/January/2 ); row.push_back( 10.0 ); row.push_back( false );
//         f1.push_back( row );
//         REQUIRE( std::holds_alternative<year_month_day>( row[0] ) );
//         REQUIRE( std::holds_alternative<double>( row[1] ) );
//         REQUIRE( std::holds_alternative<bool>( row[2] ) );
//         row.clear();
//         row.push_back( 2022_y/January/3 ); row.push_back( 11.1 ); row.push_back( true );
//         f1.push_back( row );
//         REQUIRE( std::holds_alternative<year_month_day>( row[0] ) );
//         REQUIRE( std::holds_alternative<double>( row[1] ) );
//         REQUIRE( std::holds_alternative<bool>( row[2] ) );
//         row.clear();
//         row.push_back( 2022_y/January/4 ); row.push_back( 12.2 ); row.push_back( false );
//         f1.push_back( row );
//         REQUIRE( std::holds_alternative<year_month_day>( row[0] ) );
//         REQUIRE( std::holds_alternative<double>( row[1] ) );
//         REQUIRE( std::holds_alternative<bool>( row[2] ) );
//         row.clear();
//         REQUIRE( f1.size() == 3 );
//         REQUIRE( f1.begin()->at(_0) == 2022_y/January/2 );
//         REQUIRE( f1.begin()->at(_1) == 10.0 );
//         REQUIRE( f1.begin()->at(_2) == false );
//         REQUIRE( (f1.begin() + 1)->at(_0) == 2022_y/January/3 );
//         REQUIRE( (f1.begin() + 1)->at(_1) == 11.1 );
//         REQUIRE( (f1.begin() + 1)->at(_2) == true );
//         REQUIRE( (f1.begin() + 2)->at(_0) == 2022_y/January/4 );
//         REQUIRE( (f1.begin() + 2)->at(_1) == 12.2 );
//         REQUIRE( (f1.begin() + 2)->at(_2) == false );
//     }
//
//     SECTION( "missing" )
//     {
//         frame<year_month_day, double, bool> f0;
//         f0.set_column_names( "date", "temperature", "rain" );
//         auto f1 = f0.allow_missing( _1, _2 );
//
//         using row_type = decltype(f1)::row_type;
//         row_type row;
//         row.push_back( 2022_y/January/2 ); row.push_back( mi(10.0) ); row.push_back( mi(false) );
//         f1.push_back( row );
//         REQUIRE( std::holds_alternative<year_month_day>( row[0] ) );
//         REQUIRE( std::holds_alternative<mi<double>>( row[1] ) );
//         REQUIRE( std::holds_alternative<mi<bool>>( row[2] ) );
//         row.clear();
//         row.push_back( 2022_y/January/3 ); row.push_back( mi<double>(missing) ); row.push_back(
//         mi<bool>(missing) ); f1.push_back( row ); REQUIRE(
//         std::holds_alternative<year_month_day>( row[0] ) ); REQUIRE(
//         std::holds_alternative<mi<double>>( row[1] ) ); REQUIRE(
//         std::holds_alternative<mi<bool>>( row[2] ) ); row.clear(); row.push_back(
//         2022_y/January/4 ); row.push_back( mi(12.2) ); row.push_back( mi(true) ); f1.push_back(
//         row ); REQUIRE( std::holds_alternative<year_month_day>( row[0] ) ); REQUIRE(
//         std::holds_alternative<mi<double>>( row[1] ) ); REQUIRE(
//         std::holds_alternative<mi<bool>>( row[2] ) ); row.clear(); REQUIRE( f1.size() == 3 );
//         REQUIRE( f1.begin()->at(_0) == 2022_y/January/2 );
//         REQUIRE( f1.begin()->at(_1) == 10.0 );
//         REQUIRE( f1.begin()->at(_2) == false );
//         REQUIRE( (f1.begin() + 1)->at(_0) == 2022_y/January/3 );
//         REQUIRE( (f1.begin() + 1)->at(_1) == missing );
//         REQUIRE( (f1.begin() + 1)->at(_2) == missing );
//         REQUIRE( (f1.begin() + 2)->at(_0) == 2022_y/January/4 );
//         REQUIRE( (f1.begin() + 2)->at(_1) == 12.2 );
//         REQUIRE( (f1.begin() + 2)->at(_2) == true );
//     }
// }

TEST_CASE("resize()", "[frame]")
{
    SECTION("bigger")
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 2, 10.0, false);
        f1.push_back(2022_y / January / 3, 11.1, false);
        f1.push_back(2022_y / January / 4, 12.2, false);
        REQUIRE(f1.size() == 3);
        f1.resize(f1.size() + 1);
        REQUIRE(f1.size() == 4);
        REQUIRE((f1.begin() + 3)->at(_0) == 0_y / 0 / 0);
        REQUIRE((f1.begin() + 3)->at(_1) == 0.0);
        REQUIRE((f1.begin() + 3)->at(_2) == false);
    }

    SECTION("smaller")
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 2, 10.0, false);
        f1.push_back(2022_y / January / 3, 11.1, false);
        f1.push_back(2022_y / January / 4, 12.2, false);
        REQUIRE(f1.size() == 3);
        f1.resize(f1.size() - 1);
        REQUIRE(f1.size() == 2);
        REQUIRE(f1.begin()->at(_0) == 2022_y / January / 2);
        REQUIRE(f1.begin()->at(_1) == 10.0);
        REQUIRE(f1.begin()->at(_2) == false);
        REQUIRE((f1.begin() + 1)->at(_0) == 2022_y / January / 3);
        REQUIRE((f1.begin() + 1)->at(_1) == 11.1);
        REQUIRE((f1.begin() + 1)->at(_2) == false);
    }
}

TEST_CASE("columns()", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, false);
    f1.push_back(2022_y / January / 4, 12.2, false);
    frame<double, year_month_day> f2 = f1.columns("temperature", "date");
    REQUIRE(f2.num_columns() == 2);
    REQUIRE(f2.column_name<0>() == "temperature");
    REQUIRE(f2.column_name<1>() == "date");
    REQUIRE(f2.begin()->at(_0) == 10.0);
    REQUIRE(f2.begin()->at(_1) == 2022_y / January / 2);
    REQUIRE((f2.begin() + 1)->at(_0) == 11.1);
    REQUIRE((f2.begin() + 1)->at(_1) == 2022_y / January / 3);
    REQUIRE((f2.begin() + 2)->at(_0) == 12.2);
    REQUIRE((f2.begin() + 2)->at(_1) == 2022_y / January / 4);

    auto f3 = f1.columns(_2, _0);
    REQUIRE(f3.column_name<0>() == "rain");
    REQUIRE(f3.column_name<1>() == "date");
    REQUIRE(f3.begin()->at(_0) == false);
    REQUIRE(f3.begin()->at(_1) == 2022_y / January / 2);
}

TEST_CASE("rows()", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, true);
    f1.push_back(2022_y / January / 4, 12.2, false);
    f1.push_back(2022_y / January / 5, 13.3, false);
    f1.push_back(2022_y / January / 6, 14.4, true);
    f1.push_back(2022_y / January / 7, 15.5, false);

    auto frain             = f1.rows(col<2>() == true);
    auto fnorain           = f1.rows(false == col<2>());
    auto fhot              = f1.rows(_1 >= 14);
    auto fhotandrain       = f1.rows(col1 >= 13 && col2 == true);
    auto fcoldandrain      = f1.rows(_1 <= 12 && _2 == true && _0 > 2022_y / January / 4);
    auto f2or3orhotandrain = f1.rows(
        (_1 >= 13 && _2 == true) || _0 == 2022_y / January / 2 || _0 == 2022_y / January / 3);

    REQUIRE(frain.size() == 2);
    REQUIRE(fnorain.size() == 4);
    REQUIRE(fhot.size() == 2);
    REQUIRE(fhotandrain.size() == 1);
    REQUIRE(fcoldandrain.empty());
    REQUIRE(f2or3orhotandrain.size() == 3);

    REQUIRE((frain.begin() + 0)->at(_0) == 2022_y / January / 3);
    REQUIRE((frain.begin() + 0)->at(_1) == 11.1);
    REQUIRE((frain.begin() + 0)->at(_2) == true);
    REQUIRE((frain.begin() + 1)->at(_0) == 2022_y / January / 6);
    REQUIRE((frain.begin() + 1)->at(_1) == 14.4);
    REQUIRE((frain.begin() + 1)->at(_2) == true);

    REQUIRE((fnorain.begin() + 0)->at(_0) == 2022_y / January / 2);
    REQUIRE((fnorain.begin() + 0)->at(_1) == 10.0);
    REQUIRE((fnorain.begin() + 0)->at(_2) == false);
    REQUIRE((fnorain.begin() + 1)->at(_0) == 2022_y / January / 4);
    REQUIRE((fnorain.begin() + 1)->at(_1) == 12.2);
    REQUIRE((fnorain.begin() + 1)->at(_2) == false);
    REQUIRE((fnorain.begin() + 2)->at(_0) == 2022_y / January / 5);
    REQUIRE((fnorain.begin() + 2)->at(_1) == 13.3);
    REQUIRE((fnorain.begin() + 2)->at(_2) == false);
    REQUIRE((fnorain.begin() + 3)->at(_0) == 2022_y / January / 7);
    REQUIRE((fnorain.begin() + 3)->at(_1) == 15.5);
    REQUIRE((fnorain.begin() + 3)->at(_2) == false);

    REQUIRE((fhot.begin() + 0)->at(_0) == 2022_y / January / 6);
    REQUIRE((fhot.begin() + 0)->at(_1) == 14.4);
    REQUIRE((fhot.begin() + 0)->at(_2) == true);
    REQUIRE((fhot.begin() + 1)->at(_0) == 2022_y / January / 7);
    REQUIRE((fhot.begin() + 1)->at(_1) == 15.5);
    REQUIRE((fhot.begin() + 1)->at(_2) == false);

    REQUIRE((fhotandrain.begin() + 0)->at(_0) == 2022_y / January / 6);
    REQUIRE((fhotandrain.begin() + 0)->at(_1) == 14.4);
    REQUIRE((fhotandrain.begin() + 0)->at(_2) == true);

    REQUIRE((f2or3orhotandrain.begin() + 0)->at(_0) == 2022_y / January / 2);
    REQUIRE((f2or3orhotandrain.begin() + 0)->at(_1) == 10.0);
    REQUIRE((f2or3orhotandrain.begin() + 0)->at(_2) == false);
    REQUIRE((f2or3orhotandrain.begin() + 1)->at(_0) == 2022_y / January / 3);
    REQUIRE((f2or3orhotandrain.begin() + 1)->at(_1) == 11.1);
    REQUIRE((f2or3orhotandrain.begin() + 1)->at(_2) == true);
    REQUIRE((f2or3orhotandrain.begin() + 2)->at(_0) == 2022_y / January / 6);
    REQUIRE((f2or3orhotandrain.begin() + 2)->at(_1) == 14.4);
    REQUIRE((f2or3orhotandrain.begin() + 2)->at(_2) == true);
}

TEST_CASE("operator+", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    frame<year_month_day, double, bool> f2;
    f1.set_column_names("date", "temperature", "rain");
    f2.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, true);
    f1.push_back(2022_y / January / 4, 12.2, false);
    f2.push_back(2022_y / January / 5, 13.3, false);
    f2.push_back(2022_y / January / 6, 14.4, true);
    f2.push_back(2022_y / January / 7, 15.5, false);
    REQUIRE(f1.size() == 3);
    REQUIRE(f2.size() == 3);

    auto f3 = f2 + f1;

    REQUIRE(f1.size() == 3);
    REQUIRE(f2.size() == 3);
    REQUIRE(f3.size() == 6);

    REQUIRE((f3.begin() + 0)->at(_0) == 2022_y / January / 5);
    REQUIRE((f3.begin() + 0)->at(_1) == 13.3);
    REQUIRE((f3.begin() + 0)->at(_2) == false);
    REQUIRE((f3.begin() + 1)->at(_0) == 2022_y / January / 6);
    REQUIRE((f3.begin() + 1)->at(_1) == 14.4);
    REQUIRE((f3.begin() + 1)->at(_2) == true);
    REQUIRE((f3.begin() + 2)->at(_0) == 2022_y / January / 7);
    REQUIRE((f3.begin() + 2)->at(_1) == 15.5);
    REQUIRE((f3.begin() + 2)->at(_2) == false);
    REQUIRE((f3.begin() + 3)->at(_0) == 2022_y / January / 2);
    REQUIRE((f3.begin() + 3)->at(_1) == 10.0);
    REQUIRE((f3.begin() + 3)->at(_2) == false);
    REQUIRE((f3.begin() + 4)->at(_0) == 2022_y / January / 3);
    REQUIRE((f3.begin() + 4)->at(_1) == 11.1);
    REQUIRE((f3.begin() + 4)->at(_2) == true);
    REQUIRE((f3.begin() + 5)->at(_0) == 2022_y / January / 4);
    REQUIRE((f3.begin() + 5)->at(_1) == 12.2);
    REQUIRE((f3.begin() + 5)->at(_2) == false);
}

TEST_CASE("new_series()", "[frame]")
{
    SECTION("default")
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 2, 10.0, false);
        f1.push_back(2022_y / January / 3, 11.1, true);
        f1.push_back(2022_y / January / 4, 12.2, false);
        f1.push_back(2022_y / January / 5, 13.3, false);
        f1.push_back(2022_y / January / 6, 14.4, true);
        f1.push_back(2022_y / January / 7, 15.5, false);
        auto f2 = f1.new_series<int>("index");

        auto it = f2.begin();
        REQUIRE((*it)[_0] == 2022_y / January / 2);
        REQUIRE((*it)[_1] == 10.0);
        REQUIRE((*it)[_2] == false);
        REQUIRE((*it)[_3] == 0);
        it++;
        REQUIRE((*it)[_0] == 2022_y / January / 3);
        REQUIRE((*it)[_1] == 11.1);
        REQUIRE((*it)[_2] == true);
        REQUIRE((*it)[_3] == 0);
        it++;
        REQUIRE((*it)[_0] == 2022_y / January / 4);
        REQUIRE((*it)[_1] == 12.2);
        REQUIRE((*it)[_2] == false);
        REQUIRE((*it)[_3] == 0);
        it++;
        REQUIRE((*it)[_0] == 2022_y / January / 5);
        REQUIRE((*it)[_1] == 13.3);
        REQUIRE((*it)[_2] == false);
        REQUIRE((*it)[_3] == 0);
        it++;
        REQUIRE((*it)[_0] == 2022_y / January / 6);
        REQUIRE((*it)[_1] == 14.4);
        REQUIRE((*it)[_2] == true);
        REQUIRE((*it)[_3] == 0);
        it++;
        REQUIRE((*it)[_0] == 2022_y / January / 7);
        REQUIRE((*it)[_1] == 15.5);
        REQUIRE((*it)[_2] == false);
        REQUIRE((*it)[_3] == 0);
    }

    SECTION("expression")
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 2, 10.0, false);
        f1.push_back(2022_y / January / 3, 11.1, true);
        f1.push_back(2022_y / January / 4, 12.2, false);
        f1.push_back(2022_y / January / 5, 13.3, false);
        f1.push_back(2022_y / January / 6, 14.4, true);
        f1.push_back(2022_y / January / 7, 15.5, false);
        auto f2 = f1.new_series<year_month_day>("index", _0 + years(1));
        auto f3 = f1.new_series<year_month_day>("index", [](auto&, auto& c, auto&) {
            auto& col1val = c->at(_0);
            return col1val + years(1);
        });
        REQUIRE(f2 == f3);

        REQUIRE((f2.begin() + 0)->at(_0) == 2022_y / January / 2);
        REQUIRE((f2.begin() + 0)->at(_1) == 10.0);
        REQUIRE((f2.begin() + 0)->at(_2) == false);
        REQUIRE((f2.begin() + 0)->at(_3) == 2023_y / January / 2);

        REQUIRE((f2.begin() + 1)->at(_0) == 2022_y / January / 3);
        REQUIRE((f2.begin() + 1)->at(_1) == 11.1);
        REQUIRE((f2.begin() + 1)->at(_2) == true);
        REQUIRE((f2.begin() + 1)->at(_3) == 2023_y / January / 3);

        REQUIRE((f2.begin() + 2)->at(_0) == 2022_y / January / 4);
        REQUIRE((f2.begin() + 2)->at(_1) == 12.2);
        REQUIRE((f2.begin() + 2)->at(_2) == false);
        REQUIRE((f2.begin() + 2)->at(_3) == 2023_y / January / 4);

        REQUIRE((f2.begin() + 3)->at(_0) == 2022_y / January / 5);
        REQUIRE((f2.begin() + 3)->at(_1) == 13.3);
        REQUIRE((f2.begin() + 3)->at(_2) == false);
        REQUIRE((f2.begin() + 3)->at(_3) == 2023_y / January / 5);

        REQUIRE((f2.begin() + 4)->at(_0) == 2022_y / January / 6);
        REQUIRE((f2.begin() + 4)->at(_1) == 14.4);
        REQUIRE((f2.begin() + 4)->at(_2) == true);
        REQUIRE((f2.begin() + 4)->at(_3) == 2023_y / January / 6);

        REQUIRE((f2.begin() + 5)->at(_0) == 2022_y / January / 7);
        REQUIRE((f2.begin() + 5)->at(_1) == 15.5);
        REQUIRE((f2.begin() + 5)->at(_2) == false);
        REQUIRE((f2.begin() + 5)->at(_3) == 2023_y / January / 7);
    }
}

TEST_CASE("operator<<()", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 1, 8.9, false);
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, true);
    f1.push_back(2022_y / January / 4, 12.2, false);
    f1.push_back(2022_y / January / 5, 13.3, false);
    f1.push_back(2022_y / January / 6, 14.4, true);
    f1.push_back(2022_y / January / 7, 15.5, false);
    f1.push_back(2022_y / January / 8, 9.1, true);
    f1.push_back(2022_y / January / 9, 9.3, false);

    dout << f1;
    frame<year_month_day, double, bool> fcoldandrain =
        f1.rows(_1 <= 12 && _2 == true && _0 > 2022_y / January / 4);
    dout << fcoldandrain;
    auto f2 = f1.new_series<year_month_day>("next year", _0 + years(1));
    dout << f2;
    series<year_month_day> s = f1.column(_0);
    dout << s;
}

TEST_CASE("allow_missing()", "[frame]")
{
    SECTION("all")
    {
        frame<year_month_day, double, bool> f1;
        auto f2 = f1.allow_missing();
        f2.set_column_names("date", "temperature", "rain");
        f2.push_back(2022_y / January / 1, 8.9, false);
        f2.push_back(missing, 10.0, false);
        f2.push_back(2022_y / January / 3, missing, true);
        f2.push_back(2022_y / January / 4, 12.2, missing);
        f2.push_back(missing, missing, false);
        REQUIRE(f2.size() == 5);
    }

    SECTION("existing")
    {
        frame<year_month_day, mi<double>, bool> f1;
        auto f2 = f1.allow_missing();
        f2.set_column_names("date", "temperature", "rain");
        f2.push_back(2022_y / January / 1, 8.9, false);
        f2.push_back(missing, 10.0, false);
        f2.push_back(2022_y / January / 3, missing, true);
        f2.push_back(2022_y / January / 4, 12.2, missing);
        f2.push_back(missing, missing, false);
        REQUIRE(f2.size() == 5);
    }

    SECTION("contains")
    {
        REQUIRE(mf::detail::contains<0, 3, 0, 1, 2>::value == true);
        REQUIRE(mf::detail::contains<4, 3, 0, 1, 2>::value == false);
        REQUIRE(mf::detail::contains<4>::value == false);
        REQUIRE(mf::detail::contains<1, 3, 0, 1, 2>::value == true);
        REQUIRE(mf::detail::contains<2, 3, 0, 1, 2>::value == true);
        REQUIRE(mf::detail::contains<3, 3, 0, 1, 2>::value == true);
        REQUIRE(mf::detail::contains<4, 0, 1, 3, 2>::value == false);
        REQUIRE(mf::detail::contains<4, 0, 0, 0, 0>::value == false);
    }

    SECTION("some")
    {
        frame<year_month_day, double, bool> f1;
        auto f2 = f1.allow_missing(_0, _2);
        f2.set_column_names("date", "temperature", "rain");
        f2.push_back(2022_y / January / 1, 8.9, false);
        f2.push_back(missing, 10.0, false);
        f2.push_back(2022_y / January / 3, 11.1, true);
        f2.push_back(2022_y / January / 4, 12.2, missing);
        f2.push_back(missing, 13.3, false);
        REQUIRE(f2.size() == 5);
    }

    SECTION("some existing")
    {
        frame<year_month_day, double, mi<bool>> f1;
        auto f2 = f1.allow_missing(_0, _2);
        f2.set_column_names("date", "temperature", "rain");
        f2.push_back(2022_y / January / 1, 8.9, false);
        f2.push_back(missing, 10.0, false);
        f2.push_back(2022_y / January / 3, 11.1, true);
        f2.push_back(2022_y / January / 4, 12.2, missing);
        f2.push_back(missing, 13.3, false);
        REQUIRE(f2.size() == 5);
    }
}

TEST_CASE("disallow_missing()", "[frame]")
{
    SECTION("all")
    {
        frame<mi<year_month_day>, mi<double>, mi<bool>> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 1, missing, false);
        f1.push_back(missing, 10.0, false);
        f1.push_back(2022_y / January / 3, missing, true);
        f1.push_back(2022_y / January / 4, 12.2, missing);
        f1.push_back(missing, missing, false);
        auto f2 = f1.disallow_missing();
        REQUIRE(f2.size() == 5);
        auto b = f2.begin();
        REQUIRE(b->at(_0) == 2022_y / January / 1); // unchanged
        REQUIRE(b->at(_1) == 0.0); // default double
        REQUIRE(b->at(_2) == false); // unchanged
        b++;
        REQUIRE(b->at(_0) == year_month_day{}); // default date
        b++;
        REQUIRE(b->at(_1) == 0.0); // default date
        b++;
        REQUIRE(b->at(_2) == false); // default bool
        b++;
        REQUIRE(b->at(_0) == year_month_day{}); // default date
        REQUIRE(b->at(_1) == 0.0); // default date
    }

    SECTION("some")
    {
        auto f1 = frame<year_month_day, double, bool>{}.allow_missing();
        f1.push_back(2022_y / January / 1, missing, false);
        f1.push_back(missing, 10.0, false);
        f1.push_back(2022_y / January / 3, missing, true);
        f1.push_back(2022_y / January / 4, 12.2, missing);
        f1.push_back(missing, missing, false);
        auto f2 = f1.disallow_missing(_0, _2);
        REQUIRE(f2.size() == 5);
        auto b = f2.begin();
        REQUIRE(b->at(_0) == 2022_y / January / 1); // unchanged
        REQUIRE(b->at(_1) == missing); // unchanged
        REQUIRE(b->at(_2) == false); // unchanged
        b++;
        REQUIRE(b->at(_0) == year_month_day{}); // default date
        REQUIRE(b->at(_1) == 10.0); // unchanged
        REQUIRE(b->at(_2) == false); // unchanged
        b++;
        REQUIRE(b->at(_0) == 2022_y / January / 3); // unchanged
        REQUIRE(b->at(_1) == missing); // unchanged
        REQUIRE(b->at(_2) == true); // unchanged
        b++;
        REQUIRE(b->at(_0) == 2022_y / January / 4); // unchanged
        REQUIRE(b->at(_1) == 12.2); // unchanged
        REQUIRE(b->at(_2) == false); // default bool
        b++;
        REQUIRE(b->at(_0) == year_month_day{}); // default date
        REQUIRE(b->at(_1) == missing); // unchanged
        REQUIRE(b->at(_2) == false); // unchanged
    }
}

TEST_CASE("expression offsets", "[frame]")
{
    SECTION("initially allow missing")
    {
        frame<mi<year_month_day>, double, bool> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 1, 8.9, false);
        f1.push_back(2022_y / January / 2, 10.0, false);
        f1.push_back(2022_y / January / 3, 11.1, true);
        f1.push_back(2022_y / January / 4, 12.2, false);
        f1.push_back(2022_y / January / 5, 13.3, false);
        f1.push_back(2022_y / January / 6, 14.4, true);
        f1.push_back(2022_y / January / 7, 15.5, false);
        f1.push_back(2022_y / January / 8, 9.1, true);
        f1.push_back(2022_y / January / 9, 9.3, false);

        auto f2 = f1.new_series<mi<year_month_day>>("yesterday", _0[-1]);
        dout << f2;
        REQUIRE(f2.num_columns() == 4);
        auto b2 = f2.begin();
        REQUIRE((b2 + 0)->at(_3) == missing);
        REQUIRE((b2 + 1)->at(_3) == 2022_y / January / 1);
        REQUIRE((b2 + 2)->at(_3) == 2022_y / January / 2);
        REQUIRE((b2 + 8)->at(_3) == 2022_y / January / 8);

        auto f3 = f1.new_series<mi<year_month_day>>("tomorrow last year", _0[+1] - years(1));
        dout << f3;
        REQUIRE(f3.num_columns() == 4);
        auto b3 = f3.begin();
        REQUIRE((b3 + 0)->at(_3) == 2021_y / January / 2);
        REQUIRE((b3 + 1)->at(_3) == 2021_y / January / 3);
        REQUIRE((b3 + 7)->at(_3) == 2021_y / January / 9);
        REQUIRE((b3 + 8)->at(_3) == missing);
    }

    SECTION("initially disallow missing")
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 1, 8.9, false);
        f1.push_back(2022_y / January / 2, 10.0, false);
        f1.push_back(2022_y / January / 3, 11.1, true);
        f1.push_back(2022_y / January / 4, 12.2, false);
        f1.push_back(2022_y / January / 5, 13.3, false);
        f1.push_back(2022_y / January / 6, 14.4, true);
        f1.push_back(2022_y / January / 7, 15.5, false);
        f1.push_back(2022_y / January / 8, 9.1, true);
        f1.push_back(2022_y / January / 9, 9.3, false);

        auto f2 = f1.new_series<mi<year_month_day>>("yesterday", _0[-1]);
        dout << f2;
        REQUIRE(f2.num_columns() == 4);
        auto b2 = f2.begin();
        REQUIRE((b2 + 0)->at(_3) == missing);
        REQUIRE((b2 + 1)->at(_3) == 2022_y / January / 1);
        REQUIRE((b2 + 2)->at(_3) == 2022_y / January / 2);
        REQUIRE((b2 + 8)->at(_3) == 2022_y / January / 8);

        auto f3 = f1.new_series<mi<year_month_day>>("tomorrow last year", _0[+1] - years(1));
        dout << f3;
        REQUIRE(f3.num_columns() == 4);
        auto b3 = f3.begin();
        REQUIRE((b3 + 0)->at(_3) == 2021_y / January / 2);
        REQUIRE((b3 + 1)->at(_3) == 2021_y / January / 3);
        REQUIRE((b3 + 7)->at(_3) == 2021_y / January / 9);
        REQUIRE((b3 + 8)->at(_3) == missing);
    }

    SECTION("initially disallow missing into disallow missing")
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 1, 8.9, false);
        f1.push_back(2022_y / January / 2, 10.0, false);
        f1.push_back(2022_y / January / 3, 11.1, true);
        f1.push_back(2022_y / January / 4, 12.2, false);
        f1.push_back(2022_y / January / 5, 13.3, false);
        f1.push_back(2022_y / January / 6, 14.4, true);
        f1.push_back(2022_y / January / 7, 15.5, false);
        f1.push_back(2022_y / January / 8, 9.1, true);
        f1.push_back(2022_y / January / 9, 9.3, false);

        auto f2 = f1.new_series<year_month_day>("yesterday", _0[-1]);
        dout << f2;
        REQUIRE(f2.num_columns() == 4);
        auto b2 = f2.begin();
        REQUIRE((b2 + 0)->at(_3) == year_month_day{});
        REQUIRE((b2 + 1)->at(_3) == 2022_y / January / 1);
        REQUIRE((b2 + 2)->at(_3) == 2022_y / January / 2);
        REQUIRE((b2 + 8)->at(_3) == 2022_y / January / 8);

        auto f3 = f1.new_series<year_month_day>("tomorrow last year", _0[+1] - years(1));
        dout << f3;
        REQUIRE(f3.num_columns() == 4);
        auto b3 = f3.begin();
        REQUIRE((b3 + 0)->at(_3) == 2021_y / January / 2);
        REQUIRE((b3 + 1)->at(_3) == 2021_y / January / 3);
        REQUIRE((b3 + 7)->at(_3) == 2021_y / January / 9);
        REQUIRE((b3 + 8)->at(_3) == year_month_day{});
    }
}

TEST_CASE("drop_missing()", "[frame]")
{
    frame<mi<year_month_day>, mi<double>, mi<bool>> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 1, missing, false);
    f1.push_back(2022_y / January / 1, 10.0, false);
    f1.push_back(2022_y / January / 3, missing, true);
    f1.push_back(2022_y / January / 4, 12.2, missing);
    f1.push_back(2022_y / January / 5, 13.3, false);
    f1.push_back(missing, 14.4, true);
    f1.push_back(2022_y / January / 7, 15.5, false);
    f1.push_back(2022_y / January / 8, 9.1, true);
    f1.push_back(2022_y / January / 9, 9.3, missing);

    auto f2 = f1.drop_missing();
    dout << f1;
    dout << f2;
    REQUIRE(f2.size() == 4);
}

TEST_CASE("corr", "[frame]")
{
    SECTION("frame::mean")
    {
        frame<mi<year_month_day>, double, int> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 1, 8.9, 10);
        f1.push_back(2022_y / January / 2, 10.0, 10);
        f1.push_back(2022_y / January / 3, 11.1, 7);
        f1.push_back(2022_y / January / 4, 12.2, 10);
        f1.push_back(2022_y / January / 5, 13.3, 10);
        f1.push_back(2022_y / January / 6, 14.4, 7);
        f1.push_back(2022_y / January / 7, 15.5, 10);
        f1.push_back(2022_y / January / 8, 9.1, 7);
        f1.push_back(2022_y / January / 9, 9.3, 10);

        double mtemp = f1.mean(_1);
        double mrain = f1.mean(_2);

        REQUIRE(mtemp == Approx(11.533333));
        REQUIRE(mrain == Approx(9.0));
    }

    SECTION("detail::mean")
    {
        const auto NUMELEMS = 1000;
        double ad[NUMELEMS];
        float af[NUMELEMS];
        int ai[NUMELEMS];
        unsigned au[NUMELEMS];
        for (auto i = 0; i < NUMELEMS; ++i) {
            ad[i] = static_cast<double>(i + 1);
            af[i] = static_cast<float>(i + 1);
            ai[i] = static_cast<int>(i + 1);
            au[i] = static_cast<unsigned>(i + 1);
        }

        double adm = mf::detail::mean<double>(ad, NUMELEMS);
        REQUIRE(adm == Approx(500.5));
        double admv = mf::detail::mean(ad, NUMELEMS);
        REQUIRE(admv == Approx(500.5));

        float afm = mf::detail::mean<float>(af, NUMELEMS);
        REQUIRE(afm == Approx(500.5f));
        float afmv = mf::detail::mean(af, NUMELEMS);
        REQUIRE(afmv == Approx(500.5));

        int aim = mf::detail::mean(ai, NUMELEMS);
        REQUIRE(aim == Approx(500));
        int aum = mf::detail::mean(au, NUMELEMS);
        REQUIRE(aum == Approx(500U));
    }

    SECTION("detail::correlate_pearson")
    {
        const auto NUMELEMS = 20;
        double* adl         = new double[NUMELEMS];
        double* adr         = new double[NUMELEMS];
        float* afl          = new float[NUMELEMS];
        float* afr          = new float[NUMELEMS];
        for (auto i = 0; i < NUMELEMS; ++i) {
            adl[i] = static_cast<double>(i + 1);
            adr[i] = static_cast<double>(i + std::sin(i));
            afl[i] = static_cast<float>(i + 1);
            afr[i] = static_cast<float>(i + std::sin(i));
        }

        double doublcorr = mf::detail::correlate_pearson(adl, adr, NUMELEMS);
        float floatcorr  = mf::detail::correlate_pearson(afl, afr, NUMELEMS);

        REQUIRE(doublcorr == Approx(0.992916306));
        REQUIRE(floatcorr == Approx(0.992916f));

        delete[] adl;
        delete[] adr;
        delete[] afl;
        delete[] afr;
    }

    SECTION("frame::corr()")
    {
        frame<double, double> f1;
        f1.set_column_names("temperature", "rain");
        f1.push_back(1.0, 9.0);
        f1.push_back(2.0, 8.0);
        f1.push_back(3.0, 7.0);
        f1.push_back(4.0, 6.0);
        f1.push_back(5.0, 5.0);
        f1.push_back(6.0, 4.0);
        f1.push_back(7.0, 3.0);
        f1.push_back(8.0, 2.0);
        f1.push_back(9.0, 1.0);
        double corr = f1.corr(_0, _1);
        REQUIRE(corr == Approx(-1.0));
        f1.clear();
        f1.push_back(1.0, 1.0);
        f1.push_back(2.0, 2.0);
        f1.push_back(3.0, 3.0);
        f1.push_back(4.0, 4.0);
        f1.push_back(5.0, 5.0);
        f1.push_back(6.0, 6.0);
        f1.push_back(7.0, 7.0);
        f1.push_back(8.0, 8.0);
        f1.push_back(9.0, 9.0);
        corr = f1.corr(_0, _1);
        REQUIRE(corr == Approx(1.0));
    }

    SECTION("corr() with int")
    {
        frame<year_month_day, double, int> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 1, 8.9, 10);
        f1.push_back(2022_y / January / 2, 10.0, 10);
        f1.push_back(2022_y / January / 3, 11.1, 7);
        f1.push_back(2022_y / January / 4, 12.2, 10);
        f1.push_back(2022_y / January / 5, 13.3, 10);
        f1.push_back(2022_y / January / 6, 14.4, 7);
        f1.push_back(2022_y / January / 7, 15.5, 10);
        f1.push_back(2022_y / January / 8, 9.1, 7);
        f1.push_back(2022_y / January / 9, 9.3, 10);

        double corr1 = f1.corr(_2, _1);
        double corr2 = f1.corr(_1, _2);

        REQUIRE(corr1 == Approx(corr2));
    }
}

TEST_CASE("std::sort", "[frame]")
{
    SECTION("uniseries")
    {
        frame<year_month_day> f1;
        f1.set_column_names("date");

        f1.push_back(2022_y / January / 8);
        f1.push_back(2022_y / January / 9);
        f1.push_back(2022_y / January / 5);
        f1.push_back(2022_y / January / 6);
        f1.push_back(2022_y / January / 3);
        f1.push_back(2022_y / January / 4);
        f1.push_back(2022_y / January / 7);
        f1.push_back(2022_y / January / 1);
        f1.push_back(2022_y / January / 2);
        dout << f1;
        std::sort(f1.begin(), f1.end());
        dout << f1;
        auto it = f1.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / January / 1);
        REQUIRE((it + 1)->at(_0) == 2022_y / January / 2);
        REQUIRE((it + 2)->at(_0) == 2022_y / January / 3);
        REQUIRE((it + 3)->at(_0) == 2022_y / January / 4);
        REQUIRE((it + 4)->at(_0) == 2022_y / January / 5);
        REQUIRE((it + 5)->at(_0) == 2022_y / January / 6);
        REQUIRE((it + 6)->at(_0) == 2022_y / January / 7);
        REQUIRE((it + 7)->at(_0) == 2022_y / January / 8);
        REQUIRE((it + 8)->at(_0) == 2022_y / January / 9);
    }

    SECTION("multiseries")
    {
        frame<year_month_day, double, int> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 8, 9.1, 7);
        f1.push_back(2022_y / January / 9, 9.3, 10);
        f1.push_back(2022_y / January / 5, 13.3, 10);
        f1.push_back(2022_y / January / 6, 14.4, 7);
        f1.push_back(2022_y / January / 3, 11.1, 7);
        f1.push_back(2022_y / January / 4, 12.2, 10);
        f1.push_back(2022_y / January / 7, 15.5, 10);
        f1.push_back(2022_y / January / 1, 8.9, 10);
        f1.push_back(2022_y / January / 2, 10.0, 10);

        dout << f1;
        std::sort(f1.begin(), f1.end());
        dout << f1;

        auto it = f1.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / January / 1);
        REQUIRE((it + 1)->at(_0) == 2022_y / January / 2);
        REQUIRE((it + 2)->at(_0) == 2022_y / January / 3);
        REQUIRE((it + 3)->at(_0) == 2022_y / January / 4);
        REQUIRE((it + 4)->at(_0) == 2022_y / January / 5);
        REQUIRE((it + 5)->at(_0) == 2022_y / January / 6);
        REQUIRE((it + 6)->at(_0) == 2022_y / January / 7);
        REQUIRE((it + 7)->at(_0) == 2022_y / January / 8);
        REQUIRE((it + 8)->at(_0) == 2022_y / January / 9);

        REQUIRE((it + 0)->at(_1) == 8.9);
        REQUIRE((it + 1)->at(_1) == 10.0);
        REQUIRE((it + 2)->at(_1) == 11.1);
        REQUIRE((it + 3)->at(_1) == 12.2);
        REQUIRE((it + 4)->at(_1) == 13.3);
        REQUIRE((it + 5)->at(_1) == 14.4);
        REQUIRE((it + 6)->at(_1) == 15.5);
        REQUIRE((it + 7)->at(_1) == 9.1);
        REQUIRE((it + 8)->at(_1) == 9.3);

        REQUIRE((it + 0)->at(_2) == 10);
        REQUIRE((it + 1)->at(_2) == 10);
        REQUIRE((it + 2)->at(_2) == 7);
        REQUIRE((it + 3)->at(_2) == 10);
        REQUIRE((it + 4)->at(_2) == 10);
        REQUIRE((it + 5)->at(_2) == 7);
        REQUIRE((it + 6)->at(_2) == 10);
        REQUIRE((it + 7)->at(_2) == 7);
        REQUIRE((it + 8)->at(_2) == 10);
    }

    SECTION("Rearranged")
    {
        frame<year_month_day, double, int> f1;
        f1.set_column_names("date", "temperature", "rain");
        f1.push_back(2022_y / January / 8, 9.1, 7);
        f1.push_back(2022_y / January / 9, 9.3, 10);
        f1.push_back(2022_y / January / 5, 13.3, 10);
        f1.push_back(2022_y / January / 6, 14.4, 7);
        f1.push_back(2022_y / January / 3, 11.1, 7);
        f1.push_back(2022_y / January / 4, 12.2, 10);
        f1.push_back(2022_y / January / 7, 15.5, 10);
        f1.push_back(2022_y / January / 1, 8.9, 10);
        f1.push_back(2022_y / January / 2, 10.0, 10);
        auto f2 = f1.columns(_2, _1, _0);

        dout << f2;
        std::sort(f2.begin(), f2.end());
        dout << f2;

        // Should be:
        //  | rain | temperature |       date
        //__|______|_____________|____________
        // 0|    7 |         9.1 | 2022-01-08
        // 1|    7 |        11.1 | 2022-01-03
        // 2|    7 |        14.4 | 2022-01-06
        // 3|   10 |         8.9 | 2022-01-01
        // 4|   10 |         9.3 | 2022-01-09
        // 5|   10 |          10 | 2022-01-02
        // 6|   10 |        12.2 | 2022-01-04
        // 7|   10 |        13.3 | 2022-01-05
        // 8|   10 |        15.5 | 2022-01-07

        auto it = f2.cbegin();
        REQUIRE((it + 0)->at(_0) == 7);
        REQUIRE((it + 1)->at(_0) == 7);
        REQUIRE((it + 2)->at(_0) == 7);
        REQUIRE((it + 3)->at(_0) == 10);
        REQUIRE((it + 4)->at(_0) == 10);
        REQUIRE((it + 5)->at(_0) == 10);
        REQUIRE((it + 6)->at(_0) == 10);
        REQUIRE((it + 7)->at(_0) == 10);
        REQUIRE((it + 8)->at(_0) == 10);

        REQUIRE((it + 0)->at(_1) == 9.1);
        REQUIRE((it + 1)->at(_1) == 11.1);
        REQUIRE((it + 2)->at(_1) == 14.4);
        REQUIRE((it + 3)->at(_1) == 8.9);
        REQUIRE((it + 4)->at(_1) == 9.3);
        REQUIRE((it + 5)->at(_1) == 10.0);
        REQUIRE((it + 6)->at(_1) == 12.2);
        REQUIRE((it + 7)->at(_1) == 13.3);
        REQUIRE((it + 8)->at(_1) == 15.5);

        REQUIRE((it + 0)->at(_2) == 2022_y / January / 8);
        REQUIRE((it + 1)->at(_2) == 2022_y / January / 3);
        REQUIRE((it + 2)->at(_2) == 2022_y / January / 6);
        REQUIRE((it + 3)->at(_2) == 2022_y / January / 1);
        REQUIRE((it + 4)->at(_2) == 2022_y / January / 9);
        REQUIRE((it + 5)->at(_2) == 2022_y / January / 2);
        REQUIRE((it + 6)->at(_2) == 2022_y / January / 4);
        REQUIRE((it + 7)->at(_2) == 2022_y / January / 5);
        REQUIRE((it + 8)->at(_2) == 2022_y / January / 7);
    }
}

TEST_CASE("aggregate", "[frame]")
{
    frame<year_month_day, double, float, float> f1;
    f1.set_column_names("date", "temperature", "rain", "humidity");
    f1.push_back(2022_y / January / 1, 1.0, 0.0, -5.0);

    f1.push_back(2022_y / January / 2, 2.0, 1.0, -4.5);
    f1.push_back(2022_y / January / 2, 2.0, 2.0, -5.5);

    f1.push_back(2022_y / January / 3, 3.0, 2.0, -4.0);
    f1.push_back(2022_y / January / 3, 3.0, 3.0, -5.0);
    f1.push_back(2022_y / January / 3, 3.0, 4.0, -6.0);

    f1.push_back(2022_y / January / 4, 4.0, 3.0, -4.0);
    f1.push_back(2022_y / January / 4, 4.0, 4.0, -5.0);
    f1.push_back(2022_y / January / 4, 4.0, 5.0, -6.0);

    f1.push_back(2022_y / January / 5, 5.0, 4.0, -4.0);
    f1.push_back(2022_y / January / 5, 5.0, 5.0, -5.0);
    f1.push_back(2022_y / January / 5, 5.0, 6.0, -6.0);

    f1.push_back(2022_y / January / 6, 6.0, 5.0, -4.0);
    f1.push_back(2022_y / January / 6, 6.0, 6.0, -5.0);
    f1.push_back(2022_y / January / 6, 6.0, 7.0, -6.0);

    f1.push_back(2022_y / January / 7, 7.0, 6.0, -4.0);
    f1.push_back(2022_y / January / 7, 7.0, 7.0, -5.0);
    f1.push_back(2022_y / January / 7, 7.0, 8.0, -6.0);

    f1.push_back(2022_y / January / 8, 8.0, 7.0, -4.5);
    f1.push_back(2022_y / January / 8, 8.0, 8.0, -5.5);

    f1.push_back(2022_y / January / 9, 9.0, 9.0, -5.0);
    dout << "f1:\n";
    dout << f1;

    auto gf     = f1.groupby(_0, _1);

    // start with groupby( _5, _6 ).aggregate( sum( _1 ), sum( _2 ), count(), min( _4 ), sum( _3 ), sum( _1 ), max( _1 ) )
    using F = frame<char, short, int, long, float, double>;
    using RCFA = typename mf::detail::get_result_columns_from_args<F, 
          decltype(mf::agg::sum(_0)),
          decltype(mf::agg::sum(_1)),
          decltype(mf::agg::count()),
          decltype(mf::agg::min(_3)),
          decltype(mf::agg::sum(_2)),
          decltype(mf::agg::sum(_0)),
          decltype(mf::agg::max(_0))
          >::type;
    RCFA rcfa;
    (void)rcfa;
    //TD<RCFA> foo;

    using AF = typename mf::detail::get_aggregate_frame<F, index_defn<4, 5>,
          decltype(mf::agg::sum(_0)),
          decltype(mf::agg::sum(_1)),
          decltype(mf::agg::count()),
          decltype(mf::agg::min(_3)),
          decltype(mf::agg::sum(_2)),
          decltype(mf::agg::sum(_0)),
          decltype(mf::agg::max(_0))
          >::type;
    AF af;
    (void)af;
    //TD<AF> foo4;
    using TUP = tuple<
          decltype(mf::agg::sum(_0)),
          decltype(mf::agg::sum(_1)),
          decltype(mf::agg::count()),
          decltype(mf::agg::min(_3)),
          decltype(mf::agg::sum(_2)),
          decltype(mf::agg::sum(_0)),
          decltype(mf::agg::max(_0))
              >;
    TUP tup;
    (void)tup;
    //TD<TUP> foo6;
    REQUIRE(mf::detail::is_colind_at_argind<0, 0, TUP>::value == true);
    REQUIRE(mf::detail::is_colind_at_argind<1, 1, TUP>::value == true);
    REQUIRE(mf::detail::is_colind_at_argind<3, 3, TUP>::value == true);
    REQUIRE(mf::detail::is_colind_at_argind<2, 4, TUP>::value == true);
    REQUIRE(mf::detail::is_colind_at_argind<0, 5, TUP>::value == true);
    REQUIRE(mf::detail::is_colind_at_argind<0, 6, TUP>::value == true);

    REQUIRE(mf::detail::is_colind_at_argind<1, 0, TUP>::value == false);
    REQUIRE(mf::detail::is_colind_at_argind<2, 1, TUP>::value == false);
    REQUIRE(mf::detail::is_colind_at_argind<4, 3, TUP>::value == false);
    REQUIRE(mf::detail::is_colind_at_argind<3, 4, TUP>::value == false);
    REQUIRE(mf::detail::is_colind_at_argind<1, 5, TUP>::value == false);
    REQUIRE(mf::detail::is_colind_at_argind<1, 6, TUP>::value == false);

    REQUIRE(mf::detail::is_colind_at_argind<0, 2, TUP>::value == false);
    REQUIRE(mf::detail::is_colind_at_argind<1, 2, TUP>::value == false);
    REQUIRE(mf::detail::is_colind_at_argind<2, 2, TUP>::value == false);
    REQUIRE(mf::detail::is_colind_at_argind<3, 2, TUP>::value == false);
    REQUIRE(mf::detail::is_colind_at_argind<4, 2, TUP>::value == false);
    REQUIRE(mf::detail::is_colind_at_argind<5, 2, TUP>::value == false);
    REQUIRE(mf::detail::is_colind_at_argind<6, 2, TUP>::value == false);

    gf.build_index();
    auto f2 = gf.aggregate( 
        agg::sum( _2 ), 
        agg::min( _2 ), 
        agg::max( _2 ), 
        agg::mean( _2 ), 
        agg::stddev( _2 ) );
    dout << f2;
    auto f3 = gf.aggregate( 
        agg::sum( _3 ), 
        agg::min( _3 ), 
        agg::max( _3 ), 
        agg::mean( _3 ), 
        agg::stddev( _3 ) );
    dout << f3;
    std::sort( f2.begin(), f2.end() );
    std::sort( f3.begin(), f3.end() );

    // rain
    {
        auto it = f2.cbegin();
        // date
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 2);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 4);
        REQUIRE((it + 4)->at(_0) == 2022_y / 1 / 5);
        REQUIRE((it + 5)->at(_0) == 2022_y / 1 / 6);
        REQUIRE((it + 6)->at(_0) == 2022_y / 1 / 7);
        REQUIRE((it + 7)->at(_0) == 2022_y / 1 / 8);
        REQUIRE((it + 8)->at(_0) == 2022_y / 1 / 9);

        // temperature
        REQUIRE((it + 0)->at(_1) == 1);
        REQUIRE((it + 1)->at(_1) == 2);
        REQUIRE((it + 2)->at(_1) == 3);
        REQUIRE((it + 3)->at(_1) == 4);
        REQUIRE((it + 4)->at(_1) == 5);
        REQUIRE((it + 5)->at(_1) == 6);
        REQUIRE((it + 6)->at(_1) == 7);
        REQUIRE((it + 7)->at(_1) == 8);
        REQUIRE((it + 8)->at(_1) == 9);

        // sum( rain )
        REQUIRE((it + 0)->at(_2) == 0);
        REQUIRE((it + 1)->at(_2) == 3);
        REQUIRE((it + 2)->at(_2) == 9);
        REQUIRE((it + 3)->at(_2) == 12);
        REQUIRE((it + 4)->at(_2) == 15);
        REQUIRE((it + 5)->at(_2) == 18);
        REQUIRE((it + 6)->at(_2) == 21);
        REQUIRE((it + 7)->at(_2) == 15);
        REQUIRE((it + 8)->at(_2) == 9);
 
        // min( rain )
        REQUIRE((it + 0)->at(_3) == 0);
        REQUIRE((it + 1)->at(_3) == 1);
        REQUIRE((it + 2)->at(_3) == 2);
        REQUIRE((it + 3)->at(_3) == 3);
        REQUIRE((it + 4)->at(_3) == 4);
        REQUIRE((it + 5)->at(_3) == 5);
        REQUIRE((it + 6)->at(_3) == 6);
        REQUIRE((it + 7)->at(_3) == 7);
        REQUIRE((it + 8)->at(_3) == 9);

        // max( rain )
        REQUIRE((it + 0)->at(_4) == 0);
        REQUIRE((it + 1)->at(_4) == 2);
        REQUIRE((it + 2)->at(_4) == 4);
        REQUIRE((it + 3)->at(_4) == 5);
        REQUIRE((it + 4)->at(_4) == 6);
        REQUIRE((it + 5)->at(_4) == 7);
        REQUIRE((it + 6)->at(_4) == 8);
        REQUIRE((it + 7)->at(_4) == 8);
        REQUIRE((it + 8)->at(_4) == 9);
        
        // mean( rain )
        REQUIRE((it + 0)->at(_5) == 0);
        REQUIRE((it + 1)->at(_5) == 1.5);
        REQUIRE((it + 2)->at(_5) == 3);
        REQUIRE((it + 3)->at(_5) == 4);
        REQUIRE((it + 4)->at(_5) == 5);
        REQUIRE((it + 5)->at(_5) == 6);
        REQUIRE((it + 6)->at(_5) == 7);
        REQUIRE((it + 7)->at(_5) == 7.5);
        REQUIRE((it + 8)->at(_5) == 9);
        
        // stddev( rain )
        REQUIRE((it + 0)->at(_6) == Approx(0));
        REQUIRE((it + 1)->at(_6) == Approx(0.5));
        REQUIRE((it + 2)->at(_6) == Approx(0.816497));
        REQUIRE((it + 3)->at(_6) == Approx(0.816497));
        REQUIRE((it + 4)->at(_6) == Approx(0.816497));
        REQUIRE((it + 5)->at(_6) == Approx(0.816497));
        REQUIRE((it + 6)->at(_6) == Approx(0.816497));
        REQUIRE((it + 7)->at(_6) == Approx(0.5));
        REQUIRE((it + 8)->at(_6) == Approx(0));
    }


    // humidity
    {
        auto it = f3.cbegin();
        // date
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 2);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 4);
        REQUIRE((it + 4)->at(_0) == 2022_y / 1 / 5);
        REQUIRE((it + 5)->at(_0) == 2022_y / 1 / 6);
        REQUIRE((it + 6)->at(_0) == 2022_y / 1 / 7);
        REQUIRE((it + 7)->at(_0) == 2022_y / 1 / 8);
        REQUIRE((it + 8)->at(_0) == 2022_y / 1 / 9);

        // temperature
        REQUIRE((it + 0)->at(_1) == 1);
        REQUIRE((it + 1)->at(_1) == 2);
        REQUIRE((it + 2)->at(_1) == 3);
        REQUIRE((it + 3)->at(_1) == 4);
        REQUIRE((it + 4)->at(_1) == 5);
        REQUIRE((it + 5)->at(_1) == 6);
        REQUIRE((it + 6)->at(_1) == 7);
        REQUIRE((it + 7)->at(_1) == 8);
        REQUIRE((it + 8)->at(_1) == 9);

        // sum( humidity )
        REQUIRE((it + 0)->at(_2) == -5);
        REQUIRE((it + 1)->at(_2) == -10);
        REQUIRE((it + 2)->at(_2) == -15);
        REQUIRE((it + 3)->at(_2) == -15);
        REQUIRE((it + 4)->at(_2) == -15);
        REQUIRE((it + 5)->at(_2) == -15);
        REQUIRE((it + 6)->at(_2) == -15);
        REQUIRE((it + 7)->at(_2) == -10);
        REQUIRE((it + 8)->at(_2) == -5);
 
        // min( humidity )
        REQUIRE((it + 0)->at(_3) == -5);
        REQUIRE((it + 1)->at(_3) == -5.5);
        REQUIRE((it + 2)->at(_3) == -6);
        REQUIRE((it + 3)->at(_3) == -6);
        REQUIRE((it + 4)->at(_3) == -6);
        REQUIRE((it + 5)->at(_3) == -6);
        REQUIRE((it + 6)->at(_3) == -6);
        REQUIRE((it + 7)->at(_3) == -5.5);
        REQUIRE((it + 8)->at(_3) == -5);

        // max( humidity )
        REQUIRE((it + 0)->at(_4) == -5);
        REQUIRE((it + 1)->at(_4) == -4.5);
        REQUIRE((it + 2)->at(_4) == -4);
        REQUIRE((it + 3)->at(_4) == -4);
        REQUIRE((it + 4)->at(_4) == -4);
        REQUIRE((it + 5)->at(_4) == -4);
        REQUIRE((it + 6)->at(_4) == -4);
        REQUIRE((it + 7)->at(_4) == -4.5);
        REQUIRE((it + 8)->at(_4) == -5);
        
        // mean( humidity )
        REQUIRE((it + 0)->at(_5) == -5);
        REQUIRE((it + 1)->at(_5) == -5);
        REQUIRE((it + 2)->at(_5) == -5);
        REQUIRE((it + 3)->at(_5) == -5);
        REQUIRE((it + 4)->at(_5) == -5);
        REQUIRE((it + 5)->at(_5) == -5);
        REQUIRE((it + 6)->at(_5) == -5);
        REQUIRE((it + 7)->at(_5) == -5);
        REQUIRE((it + 8)->at(_5) == -5);
        
        // stddev( humidity )
        REQUIRE((it + 0)->at(_6) == Approx(0));
        REQUIRE((it + 1)->at(_6) == Approx(0.5));
        REQUIRE((it + 2)->at(_6) == Approx(0.816497));
        REQUIRE((it + 3)->at(_6) == Approx(0.816497));
        REQUIRE((it + 4)->at(_6) == Approx(0.816497));
        REQUIRE((it + 5)->at(_6) == Approx(0.816497));
        REQUIRE((it + 6)->at(_6) == Approx(0.816497));
        REQUIRE((it + 7)->at(_6) == Approx(0.5));
        REQUIRE((it + 8)->at(_6) == Approx(0));
    }

}
