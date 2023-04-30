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
#include "mainframe.hpp"

using namespace std;
using namespace std::chrono;
using namespace std::literals;
using namespace date::literals;
using namespace date;
using namespace mf;
using namespace mf::placeholders;
using mf::function::real;
using mf::function::imag;
using mf::function::cmplx;


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

    columnindex<2> ci;
    auto frain             = f1.rows(ci == true);
    auto fnorain           = f1.rows(false == ci);
    auto fhot              = f1.rows(_1 >= 14);
    auto fhotandrain       = f1.rows(col1 >= 13 && col2 == true);
    auto fcoldandrain      = f1.rows(_1 <= 12 && _2 == true && _0 > 2022_y / January / 4);
    auto f2or3orhotandrain = f1.rows(
        (_1 >= 13 && _2 == true) || _0 == 2022_y / January / 2 || _0 == 2022_y / January / 3);
    auto feven      = f1.rows(rownum % 2 == 0);
    auto ffirsthalf = f1.rows(rownum < 3);
    auto flasthalf  = f1.rows((framelen - rownum) <= 3);
    dout << "f1:\n";
    dout << f1;
    dout << "even:\n";
    dout << feven;
    dout << "first half:\n";
    dout << ffirsthalf;
    dout << "last half:\n";
    dout << flasthalf;

    REQUIRE(frain.size() == 2);
    REQUIRE(fnorain.size() == 4);
    REQUIRE(fhot.size() == 2);
    REQUIRE(fhotandrain.size() == 1);
    REQUIRE(fcoldandrain.empty());
    REQUIRE(f2or3orhotandrain.size() == 3);
    REQUIRE(feven.size() == 3);
    REQUIRE(ffirsthalf.size() == 3);
    REQUIRE(flasthalf.size() == 3);

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

    REQUIRE((feven.begin() + 0)->at(_0) == 2022_y / January / 2);
    REQUIRE((feven.begin() + 0)->at(_1) == 10.0);
    REQUIRE((feven.begin() + 0)->at(_2) == false);
    REQUIRE((feven.begin() + 1)->at(_0) == 2022_y / January / 4);
    REQUIRE((feven.begin() + 1)->at(_1) == 12.2);
    REQUIRE((feven.begin() + 1)->at(_2) == false);
    REQUIRE((feven.begin() + 2)->at(_0) == 2022_y / January / 6);
    REQUIRE((feven.begin() + 2)->at(_1) == 14.4);
    REQUIRE((feven.begin() + 2)->at(_2) == true);

    REQUIRE((ffirsthalf.begin() + 0)->at(_0) == 2022_y / January / 2);
    REQUIRE((ffirsthalf.begin() + 0)->at(_1) == 10.0);
    REQUIRE((ffirsthalf.begin() + 0)->at(_2) == false);
    REQUIRE((ffirsthalf.begin() + 1)->at(_0) == 2022_y / January / 3);
    REQUIRE((ffirsthalf.begin() + 1)->at(_1) == 11.1);
    REQUIRE((ffirsthalf.begin() + 1)->at(_2) == true);
    REQUIRE((ffirsthalf.begin() + 2)->at(_0) == 2022_y / January / 4);
    REQUIRE((ffirsthalf.begin() + 2)->at(_1) == 12.2);
    REQUIRE((ffirsthalf.begin() + 2)->at(_2) == false);

    REQUIRE((flasthalf.begin() + 0)->at(_0) == 2022_y / January / 5);
    REQUIRE((flasthalf.begin() + 0)->at(_1) == 13.3);
    REQUIRE((flasthalf.begin() + 0)->at(_2) == false);
    REQUIRE((flasthalf.begin() + 1)->at(_0) == 2022_y / January / 6);
    REQUIRE((flasthalf.begin() + 1)->at(_1) == 14.4);
    REQUIRE((flasthalf.begin() + 1)->at(_2) == true);
    REQUIRE((flasthalf.begin() + 2)->at(_0) == 2022_y / January / 7);
    REQUIRE((flasthalf.begin() + 2)->at(_1) == 15.5);
    REQUIRE((flasthalf.begin() + 2)->at(_2) == false);
}

TEST_CASE("operator,", "[frame]")
{
    columnindex<1> c1;
    columnindex<2> c2;
    columnindex<3> c3;
    columnindex<4> c4;
    auto c123   = c1.operator,(c2).operator,(c3);
    auto c1234  = c123.operator,(c4);
    auto c1234p = (c123, c4);
    (void)c1234;
    (void)c1234p;
    auto d1234 = (c1, c2, c3, c4);
    (void)d1234;
}

TEST_CASE("operator[columnindex<Inds>...]", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, false);
    f1.push_back(2022_y / January / 4, 12.2, false);

    SECTION("2 Reverse")
    {
        auto f3 = f1[_2, _0];
        REQUIRE(f3.column_name<0>() == "rain");
        REQUIRE(f3.column_name<1>() == "date");
        REQUIRE(f3.begin()->at(_0) == false);
        REQUIRE(f3.begin()->at(_1) == 2022_y / January / 2);
    }

    SECTION("1")
    {
        auto f3 = f1[_2];
        REQUIRE(f3.column_name<0>() == "rain");
        REQUIRE(f3.begin()->at(_0) == false);
    }
}

TEST_CASE("operator[size_t]", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, true);
    f1.push_back(2022_y / January / 4, 12.2, false);

    auto f2 = f1[1];
    REQUIRE(f2.size() == 1);
    auto it = f2.begin();

    REQUIRE(it->at(_0) == 2022_y / January / 3);
    REQUIRE(it->at(_1) == 11.1);
    REQUIRE(it->at(_2) == true);
}

TEST_CASE("operator[Ex]", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, true);
    f1.push_back(2022_y / January / 4, 12.2, false);
    f1.push_back(2022_y / January / 5, 13.3, false);
    f1.push_back(2022_y / January / 6, 14.4, true);
    f1.push_back(2022_y / January / 7, 15.5, false);

    columnindex<2> ci;
    auto frain        = f1[ci == true];
    auto fnorain      = f1[false == ci];
    auto fhot         = f1[_1 >= 14];
    auto fhotandrain  = f1[col1 >= 13 && col2 == true];
    auto fcoldandrain = f1[_1 <= 12 && _2 == true && _0 > 2022_y / January / 4];
    auto f2or3orhotandrain =
        f1[(_1 >= 13 && _2 == true) || _0 == 2022_y / January / 2 || _0 == 2022_y / January / 3];
    auto feven      = f1[rownum % 2 == 0];
    auto ffirsthalf = f1[rownum < 3];
    auto flasthalf  = f1[(framelen - rownum) <= 3];
    dout << "f1:\n";
    dout << f1;
    dout << "even:\n";
    dout << feven;
    dout << "first half:\n";
    dout << ffirsthalf;
    dout << "last half:\n";
    dout << flasthalf;

    REQUIRE(frain.size() == 2);
    REQUIRE(fnorain.size() == 4);
    REQUIRE(fhot.size() == 2);
    REQUIRE(fhotandrain.size() == 1);
    REQUIRE(fcoldandrain.empty());
    REQUIRE(f2or3orhotandrain.size() == 3);
    REQUIRE(feven.size() == 3);
    REQUIRE(ffirsthalf.size() == 3);
    REQUIRE(flasthalf.size() == 3);

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

    REQUIRE((feven.begin() + 0)->at(_0) == 2022_y / January / 2);
    REQUIRE((feven.begin() + 0)->at(_1) == 10.0);
    REQUIRE((feven.begin() + 0)->at(_2) == false);
    REQUIRE((feven.begin() + 1)->at(_0) == 2022_y / January / 4);
    REQUIRE((feven.begin() + 1)->at(_1) == 12.2);
    REQUIRE((feven.begin() + 1)->at(_2) == false);
    REQUIRE((feven.begin() + 2)->at(_0) == 2022_y / January / 6);
    REQUIRE((feven.begin() + 2)->at(_1) == 14.4);
    REQUIRE((feven.begin() + 2)->at(_2) == true);

    REQUIRE((ffirsthalf.begin() + 0)->at(_0) == 2022_y / January / 2);
    REQUIRE((ffirsthalf.begin() + 0)->at(_1) == 10.0);
    REQUIRE((ffirsthalf.begin() + 0)->at(_2) == false);
    REQUIRE((ffirsthalf.begin() + 1)->at(_0) == 2022_y / January / 3);
    REQUIRE((ffirsthalf.begin() + 1)->at(_1) == 11.1);
    REQUIRE((ffirsthalf.begin() + 1)->at(_2) == true);
    REQUIRE((ffirsthalf.begin() + 2)->at(_0) == 2022_y / January / 4);
    REQUIRE((ffirsthalf.begin() + 2)->at(_1) == 12.2);
    REQUIRE((ffirsthalf.begin() + 2)->at(_2) == false);

    REQUIRE((flasthalf.begin() + 0)->at(_0) == 2022_y / January / 5);
    REQUIRE((flasthalf.begin() + 0)->at(_1) == 13.3);
    REQUIRE((flasthalf.begin() + 0)->at(_2) == false);
    REQUIRE((flasthalf.begin() + 1)->at(_0) == 2022_y / January / 6);
    REQUIRE((flasthalf.begin() + 1)->at(_1) == 14.4);
    REQUIRE((flasthalf.begin() + 1)->at(_2) == true);
    REQUIRE((flasthalf.begin() + 2)->at(_0) == 2022_y / January / 7);
    REQUIRE((flasthalf.begin() + 2)->at(_1) == 15.5);
    REQUIRE((flasthalf.begin() + 2)->at(_2) == false);
}

TEST_CASE("row and column indexers combined", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, true);
    f1.push_back(2022_y / January / 4, 12.2, false);
    f1.push_back(2022_y / January / 5, 13.3, false);
    f1.push_back(2022_y / January / 6, 14.4, true);
    f1.push_back(2022_y / January / 7, 15.5, false);

    SECTION("row then column")
    {
        auto f2 = f1[_1 > 12.2][_0, _2];
        REQUIRE((f2.begin() + 0)->at(_0) == 2022_y / January / 5);
        REQUIRE((f2.begin() + 0)->at(_1) == false);
        REQUIRE((f2.begin() + 1)->at(_0) == 2022_y / January / 6);
        REQUIRE((f2.begin() + 1)->at(_1) == true);
        REQUIRE((f2.begin() + 2)->at(_0) == 2022_y / January / 7);
        REQUIRE((f2.begin() + 2)->at(_1) == false);
    }

    SECTION("column then row")
    {
        auto f2 = f1[_2, _1, _0][_1 > 12.2];
        REQUIRE((f2.begin() + 0)->at(_2) == 2022_y / January / 5);
        REQUIRE((f2.begin() + 0)->at(_0) == false);
        REQUIRE((f2.begin() + 0)->at(_1) == 13.3);
        REQUIRE((f2.begin() + 1)->at(_2) == 2022_y / January / 6);
        REQUIRE((f2.begin() + 1)->at(_0) == true);
        REQUIRE((f2.begin() + 1)->at(_1) == 14.4);
        REQUIRE((f2.begin() + 2)->at(_2) == 2022_y / January / 7);
        REQUIRE((f2.begin() + 2)->at(_0) == false);
        REQUIRE((f2.begin() + 2)->at(_1) == 15.5);
    }
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

TEST_CASE("append_column()", "[frame]")
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
        auto f2 = f1.append_column<int>("index");

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
        auto f2 = f1.append_column<year_month_day>("index", _0 + years(1));
        auto f3 = f1.append_column<year_month_day>("index", [](auto&, auto& c, auto&) {
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

TEST_CASE("make_series()", "[frame]")
{
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
        auto s2 = f1.make_series<year_month_day>("index", _0 + years(1));
        auto s3 = f1.make_series<year_month_day>("index", 
            [](auto&, auto& c, auto&) {
                auto& col1val = c->at(_0);
                return col1val + years(1);
            });
        REQUIRE(s2 == s3);

        REQUIRE(s2[0] == 2023_y / January / 2);
        REQUIRE(s2[1] == 2023_y / January / 3);
        REQUIRE(s2[2] == 2023_y / January / 4);
        REQUIRE(s2[3] == 2023_y / January / 5);
        REQUIRE(s2[4] == 2023_y / January / 6);
        REQUIRE(s2[5] == 2023_y / January / 7);
    }
}

template<typename T>
class TD;

TEST_CASE("append_series()", "[frame]")
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
        series<double> s1{30.0, 31.1, 32.2, 33.3, 34.4};
        s1.set_name("s1");
        series<int> s2{0, 1, 2, 3, 4, 5, 6};
        s2.set_name("s2");

        auto f2 = f1.append_series(s1);
        auto f3 = f2.append_series(s2);

        auto it = f3.begin();
        REQUIRE((*it)[_0] == 2022_y / January / 2);
        REQUIRE((*it)[_1] == 10.0);
        REQUIRE((*it)[_2] == false);
        REQUIRE((*it)[_3] == 30.0);
        REQUIRE((*it)[_4] == 0);
        it++;
        REQUIRE((*it)[_0] == 2022_y / January / 3);
        REQUIRE((*it)[_1] == 11.1);
        REQUIRE((*it)[_2] == true);
        REQUIRE((*it)[_3] == 31.1);
        REQUIRE((*it)[_4] == 1);
        it++;
        REQUIRE((*it)[_0] == 2022_y / January / 4);
        REQUIRE((*it)[_1] == 12.2);
        REQUIRE((*it)[_2] == false);
        REQUIRE((*it)[_3] == 32.2);
        REQUIRE((*it)[_4] == 2);
        it++;
        REQUIRE((*it)[_0] == 2022_y / January / 5);
        REQUIRE((*it)[_1] == 13.3);
        REQUIRE((*it)[_2] == false);
        REQUIRE((*it)[_3] == 33.3);
        REQUIRE((*it)[_4] == 3);
        it++;
        REQUIRE((*it)[_0] == 2022_y / January / 6);
        REQUIRE((*it)[_1] == 14.4);
        REQUIRE((*it)[_2] == true);
        REQUIRE((*it)[_3] == 34.4);
        REQUIRE((*it)[_4] == 4);
        it++;
        REQUIRE((*it)[_0] == 2022_y / January / 7);
        REQUIRE((*it)[_1] == 15.5);
        REQUIRE((*it)[_2] == false);
        REQUIRE((*it)[_3] == 0.0);
        REQUIRE((*it)[_4] == 5);
        it++;
        REQUIRE((*it)[_0] == year_month_day{});
        REQUIRE((*it)[_1] == 0.0);
        REQUIRE((*it)[_2] == false);
        REQUIRE((*it)[_3] == 0.0);
        REQUIRE((*it)[_4] == 6);
    }
}

TEST_CASE("prepend_column()", "[frame]")
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
        auto f2 = f1.prepend_column<int>("index");

        auto it = f2.begin();
        REQUIRE((*it)[_1] == 2022_y / January / 2);
        REQUIRE((*it)[_2] == 10.0);
        REQUIRE((*it)[_3] == false);
        REQUIRE((*it)[_0] == 0);
        it++;
        REQUIRE((*it)[_1] == 2022_y / January / 3);
        REQUIRE((*it)[_2] == 11.1);
        REQUIRE((*it)[_3] == true);
        REQUIRE((*it)[_0] == 0);
        it++;
        REQUIRE((*it)[_1] == 2022_y / January / 4);
        REQUIRE((*it)[_2] == 12.2);
        REQUIRE((*it)[_3] == false);
        REQUIRE((*it)[_0] == 0);
        it++;
        REQUIRE((*it)[_1] == 2022_y / January / 5);
        REQUIRE((*it)[_2] == 13.3);
        REQUIRE((*it)[_3] == false);
        REQUIRE((*it)[_0] == 0);
        it++;
        REQUIRE((*it)[_1] == 2022_y / January / 6);
        REQUIRE((*it)[_2] == 14.4);
        REQUIRE((*it)[_3] == true);
        REQUIRE((*it)[_0] == 0);
        it++;
        REQUIRE((*it)[_1] == 2022_y / January / 7);
        REQUIRE((*it)[_2] == 15.5);
        REQUIRE((*it)[_3] == false);
        REQUIRE((*it)[_0] == 0);
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
        auto f2 = f1.prepend_column<year_month_day>("index", _0 + years(1));
        auto f3 = f1.prepend_column<year_month_day>("index", [](auto&, auto& c, auto&) {
            auto& col1val = c->at(_0);
            return col1val + years(1);
        });
        REQUIRE(f2 == f3);

        REQUIRE((f2.begin() + 0)->at(_1) == 2022_y / January / 2);
        REQUIRE((f2.begin() + 0)->at(_2) == 10.0);
        REQUIRE((f2.begin() + 0)->at(_3) == false);
        REQUIRE((f2.begin() + 0)->at(_0) == 2023_y / January / 2);
                                       
        REQUIRE((f2.begin() + 1)->at(_1) == 2022_y / January / 3);
        REQUIRE((f2.begin() + 1)->at(_2) == 11.1);
        REQUIRE((f2.begin() + 1)->at(_3) == true);
        REQUIRE((f2.begin() + 1)->at(_0) == 2023_y / January / 3);
                                       
        REQUIRE((f2.begin() + 2)->at(_1) == 2022_y / January / 4);
        REQUIRE((f2.begin() + 2)->at(_2) == 12.2);
        REQUIRE((f2.begin() + 2)->at(_3) == false);
        REQUIRE((f2.begin() + 2)->at(_0) == 2023_y / January / 4);
                                       
        REQUIRE((f2.begin() + 3)->at(_1) == 2022_y / January / 5);
        REQUIRE((f2.begin() + 3)->at(_2) == 13.3);
        REQUIRE((f2.begin() + 3)->at(_3) == false);
        REQUIRE((f2.begin() + 3)->at(_0) == 2023_y / January / 5);
                                       
        REQUIRE((f2.begin() + 4)->at(_1) == 2022_y / January / 6);
        REQUIRE((f2.begin() + 4)->at(_2) == 14.4);
        REQUIRE((f2.begin() + 4)->at(_3) == true);
        REQUIRE((f2.begin() + 4)->at(_0) == 2023_y / January / 6);

        REQUIRE((f2.begin() + 5)->at(_1) == 2022_y / January / 7);
        REQUIRE((f2.begin() + 5)->at(_2) == 15.5);
        REQUIRE((f2.begin() + 5)->at(_3) == false);
        REQUIRE((f2.begin() + 5)->at(_0) == 2023_y / January / 7);
    }
}

TEST_CASE("prepend_series()", "[frame]")
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
        series<double> s1{30.0, 31.1, 32.2, 33.3, 34.4};
        s1.set_name("s1");
        series<int> s2{0, 1, 2, 3, 4, 5, 6};
        s2.set_name("s2");

        auto f2 = f1.prepend_series(s1);
        auto f3 = f2.prepend_series(s2);

        auto it = f3.begin();
        REQUIRE((*it)[_0] == 0);
        REQUIRE((*it)[_1] == 30.0);
        REQUIRE((*it)[_2] == 2022_y / January / 2);
        REQUIRE((*it)[_3] == 10.0);
        REQUIRE((*it)[_4] == false);
        it++;
        REQUIRE((*it)[_0] == 1);
        REQUIRE((*it)[_1] == 31.1);
        REQUIRE((*it)[_2] == 2022_y / January / 3);
        REQUIRE((*it)[_3] == 11.1);
        REQUIRE((*it)[_4] == true);
        it++;
        REQUIRE((*it)[_0] == 2);
        REQUIRE((*it)[_1] == 32.2);
        REQUIRE((*it)[_2] == 2022_y / January / 4);
        REQUIRE((*it)[_3] == 12.2);
        REQUIRE((*it)[_4] == false);
        it++;
        REQUIRE((*it)[_0] == 3);
        REQUIRE((*it)[_1] == 33.3);
        REQUIRE((*it)[_2] == 2022_y / January / 5);
        REQUIRE((*it)[_3] == 13.3);
        REQUIRE((*it)[_4] == false);
        it++;
        REQUIRE((*it)[_0] == 4);
        REQUIRE((*it)[_1] == 34.4);
        REQUIRE((*it)[_2] == 2022_y / January / 6);
        REQUIRE((*it)[_3] == 14.4);
        REQUIRE((*it)[_4] == true);
        it++;
        REQUIRE((*it)[_0] == 5);
        REQUIRE((*it)[_1] == 0.0);
        REQUIRE((*it)[_2] == 2022_y / January / 7);
        REQUIRE((*it)[_3] == 15.5);
        REQUIRE((*it)[_4] == false);
        it++;
        REQUIRE((*it)[_0] == 6);
        REQUIRE((*it)[_1] == 0.0);
        REQUIRE((*it)[_2] == year_month_day{});
        REQUIRE((*it)[_3] == 0.0);
        REQUIRE((*it)[_4] == false);
    }
}

TEST_CASE("expression fn", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, true);
    f1.push_back(2022_y / January / 4, 12.2, false);
    f1.push_back(2022_y / January / 5, 13.3, false);
    f1.push_back(2022_y / January / 6, 14.4, true);
    f1.push_back(2022_y / January / 7, 15.5, false);
    auto f2 = f1.append_column<double>( "tempceil", fn<double(double)>( ceil, _1 ) )
        .append_column<double>( "temptrunc", fn<double(double)>( trunc, _1 ) )
        .append_column<double>( "temptruncp.5", fn<double(double)>( trunc, _1+0.5 ) )
        .append_column<double>( "temptruncp.5p", fn<double(double)>( trunc, _1 ) +0.5 )
        .append_column<double>( "max", fn<double(double,double)>( ::fmax, _5, _6 ) );
    dout << "expression fn f2:\n";
    dout << f2;
    auto it = f2.cbegin();

    // Same as before
    REQUIRE((it + 0)->at(_0) == 2022_y / January / 2);
    REQUIRE((it + 0)->at(_1) == 10.0);
    REQUIRE((it + 0)->at(_2) == false);
    REQUIRE((it + 1)->at(_0) == 2022_y / January / 3);
    REQUIRE((it + 1)->at(_1) == 11.1);
    REQUIRE((it + 1)->at(_2) == true);
    REQUIRE((it + 2)->at(_0) == 2022_y / January / 4);
    REQUIRE((it + 2)->at(_1) == 12.2);
    REQUIRE((it + 2)->at(_2) == false);
    REQUIRE((it + 3)->at(_0) == 2022_y / January / 5);
    REQUIRE((it + 3)->at(_1) == 13.3);
    REQUIRE((it + 3)->at(_2) == false);
    REQUIRE((it + 4)->at(_0) == 2022_y / January / 6);
    REQUIRE((it + 4)->at(_1) == 14.4);
    REQUIRE((it + 4)->at(_2) == true);
    REQUIRE((it + 5)->at(_0) == 2022_y / January / 7);
    REQUIRE((it + 5)->at(_1) == 15.5);
    REQUIRE((it + 5)->at(_2) == false);

    // clang-format off
    //  |         _0 |          _1 |    _2 |       _3 |        _4 |           _5 |            _6 |   _7
    //  |       date | temperature |  rain | tempceil | temptrunc | temptruncp.5 | temptruncp.5p |  max
    //__|____________|_____________|_______|__________|___________|______________|_______________|______
    // 0| 2022-01-02 |          10 | false |       10 |        10 |           10 |          10.5 | 10.5
    // 1| 2022-01-03 |        11.1 |  true |       12 |        11 |           11 |          11.5 | 11.5
    // 2| 2022-01-04 |        12.2 | false |       13 |        12 |           12 |          12.5 | 12.5
    // 3| 2022-01-05 |        13.3 | false |       14 |        13 |           13 |          13.5 | 13.5
    // 4| 2022-01-06 |        14.4 |  true |       15 |        14 |           14 |          14.5 | 14.5
    // 5| 2022-01-07 |        15.5 | false |       16 |        15 |           16 |          15.5 |   16
    // clang-format on

    // tempceil
    REQUIRE((it + 0)->at(_3) == 10.0);
    REQUIRE((it + 1)->at(_3) == 12.0);
    REQUIRE((it + 2)->at(_3) == 13.0);
    REQUIRE((it + 3)->at(_3) == 14.0);
    REQUIRE((it + 4)->at(_3) == 15.0);
    REQUIRE((it + 5)->at(_3) == 16.0);

    // temptrunc
    REQUIRE((it + 0)->at(_4) == 10.0);
    REQUIRE((it + 1)->at(_4) == 11.0);
    REQUIRE((it + 2)->at(_4) == 12.0);
    REQUIRE((it + 3)->at(_4) == 13.0);
    REQUIRE((it + 4)->at(_4) == 14.0);
    REQUIRE((it + 5)->at(_4) == 15.0);

    // temptruncp.5
    REQUIRE((it + 0)->at(_5) == 10.0);
    REQUIRE((it + 1)->at(_5) == 11.0);
    REQUIRE((it + 2)->at(_5) == 12.0);
    REQUIRE((it + 3)->at(_5) == 13.0);
    REQUIRE((it + 4)->at(_5) == 14.0);
    REQUIRE((it + 5)->at(_5) == 16.0);

    // temptruncp.5p
    REQUIRE((it + 0)->at(_6) == 10.5);
    REQUIRE((it + 1)->at(_6) == 11.5);
    REQUIRE((it + 2)->at(_6) == 12.5);
    REQUIRE((it + 3)->at(_6) == 13.5);
    REQUIRE((it + 4)->at(_6) == 14.5);
    REQUIRE((it + 5)->at(_6) == 15.5);

    // max
    REQUIRE((it + 0)->at(_7) == 10.5);
    REQUIRE((it + 1)->at(_7) == 11.5);
    REQUIRE((it + 2)->at(_7) == 12.5);
    REQUIRE((it + 3)->at(_7) == 13.5);
    REQUIRE((it + 4)->at(_7) == 14.5);
    REQUIRE((it + 5)->at(_7) == 16.0);
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
    auto f2 = f1.append_column<year_month_day>("next year", _0 + years(1));
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

        auto f2 = f1.append_column<mi<year_month_day>>("yesterday", _0[-1]);
        dout << f2;
        REQUIRE(f2.num_columns() == 4);
        auto b2 = f2.begin();
        REQUIRE((b2 + 0)->at(_3) == missing);
        REQUIRE((b2 + 1)->at(_3) == 2022_y / January / 1);
        REQUIRE((b2 + 2)->at(_3) == 2022_y / January / 2);
        REQUIRE((b2 + 8)->at(_3) == 2022_y / January / 8);

        auto f3 = f1.append_column<mi<year_month_day>>("tomorrow last year", _0[+1] - years(1));
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

        auto f2 = f1.append_column<mi<year_month_day>>("yesterday", _0[-1]);
        dout << f2;
        REQUIRE(f2.num_columns() == 4);
        auto b2 = f2.begin();
        REQUIRE((b2 + 0)->at(_3) == missing);
        REQUIRE((b2 + 1)->at(_3) == 2022_y / January / 1);
        REQUIRE((b2 + 2)->at(_3) == 2022_y / January / 2);
        REQUIRE((b2 + 8)->at(_3) == 2022_y / January / 8);

        auto f3 = f1.append_column<mi<year_month_day>>("tomorrow last year", _0[+1] - years(1));
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

        auto f2 = f1.append_column<year_month_day>("yesterday", _0[-1]);
        dout << f2;
        REQUIRE(f2.num_columns() == 4);
        auto b2 = f2.begin();
        REQUIRE((b2 + 0)->at(_3) == year_month_day{});
        REQUIRE((b2 + 1)->at(_3) == 2022_y / January / 1);
        REQUIRE((b2 + 2)->at(_3) == 2022_y / January / 2);
        REQUIRE((b2 + 8)->at(_3) == 2022_y / January / 8);

        auto f3 = f1.append_column<year_month_day>("tomorrow last year", _0[+1] - years(1));
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

        double adm = mf::detail::mean(ad, NUMELEMS);
        REQUIRE(adm == Approx(500.5));
        double admv = mf::detail::mean(ad, NUMELEMS);
        REQUIRE(admv == Approx(500.5));

        double afm = mf::detail::mean(af, NUMELEMS);
        REQUIRE(afm == Approx(500.5));
        double afmv = mf::detail::mean(af, NUMELEMS);
        REQUIRE(afmv == Approx(500.5));

        double aim = mf::detail::mean(ai, NUMELEMS);
        REQUIRE(aim == Approx(500.5));
        double aum = mf::detail::mean(au, NUMELEMS);
        REQUIRE(aum == Approx(500.5));
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

    SECTION("method")
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
        f1.sort(_2, _1);
        dout << f1;

        auto it = f1.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / January / 8);
        REQUIRE((it + 1)->at(_0) == 2022_y / January / 3);
        REQUIRE((it + 2)->at(_0) == 2022_y / January / 6);
        REQUIRE((it + 3)->at(_0) == 2022_y / January / 1);
        REQUIRE((it + 4)->at(_0) == 2022_y / January / 9);
        REQUIRE((it + 5)->at(_0) == 2022_y / January / 2);
        REQUIRE((it + 6)->at(_0) == 2022_y / January / 4);
        REQUIRE((it + 7)->at(_0) == 2022_y / January / 5);
        REQUIRE((it + 8)->at(_0) == 2022_y / January / 7);

        REQUIRE((it + 0)->at(_1) == 9.1);
        REQUIRE((it + 1)->at(_1) == 11.1);
        REQUIRE((it + 2)->at(_1) == 14.4);
        REQUIRE((it + 3)->at(_1) == 8.9);
        REQUIRE((it + 4)->at(_1) == 9.3);
        REQUIRE((it + 5)->at(_1) == 10.0);
        REQUIRE((it + 6)->at(_1) == 12.2);
        REQUIRE((it + 7)->at(_1) == 13.3);
        REQUIRE((it + 8)->at(_1) == 15.5);

        REQUIRE((it + 0)->at(_2) == 7);
        REQUIRE((it + 1)->at(_2) == 7);
        REQUIRE((it + 2)->at(_2) == 7);
        REQUIRE((it + 3)->at(_2) == 10);
        REQUIRE((it + 4)->at(_2) == 10);
        REQUIRE((it + 5)->at(_2) == 10);
        REQUIRE((it + 6)->at(_2) == 10);
        REQUIRE((it + 7)->at(_2) == 10);
        REQUIRE((it + 8)->at(_2) == 10);
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

    auto gf = f1.groupby(_0, _1);

    // start with groupby( _5, _6 ).aggregate( sum( _1 ), sum( _2 ), count(), min( _4 ), sum( _3 ),
    // sum( _1 ), max( _1 ) )
    using F    = frame<char, short, int, long, float, double>;
    using RCFA = typename mf::detail::get_result_columns_from_args<F, decltype(mf::agg::sum(_0)),
        decltype(mf::agg::sum(_1)), decltype(mf::agg::count()), decltype(mf::agg::min(_3)),
        decltype(mf::agg::sum(_2)), decltype(mf::agg::sum(_0)), decltype(mf::agg::max(_0))>::type;
    RCFA rcfa;
    (void)rcfa;
    // TD<RCFA> foo;

    using AF = typename mf::detail::get_aggregate_frame<F, index_defn<4, 5>,
        decltype(mf::agg::sum(_0)), decltype(mf::agg::sum(_1)), decltype(mf::agg::count()),
        decltype(mf::agg::min(_3)), decltype(mf::agg::sum(_2)), decltype(mf::agg::sum(_0)),
        decltype(mf::agg::max(_0))>::type;
    AF af;
    (void)af;
    // TD<AF> foo4;
    using TUP = tuple<decltype(mf::agg::sum(_0)), decltype(mf::agg::sum(_1)),
        decltype(mf::agg::count()), decltype(mf::agg::min(_3)), decltype(mf::agg::sum(_2)),
        decltype(mf::agg::sum(_0)), decltype(mf::agg::max(_0))>;
    TUP tup;
    (void)tup;
    // TD<TUP> foo6;
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
        agg::sum(_2), agg::min(_2), agg::max(_2), agg::mean(_2), agg::stddev(_2), agg::count());
    auto f3 = gf.aggregate(
        agg::sum(_3), agg::min(_3), agg::max(_3), agg::mean(_3), agg::stddev(_3), agg::count());
    std::sort(f2.begin(), f2.end());
    std::sort(f3.begin(), f3.end());
    dout << "SELECT SUM(rain), MIN(rain), MAX(rain), MEAN(rain), STDDEV(rain), COUNT(*)\n";
    dout << f2;
    dout << "SELECT SUM(humidity), MIN(humidity), MAX(humidity), MEAN(humidity), STDDEV(humidity), "
            "COUNT(*)\n";
    dout << f3;

    // rain
    {
        // clang-format off
        //   |       date | temperature | sum( rain ) | min( rain ) | max( rain ) | mean( rain ) | stddev( rain ) | count(*)
        // __|____________|_____________|_____________|_____________|_____________|______________|________________|__________
        //  0| 2022-01-01 |           1 |           0 |           0 |           0 |            0 |              0 |        1
        //  1| 2022-01-02 |           2 |           3 |           1 |           2 |          1.5 |            0.5 |        2
        //  2| 2022-01-03 |           3 |           9 |           2 |           4 |            3 |       0.816497 |        3
        //  3| 2022-01-04 |           4 |          12 |           3 |           5 |            4 |       0.816497 |        3
        //  4| 2022-01-05 |           5 |          15 |           4 |           6 |            5 |       0.816497 |        3
        //  5| 2022-01-06 |           6 |          18 |           5 |           7 |            6 |       0.816497 |        3
        //  6| 2022-01-07 |           7 |          21 |           6 |           8 |            7 |       0.816497 |        3
        //  7| 2022-01-08 |           8 |          15 |           7 |           8 |          7.5 |            0.5 |        2
        //  8| 2022-01-09 |           9 |           9 |           9 |           9 |            9 |              0 |        1
        // clang-format on

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

        // count()
        REQUIRE((it + 0)->at(_7) == 1);
        REQUIRE((it + 1)->at(_7) == 2);
        REQUIRE((it + 2)->at(_7) == 3);
        REQUIRE((it + 3)->at(_7) == 3);
        REQUIRE((it + 4)->at(_7) == 3);
        REQUIRE((it + 5)->at(_7) == 3);
        REQUIRE((it + 6)->at(_7) == 3);
        REQUIRE((it + 7)->at(_7) == 2);
        REQUIRE((it + 8)->at(_7) == 1);
    }

    // humidity
    {
        // clang-format off
        //   |       date | temperature | sum( humidity ) | min( humidity ) | max( humidity ) | mean( humidity ) | stddev( humidity ) | count(*)
        // __|____________|_____________|_________________|_________________|_________________|__________________|____________________|__________
        //  0| 2022-01-01 |           1 |              -5 |              -5 |              -5 |               -5 |                  0 |        1
        //  1| 2022-01-02 |           2 |             -10 |            -5.5 |            -4.5 |               -5 |                0.5 |        2
        //  2| 2022-01-03 |           3 |             -15 |              -6 |              -4 |               -5 |           0.816497 |        3
        //  3| 2022-01-04 |           4 |             -15 |              -6 |              -4 |               -5 |           0.816497 |        3
        //  4| 2022-01-05 |           5 |             -15 |              -6 |              -4 |               -5 |           0.816497 |        3
        //  5| 2022-01-06 |           6 |             -15 |              -6 |              -4 |               -5 |           0.816497 |        3
        //  6| 2022-01-07 |           7 |             -15 |              -6 |              -4 |               -5 |           0.816497 |        3
        //  7| 2022-01-08 |           8 |             -10 |            -5.5 |            -4.5 |               -5 |                0.5 |        2
        //  8| 2022-01-09 |           9 |              -5 |              -5 |              -5 |               -5 |                  0 |        1
        // clang-format on

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

        // count()
        REQUIRE((it + 0)->at(_7) == 1);
        REQUIRE((it + 1)->at(_7) == 2);
        REQUIRE((it + 2)->at(_7) == 3);
        REQUIRE((it + 3)->at(_7) == 3);
        REQUIRE((it + 4)->at(_7) == 3);
        REQUIRE((it + 5)->at(_7) == 3);
        REQUIRE((it + 6)->at(_7) == 3);
        REQUIRE((it + 7)->at(_7) == 2);
        REQUIRE((it + 8)->at(_7) == 1);
    }
}

TEST_CASE("hcat", "[frame]")
{
    SECTION("same size")
    {
        frame<year_month_day, bool> f1;
        f1.set_column_names("date", "rain");
        f1.push_back(2022_y / January / 1, false);
        f1.push_back(2022_y / January / 2, false);
        f1.push_back(2022_y / January / 3, true);
        f1.push_back(2022_y / January / 4, true);

        frame<year_month_day, double> f2;
        f2.set_column_names("date", "temperature");
        f2.push_back(2022_y / January / 6, 14.4);
        f2.push_back(2022_y / January / 7, 15.5);
        f2.push_back(2022_y / January / 8, 9.1);
        f2.push_back(2022_y / January / 9, 9.3);

        frame<year_month_day, double, year_month_day, bool> f3 = f2.hcat(f1);

        REQUIRE(f3.size() == 4);

        dout << "f2.hcat(f1):\n";
        dout << f3;

        auto it = f3.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 6);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 7);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 8);
        REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 9);

        REQUIRE((it + 0)->at(_1) == 14.4);
        REQUIRE((it + 1)->at(_1) == 15.5);
        REQUIRE((it + 2)->at(_1) == 9.1);
        REQUIRE((it + 3)->at(_1) == 9.3);

        REQUIRE((it + 0)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_2) == 2022_y / 1 / 2);
        REQUIRE((it + 2)->at(_2) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_2) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_3) == false);
        REQUIRE((it + 1)->at(_3) == false);
        REQUIRE((it + 2)->at(_3) == true);
        REQUIRE((it + 3)->at(_3) == true);
    }

    SECTION("right smaller")
    {
        frame<year_month_day, bool> f1;
        f1.set_column_names("date", "rain");
        f1.push_back(2022_y / January / 1, false);
        f1.push_back(2022_y / January / 2, false);
        f1.push_back(2022_y / January / 3, true);

        frame<year_month_day, double> f2;
        f2.set_column_names("date", "temperature");
        f2.push_back(2022_y / January / 6, 14.4);
        f2.push_back(2022_y / January / 7, 15.5);
        f2.push_back(2022_y / January / 8, 9.1);
        f2.push_back(2022_y / January / 9, 9.3);

        frame<year_month_day, double, year_month_day, bool> f3 = f2.hcat(f1);

        REQUIRE(f3.size() == 4);

        dout << "f2.hcat(f1):\n";
        dout << f3;

        auto it = f3.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 6);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 7);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 8);
        REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 9);

        REQUIRE((it + 0)->at(_1) == 14.4);
        REQUIRE((it + 1)->at(_1) == 15.5);
        REQUIRE((it + 2)->at(_1) == 9.1);
        REQUIRE((it + 3)->at(_1) == 9.3);

        REQUIRE((it + 0)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_2) == 2022_y / 1 / 2);
        REQUIRE((it + 2)->at(_2) == 2022_y / 1 / 3);
        REQUIRE(!(it + 3)->at(_2).ok());

        REQUIRE((it + 0)->at(_3) == false);
        REQUIRE((it + 1)->at(_3) == false);
        REQUIRE((it + 2)->at(_3) == true);
        REQUIRE((it + 3)->at(_3) == false);
    }

    SECTION("right empty")
    {
        frame<year_month_day, bool> f1;
        f1.set_column_names("date", "rain");

        frame<year_month_day, double> f2;
        f2.set_column_names("date", "temperature");
        f2.push_back(2022_y / January / 6, 14.4);
        f2.push_back(2022_y / January / 7, 15.5);
        f2.push_back(2022_y / January / 8, 9.1);
        f2.push_back(2022_y / January / 9, 9.3);

        frame<year_month_day, double, year_month_day, bool> f3 = f2.hcat(f1);

        REQUIRE(f3.size() == 4);

        dout << "f2.hcat(f1):\n";
        dout << f3;

        auto it = f3.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 6);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 7);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 8);
        REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 9);

        REQUIRE((it + 0)->at(_1) == 14.4);
        REQUIRE((it + 1)->at(_1) == 15.5);
        REQUIRE((it + 2)->at(_1) == 9.1);
        REQUIRE((it + 3)->at(_1) == 9.3);

        REQUIRE(!(it + 0)->at(_2).ok());
        REQUIRE(!(it + 1)->at(_2).ok());
        REQUIRE(!(it + 2)->at(_2).ok());
        REQUIRE(!(it + 3)->at(_2).ok());

        REQUIRE((it + 0)->at(_3) == false);
        REQUIRE((it + 1)->at(_3) == false);
        REQUIRE((it + 2)->at(_3) == false);
        REQUIRE((it + 3)->at(_3) == false);
    }

    SECTION("left small")
    {
        frame<year_month_day, bool> f1;
        f1.set_column_names("date", "rain");
        f1.push_back(2022_y / January / 1, false);
        f1.push_back(2022_y / January / 2, false);
        f1.push_back(2022_y / January / 3, true);
        f1.push_back(2022_y / January / 4, true);

        frame<year_month_day, double> f2;
        f2.set_column_names("date", "temperature");
        f2.push_back(2022_y / January / 6, 14.4);
        f2.push_back(2022_y / January / 7, 15.5);
        f2.push_back(2022_y / January / 8, 9.1);

        frame<year_month_day, double, year_month_day, bool> f3 = f2.hcat(f1);

        REQUIRE(f3.size() == 4);

        dout << "f2.hcat(f1):\n";
        dout << f3;

        auto it = f3.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 6);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 7);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 8);
        REQUIRE(!(it + 3)->at(_0).ok());

        REQUIRE((it + 0)->at(_1) == 14.4);
        REQUIRE((it + 1)->at(_1) == 15.5);
        REQUIRE((it + 2)->at(_1) == 9.1);
        REQUIRE((it + 3)->at(_1) == 0.0);

        REQUIRE((it + 0)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_2) == 2022_y / 1 / 2);
        REQUIRE((it + 2)->at(_2) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_2) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_3) == false);
        REQUIRE((it + 1)->at(_3) == false);
        REQUIRE((it + 2)->at(_3) == true);
        REQUIRE((it + 3)->at(_3) == true);
    }

    SECTION("left empty")
    {
        frame<year_month_day, bool> f1;
        f1.set_column_names("date", "rain");
        f1.push_back(2022_y / January / 1, false);
        f1.push_back(2022_y / January / 2, false);
        f1.push_back(2022_y / January / 3, true);
        f1.push_back(2022_y / January / 4, true);

        frame<year_month_day, double> f2;
        f2.set_column_names("date", "temperature");

        frame<year_month_day, double, year_month_day, bool> f3 = f2.hcat(f1);

        REQUIRE(f3.size() == 4);

        dout << "f2.hcat(f1):\n";
        dout << f3;

        auto it = f3.cbegin();
        REQUIRE(!(it + 0)->at(_0).ok());
        REQUIRE(!(it + 1)->at(_0).ok());
        REQUIRE(!(it + 2)->at(_0).ok());
        REQUIRE(!(it + 3)->at(_0).ok());

        REQUIRE((it + 0)->at(_1) == 0.0);
        REQUIRE((it + 1)->at(_1) == 0.0);
        REQUIRE((it + 2)->at(_1) == 0.0);
        REQUIRE((it + 3)->at(_1) == 0.0);

        REQUIRE((it + 0)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_2) == 2022_y / 1 / 2);
        REQUIRE((it + 2)->at(_2) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_2) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_3) == false);
        REQUIRE((it + 1)->at(_3) == false);
        REQUIRE((it + 2)->at(_3) == true);
        REQUIRE((it + 3)->at(_3) == true);
    }
}

TEST_CASE("innerjoin", "[frame]")
{
    SECTION("no duplicates")
    {
        frame<year_month_day, bool> f1;
        f1.set_column_names("date", "rain");
        f1.push_back(2022_y / January / 1, false);
        f1.push_back(2022_y / January / 2, false);
        f1.push_back(2022_y / January / 3, true);
        f1.push_back(2022_y / January / 4, true);

        frame<year_month_day, double> f2;
        f2.set_column_names("date", "temperature");
        f2.push_back(2022_y / January / 1, 9.0);
        f2.push_back(2022_y / January / 2, 10.0);
        f2.push_back(2022_y / January / 3, 11.0);
        f2.push_back(2022_y / January / 4, 12.2);

        auto res = innerjoin(f1, _0, f2, _0);
        res.sort(_0, _1);
        dout << res;
        REQUIRE(res.size() == 4);

        // 0| 2022-01-01 | false | 2022-01-01 |    9
        // 1| 2022-01-02 | false | 2022-01-02 |   10
        // 2| 2022-01-03 |  true | 2022-01-03 |   11
        // 3| 2022-01-04 |  true | 2022-01-04 | 12.2

        auto it = res.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 2);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_1) == false);
        REQUIRE((it + 1)->at(_1) == false);
        REQUIRE((it + 2)->at(_1) == true);
        REQUIRE((it + 3)->at(_1) == true);

        REQUIRE((it + 0)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_2) == 2022_y / 1 / 2);
        REQUIRE((it + 2)->at(_2) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_2) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_3) == 9.0);
        REQUIRE((it + 1)->at(_3) == 10.0);
        REQUIRE((it + 2)->at(_3) == 11.0);
        REQUIRE((it + 3)->at(_3) == 12.2);
    }

    SECTION("left duplicates")
    {
        frame<year_month_day, bool> f1;
        f1.set_column_names("date", "rain");
        f1.push_back(2022_y / January / 1, false);
        f1.push_back(2022_y / January / 1, true);
        f1.push_back(2022_y / January / 3, true);
        f1.push_back(2022_y / January / 4, true);

        frame<year_month_day, double> f2;
        f2.set_column_names("date", "temperature");
        f2.push_back(2022_y / January / 1, 9.0);
        f2.push_back(2022_y / January / 2, 10.0);
        f2.push_back(2022_y / January / 3, 11.0);
        f2.push_back(2022_y / January / 4, 12.2);

        auto res = innerjoin(f1, _0, f2, _0);
        res.sort(_0, _1);
        dout << res;
        REQUIRE(res.size() == 4);

        // 0| 2022-01-01 | false | 2022-01-01 |    9
        // 1| 2022-01-01 |  true | 2022-01-01 |    9
        // 2| 2022-01-03 |  true | 2022-01-03 |   11
        // 3| 2022-01-04 |  true | 2022-01-04 | 12.2

        auto it = res.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_1) == false);
        REQUIRE((it + 1)->at(_1) == true);
        REQUIRE((it + 2)->at(_1) == true);
        REQUIRE((it + 3)->at(_1) == true);

        REQUIRE((it + 0)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 2)->at(_2) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_2) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_3) == 9.0);
        REQUIRE((it + 1)->at(_3) == 9.0);
        REQUIRE((it + 2)->at(_3) == 11.0);
        REQUIRE((it + 3)->at(_3) == 12.2);
    }

    SECTION("right duplicates")
    {
        frame<year_month_day, bool> f1;
        f1.set_column_names("date", "rain");
        f1.push_back(2022_y / January / 1, false);
        f1.push_back(2022_y / January / 2, true);
        f1.push_back(2022_y / January / 3, true);
        f1.push_back(2022_y / January / 4, true);

        frame<year_month_day, double> f2;
        f2.set_column_names("date", "temperature");
        f2.push_back(2022_y / January / 1, 9.0);
        f2.push_back(2022_y / January / 1, 10.0);
        f2.push_back(2022_y / January / 3, 11.0);
        f2.push_back(2022_y / January / 4, 12.2);

        auto res = innerjoin(f1, _0, f2, _0);
        res.sort(_0, _1);
        dout << res;
        REQUIRE(res.size() == 4);

        // 0| 2022-01-01 | false | 2022-01-01 |    9
        // 1| 2022-01-01 | false | 2022-01-01 |   10
        // 2| 2022-01-03 |  true | 2022-01-03 |   11
        // 3| 2022-01-04 |  true | 2022-01-04 | 12.2

        auto it = res.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_1) == false);
        REQUIRE((it + 1)->at(_1) == false);
        REQUIRE((it + 2)->at(_1) == true);
        REQUIRE((it + 3)->at(_1) == true);

        REQUIRE((it + 0)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 2)->at(_2) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_2) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_3) == 9.0);
        REQUIRE((it + 1)->at(_3) == 10.0);
        REQUIRE((it + 2)->at(_3) == 11.0);
        REQUIRE((it + 3)->at(_3) == 12.2);
    }

    SECTION("left empty")
    {
        frame<year_month_day, bool> f1;
        f1.set_column_names("date", "rain");

        frame<year_month_day, double> f2;
        f2.set_column_names("date", "temperature");
        f2.push_back(2022_y / January / 1, 9.0);
        f2.push_back(2022_y / January / 2, 10.0);
        f2.push_back(2022_y / January / 3, 11.0);
        f2.push_back(2022_y / January / 4, 12.2);

        auto res = innerjoin(f1, _0, f2, _0);
        res.sort(_0, _1);
        dout << res;
        REQUIRE(res.size() == 0);
    }

    SECTION("right empty")
    {
        frame<year_month_day, bool> f1;
        f1.set_column_names("date", "rain");
        f1.push_back(2022_y / January / 1, false);
        f1.push_back(2022_y / January / 2, true);
        f1.push_back(2022_y / January / 3, true);
        f1.push_back(2022_y / January / 4, true);

        frame<year_month_day, double> f2;
        f2.set_column_names("date", "temperature");

        auto res = innerjoin(f1, _0, f2, _0);
        res.sort(_0, _1);
        dout << res;
        REQUIRE(res.size() == 0);
    }
}

TEST_CASE("leftjoin", "[frame]")
{
    SECTION("no duplicates")
    {
        frame<mi<year_month_day>, mi<bool>> f1;
        f1.set_column_names("date", "rain");
        f1.push_back(2022_y / January / 1, false);
        f1.push_back(2022_y / January / 2, false);
        f1.push_back(2022_y / January / 3, true);
        f1.push_back(2022_y / January / 4, true);

        frame<mi<year_month_day>, mi<double>> f2;
        f2.set_column_names("date", "temperature");
        f2.push_back(2022_y / January / 1, 9.0);
        f2.push_back(2022_y / January / 2, 10.0);
        f2.push_back(2022_y / January / 3, 11.0);
        f2.push_back(2022_y / January / 4, 12.2);

        auto res = leftjoin(f1, _0, f2, _0);
        res.sort(_0, _1);
        dout << res;
        REQUIRE(res.size() == 4);

        // 0| 2022-01-01 | false | 2022-01-01 |    9
        // 1| 2022-01-02 | false | 2022-01-02 |   10
        // 2| 2022-01-03 |  true | 2022-01-03 |   11
        // 3| 2022-01-04 |  true | 2022-01-04 | 12.2

        auto it = res.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 2);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_1) == false);
        REQUIRE((it + 1)->at(_1) == false);
        REQUIRE((it + 2)->at(_1) == true);
        REQUIRE((it + 3)->at(_1) == true);

        REQUIRE((it + 0)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_2) == 2022_y / 1 / 2);
        REQUIRE((it + 2)->at(_2) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_2) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_3) == 9.0);
        REQUIRE((it + 1)->at(_3) == 10.0);
        REQUIRE((it + 2)->at(_3) == 11.0);
        REQUIRE((it + 3)->at(_3) == 12.2);
    }

    SECTION("left duplicates")
    {
        frame<mi<year_month_day>, mi<bool>> f1;
        f1.set_column_names("date", "rain");
        f1.push_back(2022_y / January / 1, false);
        f1.push_back(2022_y / January / 1, true);
        f1.push_back(2022_y / January / 3, true);
        f1.push_back(2022_y / January / 4, true);

        frame<mi<year_month_day>, mi<double>> f2;
        f2.set_column_names("date", "temperature");
        f2.push_back(2022_y / January / 1, 9.0);
        f2.push_back(2022_y / January / 2, 10.0);
        f2.push_back(2022_y / January / 3, 11.0);
        f2.push_back(2022_y / January / 4, 12.2);

        auto res = leftjoin(f1, _0, f2, _0);
        res.sort(_0, _1);
        dout << res;
        REQUIRE(res.size() == 4);

        // 0| 2022-01-01 | false | 2022-01-01 |    9
        // 1| 2022-01-01 |  true | 2022-01-01 |    9
        // 2| 2022-01-03 |  true | 2022-01-03 |   11
        // 3| 2022-01-04 |  true | 2022-01-04 | 12.2

        auto it = res.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_1) == false);
        REQUIRE((it + 1)->at(_1) == true);
        REQUIRE((it + 2)->at(_1) == true);
        REQUIRE((it + 3)->at(_1) == true);

        REQUIRE((it + 0)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 2)->at(_2) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_2) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_3) == 9.0);
        REQUIRE((it + 1)->at(_3) == 9.0);
        REQUIRE((it + 2)->at(_3) == 11.0);
        REQUIRE((it + 3)->at(_3) == 12.2);
    }

    SECTION("right duplicates")
    {
        frame<mi<year_month_day>, mi<bool>> f1;
        f1.set_column_names("date", "rain");
        f1.push_back(2022_y / January / 1, false);
        f1.push_back(2022_y / January / 2, true);
        f1.push_back(2022_y / January / 3, true);
        f1.push_back(2022_y / January / 4, true);

        frame<mi<year_month_day>, mi<double>> f2;
        f2.set_column_names("date", "temperature");
        f2.push_back(2022_y / January / 1, 9.0);
        f2.push_back(2022_y / January / 1, 10.0);
        f2.push_back(2022_y / January / 3, 11.0);
        f2.push_back(2022_y / January / 4, 12.2);

        auto res = leftjoin(f1, _0, f2, _0);
        res.sort(_0, _1);
        dout << res;
        REQUIRE(res.size() == 5);

        // 0| 2022-01-01 | false | 2022-01-01 |       9
        // 1| 2022-01-01 | false | 2022-01-01 |      10
        // 2| 2022-01-02 |  true |    missing | missing
        // 3| 2022-01-03 |  true | 2022-01-03 |      11
        // 4| 2022-01-04 |  true | 2022-01-04 |    12.2

        auto it = res.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 2);
        REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 3);
        REQUIRE((it + 4)->at(_0) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_1) == false);
        REQUIRE((it + 1)->at(_1) == false);
        REQUIRE((it + 2)->at(_1) == true);
        REQUIRE((it + 3)->at(_1) == true);
        REQUIRE((it + 4)->at(_1) == true);

        REQUIRE((it + 0)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_2) == 2022_y / 1 / 1);
        REQUIRE((it + 2)->at(_2) == missing);
        REQUIRE((it + 3)->at(_2) == 2022_y / 1 / 3);
        REQUIRE((it + 4)->at(_2) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_3) == 9.0);
        REQUIRE((it + 1)->at(_3) == 10.0);
        REQUIRE((it + 2)->at(_3) == missing);
        REQUIRE((it + 3)->at(_3) == 11.0);
        REQUIRE((it + 4)->at(_3) == 12.2);
    }

    SECTION("left empty")
    {
        frame<mi<year_month_day>, mi<bool>> f1;
        f1.set_column_names("date", "rain");

        frame<mi<year_month_day>, mi<double>> f2;
        f2.set_column_names("date", "temperature");
        f2.push_back(2022_y / January / 1, 9.0);
        f2.push_back(2022_y / January / 2, 10.0);
        f2.push_back(2022_y / January / 3, 11.0);
        f2.push_back(2022_y / January / 4, 12.2);

        auto res = leftjoin(f1, _0, f2, _0);
        res.sort(_0, _1);
        dout << res;
        REQUIRE(res.size() == 0);
    }

    SECTION("right empty")
    {
        frame<mi<year_month_day>, mi<bool>> f1;
        f1.set_column_names("date", "rain");
        f1.push_back(2022_y / January / 1, false);
        f1.push_back(2022_y / January / 2, true);
        f1.push_back(2022_y / January / 3, true);
        f1.push_back(2022_y / January / 4, true);

        frame<mi<year_month_day>, mi<double>> f2;
        f2.set_column_names("date", "temperature");

        auto res = leftjoin(f1, _0, f2, _0);
        res.sort(_0, _1);
        dout << res;
        REQUIRE(res.size() == 4);
        // 1| 2022-01-01 | false | missing | missing
        // 2| 2022-01-02 |  true | missing | missing
        // 3| 2022-01-03 |  true | missing | missing
        // 4| 2022-01-04 |  true | missing | missing

        auto it = res.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
        REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 2);
        REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 3);
        REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 4);

        REQUIRE((it + 0)->at(_1) == false);
        REQUIRE((it + 1)->at(_1) == true);
        REQUIRE((it + 2)->at(_1) == true);
        REQUIRE((it + 3)->at(_1) == true);

        REQUIRE((it + 0)->at(_2) == missing);
        REQUIRE((it + 1)->at(_2) == missing);
        REQUIRE((it + 2)->at(_2) == missing);
        REQUIRE((it + 3)->at(_2) == missing);

        REQUIRE((it + 0)->at(_3) == missing);
        REQUIRE((it + 1)->at(_3) == missing);
        REQUIRE((it + 2)->at(_3) == missing);
        REQUIRE((it + 3)->at(_3) == missing);
    }
}

TEST_CASE("replace_missing", "[frame]")
{
    frame<mi<year_month_day>, mi<double>, mi<bool>> f1;
    f1.set_column_names("date", "temperature", "rain");
    f1.push_back(2022_y / January / 1, 8.9, true);
    f1.push_back(2022_y / January / 2, 10.0, false);
    f1.push_back(2022_y / January / 3, 11.1, true);
    f1.push_back(2022_y / January / 4, 12.2, false);
    f1.push_back(2022_y / January / 5, 14.4, false);
    REQUIRE(f1.size() == 5);
    auto row = f1.row(4);

    SECTION("no replacemant")
    {
        frame_row<mi<year_month_day>, mi<double>, mi<bool>> fr1;
        fr1.at(_0) = 2022_y / January / 6;
        fr1.at(_1) = 15.5;
        fr1.at(_2) = true;

        fr1.replace_missing(row);

        REQUIRE(fr1.at(_0) == 2022_y / January / 6);
        REQUIRE(fr1.at(_1) == 15.5);
        REQUIRE(fr1.at(_2) == true);
    }

    SECTION("first date replacemant")
    {
        frame_row<mi<year_month_day>, mi<double>, mi<bool>> fr1;
        fr1.at(_0) = missing;
        fr1.at(_1) = 15.5;
        fr1.at(_2) = true;

        fr1.replace_missing(row);

        REQUIRE(fr1.at(_0) == 2022_y / January / 5);
        REQUIRE(fr1.at(_1) == 15.5);
        REQUIRE(fr1.at(_2) == true);
    }

    SECTION("second fp replacemant")
    {
        frame_row<mi<year_month_day>, mi<double>, mi<bool>> fr1;
        fr1.at(_0) = 2022_y / January / 6;
        fr1.at(_1) = missing;
        fr1.at(_2) = true;

        fr1.replace_missing(row);

        REQUIRE(fr1.at(_0) == 2022_y / January / 6);
        REQUIRE(fr1.at(_1) == 14.4);
        REQUIRE(fr1.at(_2) == true);
    }

    SECTION("third bool replacemant")
    {
        frame_row<mi<year_month_day>, mi<double>, mi<bool>> fr1;
        fr1.at(_0) = 2022_y / January / 6;
        fr1.at(_1) = 15.5;
        fr1.at(_2) = missing;

        fr1.replace_missing(row);

        REQUIRE(fr1.at(_0) == 2022_y / January / 6);
        REQUIRE(fr1.at(_1) == 15.5);
        REQUIRE(fr1.at(_2) == false);
    }

    SECTION("first and last replacemant")
    {
        frame_row<mi<year_month_day>, mi<double>, mi<bool>> fr1;
        fr1.at(_0) = missing;
        fr1.at(_1) = 15.5;
        fr1.at(_2) = missing;

        fr1.replace_missing(row);

        REQUIRE(fr1.at(_0) == 2022_y / January / 5);
        REQUIRE(fr1.at(_1) == 15.5);
        REQUIRE(fr1.at(_2) == false);
    }

    SECTION("full replacemant")
    {
        frame_row<mi<year_month_day>, mi<double>, mi<bool>> fr1;
        fr1.at(_0) = missing;
        fr1.at(_1) = missing;
        fr1.at(_2) = missing;

        fr1.replace_missing(row);

        REQUIRE(fr1.at(_0) == 2022_y / January / 5);
        REQUIRE(fr1.at(_1) == 14.4);
        REQUIRE(fr1.at(_2) == false);
    }
}

TEST_CASE("fill_forward", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    auto f2 = f1.allow_missing();
    f2.set_column_names("date", "temperature", "rain");
    f2.push_back(2022_y / January / 1, 8.9, missing);
    f2.push_back(missing, 10.0, false);
    f2.push_back(missing, missing, true);
    f2.push_back(2022_y / January / 4, 12.2, missing);
    f2.push_back(missing, missing, false);
    f2.push_back(2022_y / January / 6, 15.5, false);
    REQUIRE(f2.size() == 6);
    auto f3 = f2.fill_forward();

    auto it = f2.cbegin();
    REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
    REQUIRE((it + 1)->at(_0) == missing);
    REQUIRE((it + 2)->at(_0) == missing);
    REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 4);
    REQUIRE((it + 4)->at(_0) == missing);
    REQUIRE((it + 5)->at(_0) == 2022_y / 1 / 6);

    REQUIRE((it + 0)->at(_1) == 8.9);
    REQUIRE((it + 1)->at(_1) == 10.0);
    REQUIRE((it + 2)->at(_1) == missing);
    REQUIRE((it + 3)->at(_1) == 12.2);
    REQUIRE((it + 4)->at(_1) == missing);
    REQUIRE((it + 5)->at(_1) == 15.5);

    REQUIRE((it + 0)->at(_2) == missing);
    REQUIRE((it + 1)->at(_2) == false);
    REQUIRE((it + 2)->at(_2) == true);
    REQUIRE((it + 3)->at(_2) == missing);
    REQUIRE((it + 4)->at(_2) == false);
    REQUIRE((it + 5)->at(_2) == false);

    it = f3.cbegin();
    REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
    REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 1);
    REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 1);
    REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 4);
    REQUIRE((it + 4)->at(_0) == 2022_y / 1 / 4);
    REQUIRE((it + 5)->at(_0) == 2022_y / 1 / 6);

    REQUIRE((it + 0)->at(_1) == 8.9);
    REQUIRE((it + 1)->at(_1) == 10.0);
    REQUIRE((it + 2)->at(_1) == 10.0);
    REQUIRE((it + 3)->at(_1) == 12.2);
    REQUIRE((it + 4)->at(_1) == 12.2);
    REQUIRE((it + 5)->at(_1) == 15.5);

    REQUIRE((it + 0)->at(_2) == missing);
    REQUIRE((it + 1)->at(_2) == false);
    REQUIRE((it + 2)->at(_2) == true);
    REQUIRE((it + 3)->at(_2) == true);
    REQUIRE((it + 4)->at(_2) == false);
    REQUIRE((it + 5)->at(_2) == false);
}

TEST_CASE("fill_backward", "[frame]")
{
    frame<year_month_day, double, bool> f1;
    auto f2 = f1.allow_missing();
    f2.set_column_names("date", "temperature", "rain");
    f2.push_back(2022_y / January / 1, 8.9, missing);
    f2.push_back(missing, 10.0, false);
    f2.push_back(missing, missing, true);
    f2.push_back(2022_y / January / 4, 12.2, missing);
    f2.push_back(missing, missing, false);
    f2.push_back(2022_y / January / 6, 15.5, false);
    REQUIRE(f2.size() == 6);
    auto f3 = f2.fill_backward();

    auto it = f2.cbegin();
    REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
    REQUIRE((it + 1)->at(_0) == missing);
    REQUIRE((it + 2)->at(_0) == missing);
    REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 4);
    REQUIRE((it + 4)->at(_0) == missing);
    REQUIRE((it + 5)->at(_0) == 2022_y / 1 / 6);

    REQUIRE((it + 0)->at(_1) == 8.9);
    REQUIRE((it + 1)->at(_1) == 10.0);
    REQUIRE((it + 2)->at(_1) == missing);
    REQUIRE((it + 3)->at(_1) == 12.2);
    REQUIRE((it + 4)->at(_1) == missing);
    REQUIRE((it + 5)->at(_1) == 15.5);

    REQUIRE((it + 0)->at(_2) == missing);
    REQUIRE((it + 1)->at(_2) == false);
    REQUIRE((it + 2)->at(_2) == true);
    REQUIRE((it + 3)->at(_2) == missing);
    REQUIRE((it + 4)->at(_2) == false);
    REQUIRE((it + 5)->at(_2) == false);

    it = f3.cbegin();
    REQUIRE((it + 0)->at(_0) == 2022_y / 1 / 1);
    REQUIRE((it + 1)->at(_0) == 2022_y / 1 / 4);
    REQUIRE((it + 2)->at(_0) == 2022_y / 1 / 4);
    REQUIRE((it + 3)->at(_0) == 2022_y / 1 / 4);
    REQUIRE((it + 4)->at(_0) == 2022_y / 1 / 6);
    REQUIRE((it + 5)->at(_0) == 2022_y / 1 / 6);

    REQUIRE((it + 0)->at(_1) == 8.9);
    REQUIRE((it + 1)->at(_1) == 10.0);
    REQUIRE((it + 2)->at(_1) == 12.2);
    REQUIRE((it + 3)->at(_1) == 12.2);
    REQUIRE((it + 4)->at(_1) == 15.5);
    REQUIRE((it + 5)->at(_1) == 15.5);

    REQUIRE((it + 0)->at(_2) == false);
    REQUIRE((it + 1)->at(_2) == false);
    REQUIRE((it + 2)->at(_2) == true);
    REQUIRE((it + 3)->at(_2) == false);
    REQUIRE((it + 4)->at(_2) == false);
    REQUIRE((it + 5)->at(_2) == false);
}

TEST_CASE("stddev", "[frame]")
{
    SECTION("double")
    {
        series<double> s1{ 1, 2, 3, 4, 5 };
        REQUIRE(s1.stddev() == Approx(1.414213));

        series<double> s2{ 2, 3, 4 };
        REQUIRE(s2.stddev() == Approx(0.816497));

        series<double> s3{ -1, -2, -3, -4, -5 };
        REQUIRE(s3.stddev() == Approx(1.414213));

        series<double> s4{ -2, -3, -4 };
        REQUIRE(s4.stddev() == Approx(0.816497));
    }

    SECTION("int")
    {
        series<int> s1{ 1, 2, 3, 4, 5 };
        REQUIRE(s1.stddev() == Approx(1.414213));

        series<int> s2{ 2, 3, 4 };
        REQUIRE(s2.stddev() == Approx(0.816497));

        series<int> s3{ -1, -2, -3, -4, -5 };
        REQUIRE(s3.stddev() == Approx(1.414213));

        series<int> s4{ -2, -3, -4 };
        REQUIRE(s4.stddev() == Approx(0.816497));
    }
}

template<typename T>
class TD;

TEST_CASE("minmax", "[frame]")
{
    SECTION("double")
    {
        {
            frame<double, int> s;
            s.push_back(1, 0);
            s.push_back(2, 0);
            s.push_back(3, 0);
            s.push_back(4, 0);
            s.push_back(5, 0);
            auto [minv, maxv] = s.minmax(_0);
            REQUIRE(minv == 1);
            REQUIRE(maxv == 5);
        }

        {
            frame<double, int> s;
            s.push_back(5, 0);
            s.push_back(2, 0);
            s.push_back(3, 0);
            s.push_back(1, 0);
            s.push_back(4, 0);
            auto [minv, maxv] = s.minmax(_0);
            REQUIRE(minv == 1);
            REQUIRE(maxv == 5);
        }

        {
            frame<double, int> s;
            s.push_back(-1, 0);
            s.push_back(-2, 0);
            s.push_back(-3, 0);
            s.push_back(-4, 0);
            s.push_back(-5, 0);
            auto [minv, maxv] = s.minmax(_0);
            REQUIRE(minv == -5);
            REQUIRE(maxv == -1);
        }

        {
            frame<double, int> s;
            s.push_back(-5, 0);
            s.push_back(-2, 0);
            s.push_back(-3, 0);
            s.push_back(-1, 0);
            s.push_back(-4, 0);
            auto [minv, maxv] = s.minmax(_0);
            REQUIRE(minv == -5);
            REQUIRE(maxv == -1);
        }
    }

    SECTION("int")
    {
        {
            frame<double, int> s;
            s.push_back(0, 1);
            s.push_back(0, 2);
            s.push_back(0, 3);
            s.push_back(0, 4);
            s.push_back(0, 5);
            auto [minv, maxv] = s.minmax(_1);
            REQUIRE(minv == 1);
            REQUIRE(maxv == 5);
        }

        {
            frame<double, int> s;
            s.push_back(0, 5);
            s.push_back(0, 2);
            s.push_back(0, 3);
            s.push_back(0, 1);
            s.push_back(0, 4);
            auto [minv, maxv] = s.minmax(_1);
            REQUIRE(minv == 1);
            REQUIRE(maxv == 5);
        }

        {
            frame<double, int> s;
            s.push_back(0, -1);
            s.push_back(0, -2);
            s.push_back(0, -3);
            s.push_back(0, -4);
            s.push_back(0, -5);
            auto [minv, maxv] = s.minmax(_1);
            REQUIRE(minv == -5);
            REQUIRE(maxv == -1);
        }

        {
            frame<double, int> s;
            s.push_back(0, -5);
            s.push_back(0, -2);
            s.push_back(0, -3);
            s.push_back(0, -1);
            s.push_back(0, -4);
            auto [minv, maxv] = s.minmax(_1);
            REQUIRE(minv == -5);
            REQUIRE(maxv == -1);
        }
    }

}

TEST_CASE("complex", "[frame]")
{
    SECTION("cmplx")
    {
        frame<double, year_month_day, double> f1;
        f1.set_column_names("real", "date", "imag");
        f1.push_back( 8.9, 2022_y / January / 1, 10.2);
        f1.push_back(10.2, 2022_y / January / 2, 10.2);
        f1.push_back(11.1, 2022_y / January / 3,  7.7);
        f1.push_back(12.2, 2022_y / January / 4, 10.2);
        f1.push_back(13.3, 2022_y / January / 5, 10.2);
        f1.push_back(14.4, 2022_y / January / 6,  7.7);
        f1.push_back(15.5, 2022_y / January / 7, 10.2);
        f1.push_back( 9.1, 2022_y / January / 8,  7.7);
        f1.push_back( 9.3, 2022_y / January / 9, 10.2);

        auto f2 = f1.prepend_column<complex<double>>("cmplx", cmplx<double>(_0, _2));
        auto it = f2.cbegin();
        REQUIRE((it + 0)->at(_0) ==  8.9 + 10.2i);
        REQUIRE((it + 1)->at(_0) == 10.2 + 10.2i);
        REQUIRE((it + 2)->at(_0) == 11.1 +  7.7i);
        REQUIRE((it + 3)->at(_0) == 12.2 + 10.2i);
        REQUIRE((it + 4)->at(_0) == 13.3 + 10.2i);
        REQUIRE((it + 5)->at(_0) == 14.4 +  7.7i);
        REQUIRE((it + 6)->at(_0) == 15.5 + 10.2i);
        REQUIRE((it + 7)->at(_0) ==  9.1 +  7.7i);
        REQUIRE((it + 8)->at(_0) ==  9.3 + 10.2i);

        REQUIRE((it + 0)->at(_1) ==  8.9);
        REQUIRE((it + 1)->at(_1) == 10.2);
        REQUIRE((it + 2)->at(_1) == 11.1);
        REQUIRE((it + 3)->at(_1) == 12.2);
        REQUIRE((it + 4)->at(_1) == 13.3);
        REQUIRE((it + 5)->at(_1) == 14.4);
        REQUIRE((it + 6)->at(_1) == 15.5);
        REQUIRE((it + 7)->at(_1) ==  9.1);
        REQUIRE((it + 8)->at(_1) ==  9.3);

        REQUIRE((it + 0)->at(_2) == 2022_y / January / 1);
        REQUIRE((it + 1)->at(_2) == 2022_y / January / 2);
        REQUIRE((it + 2)->at(_2) == 2022_y / January / 3);
        REQUIRE((it + 3)->at(_2) == 2022_y / January / 4);
        REQUIRE((it + 4)->at(_2) == 2022_y / January / 5);
        REQUIRE((it + 5)->at(_2) == 2022_y / January / 6);
        REQUIRE((it + 6)->at(_2) == 2022_y / January / 7);
        REQUIRE((it + 7)->at(_2) == 2022_y / January / 8);
        REQUIRE((it + 8)->at(_2) == 2022_y / January / 9);

        REQUIRE((it + 0)->at(_3) == 10.2);
        REQUIRE((it + 1)->at(_3) == 10.2);
        REQUIRE((it + 2)->at(_3) ==  7.7);
        REQUIRE((it + 3)->at(_3) == 10.2);
        REQUIRE((it + 4)->at(_3) == 10.2);
        REQUIRE((it + 5)->at(_3) ==  7.7);
        REQUIRE((it + 6)->at(_3) == 10.2);
        REQUIRE((it + 7)->at(_3) ==  7.7);
        REQUIRE((it + 8)->at(_3) == 10.2);
    }

    SECTION("realimag")
    {
        frame<complex<double>, year_month_day> f1;
        f1.set_column_names("cmplx", "date");
        f1.push_back( 8.9 + 10.2i, 2022_y / January / 1);
        f1.push_back(10.2 + 10.2i, 2022_y / January / 2);
        f1.push_back(11.1 +  7.7i, 2022_y / January / 3);
        f1.push_back(12.2 + 10.2i, 2022_y / January / 4);
        f1.push_back(13.3 + 10.2i, 2022_y / January / 5);
        f1.push_back(14.4 +  7.7i, 2022_y / January / 6);
        f1.push_back(15.5 + 10.2i, 2022_y / January / 7);
        f1.push_back( 9.1 +  7.7i, 2022_y / January / 8);
        f1.push_back( 9.3 + 10.2i, 2022_y / January / 9);

        auto f2 = f1.append_column<double>("real", real<double>(_0));
        auto f3 = f2.append_column<double>("imag", imag<double>(_0));
        auto it = f3.cbegin();
        REQUIRE((it + 0)->at(_0) ==  8.9 + 10.2i);
        REQUIRE((it + 1)->at(_0) == 10.2 + 10.2i);
        REQUIRE((it + 2)->at(_0) == 11.1 +  7.7i);
        REQUIRE((it + 3)->at(_0) == 12.2 + 10.2i);
        REQUIRE((it + 4)->at(_0) == 13.3 + 10.2i);
        REQUIRE((it + 5)->at(_0) == 14.4 +  7.7i);
        REQUIRE((it + 6)->at(_0) == 15.5 + 10.2i);
        REQUIRE((it + 7)->at(_0) ==  9.1 +  7.7i);
        REQUIRE((it + 8)->at(_0) ==  9.3 + 10.2i);

        REQUIRE((it + 0)->at(_1) == 2022_y / January / 1);
        REQUIRE((it + 1)->at(_1) == 2022_y / January / 2);
        REQUIRE((it + 2)->at(_1) == 2022_y / January / 3);
        REQUIRE((it + 3)->at(_1) == 2022_y / January / 4);
        REQUIRE((it + 4)->at(_1) == 2022_y / January / 5);
        REQUIRE((it + 5)->at(_1) == 2022_y / January / 6);
        REQUIRE((it + 6)->at(_1) == 2022_y / January / 7);
        REQUIRE((it + 7)->at(_1) == 2022_y / January / 8);
        REQUIRE((it + 8)->at(_1) == 2022_y / January / 9);

        REQUIRE((it + 0)->at(_2) ==  8.9);
        REQUIRE((it + 1)->at(_2) == 10.2);
        REQUIRE((it + 2)->at(_2) == 11.1);
        REQUIRE((it + 3)->at(_2) == 12.2);
        REQUIRE((it + 4)->at(_2) == 13.3);
        REQUIRE((it + 5)->at(_2) == 14.4);
        REQUIRE((it + 6)->at(_2) == 15.5);
        REQUIRE((it + 7)->at(_2) ==  9.1);
        REQUIRE((it + 8)->at(_2) ==  9.3);

        REQUIRE((it + 0)->at(_3) == 10.2);
        REQUIRE((it + 1)->at(_3) == 10.2);
        REQUIRE((it + 2)->at(_3) ==  7.7);
        REQUIRE((it + 3)->at(_3) == 10.2);
        REQUIRE((it + 4)->at(_3) == 10.2);
        REQUIRE((it + 5)->at(_3) ==  7.7);
        REQUIRE((it + 6)->at(_3) == 10.2);
        REQUIRE((it + 7)->at(_3) ==  7.7);
        REQUIRE((it + 8)->at(_3) == 10.2);
    }
}

TEST_CASE("remove_column", "[frame]")
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

    frame<double, int> fa = f1.remove_column(_0);
    frame<year_month_day, int> fb = f1.remove_column(_1);
    frame<year_month_day, double> fc = f1.remove_column(_2);

    {
        auto it = fa.cbegin();
        REQUIRE((it + 0)->at(_0) == 8.9);
        REQUIRE((it + 1)->at(_0) == 10.0);
        REQUIRE((it + 2)->at(_0) == 11.1);
        REQUIRE((it + 3)->at(_0) == 12.2);
        REQUIRE((it + 4)->at(_0) == 13.3);
        REQUIRE((it + 5)->at(_0) == 14.4);
        REQUIRE((it + 6)->at(_0) == 15.5);
        REQUIRE((it + 7)->at(_0) == 9.1);
        REQUIRE((it + 8)->at(_0) == 9.3);
        REQUIRE((it + 0)->at(_1) == 10);
        REQUIRE((it + 1)->at(_1) == 10);
        REQUIRE((it + 2)->at(_1) == 7);
        REQUIRE((it + 3)->at(_1) == 10);
        REQUIRE((it + 4)->at(_1) == 10);
        REQUIRE((it + 5)->at(_1) == 7);
        REQUIRE((it + 6)->at(_1) == 10);
        REQUIRE((it + 7)->at(_1) == 7);
        REQUIRE((it + 8)->at(_1) == 10);
    }

    {
        auto it = fb.cbegin();
        REQUIRE((it + 0)->at(_0) == 2022_y / January / 1);
        REQUIRE((it + 1)->at(_0) == 2022_y / January / 2);
        REQUIRE((it + 2)->at(_0) == 2022_y / January / 3);
        REQUIRE((it + 3)->at(_0) == 2022_y / January / 4);
        REQUIRE((it + 4)->at(_0) == 2022_y / January / 5);
        REQUIRE((it + 5)->at(_0) == 2022_y / January / 6);
        REQUIRE((it + 6)->at(_0) == 2022_y / January / 7);
        REQUIRE((it + 7)->at(_0) == 2022_y / January / 8);
        REQUIRE((it + 8)->at(_0) == 2022_y / January / 9);
        REQUIRE((it + 0)->at(_1) == 10);
        REQUIRE((it + 1)->at(_1) == 10);
        REQUIRE((it + 2)->at(_1) == 7);
        REQUIRE((it + 3)->at(_1) == 10);
        REQUIRE((it + 4)->at(_1) == 10);
        REQUIRE((it + 5)->at(_1) == 7);
        REQUIRE((it + 6)->at(_1) == 10);
        REQUIRE((it + 7)->at(_1) == 7);
        REQUIRE((it + 8)->at(_1) == 10);
    }

    {
        auto it = fc.cbegin();
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
    }
}

TEST_CASE("operator*", "[frame]")
{
    SECTION("legitimate")
    {
        frame<double, year_month_day, double> f1;
        f1.set_column_names("real", "date", "imag");
        f1.push_back( 8.9, 2022_y / January / 1, 10.2);

        REQUIRE(*f1[_0] == 8.9);
        REQUIRE(*f1[_1] == 2022_y / January / 1);
        REQUIRE(*f1[_2] == 10.2);

        f1.push_back(10.2, 2022_y / January / 2, 10.2);
        f1.push_back(11.1, 2022_y / January / 3,  7.7);
        f1.push_back(12.2, 2022_y / January / 4, 10.2);
        f1.push_back(13.3, 2022_y / January / 5, 10.2);
        f1.push_back(14.4, 2022_y / January / 6,  7.7);

        REQUIRE(*f1[_0] == 8.9);
        REQUIRE(*f1[_1] == 2022_y / January / 1);
        REQUIRE(*f1[_2] == 10.2);

        REQUIRE(*f1[_0][3] == 12.2);
        REQUIRE(*f1[_1][3] == 2022_y / January / 4);
        REQUIRE(*f1[_2][3] == 10.2);

        REQUIRE(*f1[3][_0] == 12.2);
        REQUIRE(*f1[3][_1] == 2022_y / January / 4);
        REQUIRE(*f1[3][_2] == 10.2);
    }

    SECTION("invalid")
    {
        frame<double, year_month_day, double> f1;
        f1.set_column_names("real", "date", "imag");

        REQUIRE_THROWS_AS(*f1[_0], std::out_of_range);
        REQUIRE_THROWS_AS(*f1[_1], std::out_of_range);
        REQUIRE_THROWS_AS(*f1[_2], std::out_of_range);
    }
}

TEST_CASE("operator()", "[frame]")
{
    SECTION("legitimate")
    {
        auto func = [](const series<year_month_day>& s) { 
            REQUIRE(s.size() == 5);
            REQUIRE(s.name() == "date");
            REQUIRE(s[0] == 2022_y / January / 1);
            REQUIRE(s[1] == 2022_y / January / 2);
            REQUIRE(s[2] == 2022_y / January / 3);
            REQUIRE(s[3] == 2022_y / January / 4);
            REQUIRE(s[4] == 2022_y / January / 5);
        };
        
        frame<double, year_month_day, double> f1;
        f1.set_column_names("real", "date", "imag");
        f1.push_back( 8.9, 2022_y / January / 1, 10.2);
        f1.push_back(10.2, 2022_y / January / 2, 10.2);
        f1.push_back(11.1, 2022_y / January / 3,  7.7);
        f1.push_back(12.2, 2022_y / January / 4, 10.2);
        f1.push_back(13.3, 2022_y / January / 5, 10.2);

        func(f1[_1]);
    }
}

//template<typename Func, typename Arg>
//struct fnobj;
//
//template<typename R, typename FnArg, typename Arg>
//struct fnobj<R (FnArg), Arg>
//{
//    using is_expr = void;
//
//    using F = R(FnArg);
//
//    fnobj(F& f, const Arg& arg) 
//        : m_f( f ) 
//        , m_arg(arg)
//    {
//    }
//
//    R operator()() const
//    {
//        return std::apply(m_f, m_arg);
//    }
//
//    template<typename T>
//    R
//    operator()(
//        const typename std::vector<T>::iterator& begin,
//        const typename std::vector<T>::iterator& curr,
//        const typename std::vector<T>::iterator& end) const
//    {
//        Arg a = std::get<0>(m_arg).operator()(begin, curr, end);
//        return std::apply(m_f, a);
//    }
//
//    F& m_f;
//    std::tuple<Arg> m_arg;
//};
//
//int foo(int a)
//{
//    printf("foo(a=%d)\n", a);
//    return a+1;
//}
//
//TEST_CASE("fn", "[frame]")
//{
//    vector<float> arr{ 
//        1.0, 
//        1.0, 
//        2.0, 
//        3.0, 
//        5.0, 
//        8.0, 
//        13.0, 
//        21.0, 
//        34.0, 
//        55.0, 
//        89.0 };
//
//    fnobj<decltype(foo), decltype(_0)> fn(foo, _0);
//    (void)fn;
//    auto res = fn.operator()<float>(arr.begin(), arr.begin(), arr.end());
//    (void)res;
//}

