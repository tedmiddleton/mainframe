#include <iostream>
#include <ostream>
#include <x86intrin.h>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mainframe/expression.hpp>
#include <mainframe/frame.hpp>
#include "date.h"
#include "debug_cout.hpp"

using namespace std;
using namespace std::chrono;
using namespace std::literals;
using namespace date::literals;
using namespace date;
using namespace mf;

namespace std
{
    template<>
    struct hash<date::year_month_day>
    {
        size_t operator()( const date::year_month_day& ymd ) const noexcept
        {
            auto hi = std::hash<int>{};
            auto hu = std::hash<unsigned>{};
            auto y = hi(static_cast<int>(ymd.year()));
            auto m = hu(static_cast<unsigned>(ymd.month()));
            auto d = hu(static_cast<unsigned>(ymd.day()));
            return y ^ m ^ d;
        }
    };

    template<>
    struct hash<date::year_month>
    {
        size_t operator()( const date::year_month& ym ) const noexcept
        {
            auto hi = std::hash<int>{};
            auto hu = std::hash<unsigned>{};
            auto y = hi(static_cast<int>(ym.year()));
            auto m = hu(static_cast<unsigned>(ym.month()));
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
    friend ostream & operator<<( ostream & o, const HasStreamOp & tc )
    {
        o << "HasStreamOp{d=" << tc.d << ",i=" << tc.i << "}";
        return o;
    }
};

class TestType
{
public:
    TestType() = default;
    TestType( int x ) : 
        f1( (double)x ),
        f2( x )
    {}
    double getf1() { return f1; }
    int getf2() { return f2; }
private:
    double f1;
    int f2;

    friend ostream& operator<<( ostream&, const TestType& );
};

ostream& operator<<( ostream& o, const TestType& tt )
{
    o << "TestType{ f1=" << tt.f1 << ", f2=" << tt.f2 << " }";
    return o;
}

template<typename X>
class TestTypeX
{
public:
    TestTypeX() = default;
    TestTypeX( X x ) : 
        f1( x )
    {}
    X getf1() { return f1; }
private:
    X f1;

    template<typename Y>
    friend ostream& operator<<( ostream&, const TestTypeX<Y>& );
};

template<typename X>
ostream& operator<<( ostream& o, const TestTypeX<X>& tt )
{
    o << "TestTypeX{ f1=" << tt.f1 << " }";
    return o;
}

TEST_CASE( "ctor" "[mainframe]" )
{
    frame<year_month_day, double, bool> f1;
    REQUIRE( f1.size() == 0 );
}

TEST_CASE( "ctor( const& )" "[mainframe]" )
{
    frame<year_month_day, double, bool> f1;
    REQUIRE( f1.size() == 0 );
    f1.push_back( 2022_y/January/2, 10.9, true );
    f1.push_back( 2022_y/January/3, 11.0, true );
    f1.push_back( 2022_y/January/4, 11.1, false );
    f1.push_back( 2022_y/January/5, 11.2, true );
    REQUIRE( f1.size() == 4 );
    frame<year_month_day, double, bool> f2( f1 );
    REQUIRE( f1.size() == 4 );
    REQUIRE( f2.size() == 4 );
}

TEST_CASE( "ctor( && )" "[mainframe]" )
{
    frame<year_month_day, double, bool> f1;
    REQUIRE( f1.size() == 0 );
    f1.push_back( 2022_y/January/2, 10.9, true );
    f1.push_back( 2022_y/January/3, 11.0, true );
    f1.push_back( 2022_y/January/4, 11.1, false );
    f1.push_back( 2022_y/January/5, 11.2, true );
    REQUIRE( f1.size() == 4 );
    frame<year_month_day, double, bool> f2( std::move(f1) );
    REQUIRE( f1.size() == 0 );
    REQUIRE( f2.size() == 4 );
}

TEST_CASE( "generic" "[mainframe]" )
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names( "date", "temperature", "rain" );
    f1.push_back( 2022_y/January/2, 10.9, true );
    f1.push_back( 2022_y/January/3, 11.0, true );
    f1.push_back( 2022_y/January/4, 11.1, false );
    f1.push_back( 2022_y/January/5, 11.2, true );
    REQUIRE( f1.column_name<0>() == "date" );
    REQUIRE( f1.column_name<1>() == "temperature" );
    REQUIRE( f1.column_name<2>() == "rain" );
    REQUIRE( f1.size() == 4 );

    auto f1i = f1.begin();
    auto f1e = f1.end();
    REQUIRE( (f1e - f1i) == 4 );

    frame<year_month_day, double, optional<bool>> f2;
    f2.push_back( 2022_y/January/2, 10.9, true );
    f2.push_back( 2022_y/January/3, 11.0, nullopt );
    f2.push_back( 2022_y/January/4, 11.1, false );
    f2.push_back( 2022_y/January/5, 11.2, true );
    f2.push_back( 2022_y/January/6, 11.3, true );
    f2.push_back( 2022_y/January/7, 11.2, nullopt );
    f2.push_back( 2022_y/January/8, 11.1, true );
    f2.push_back( 2022_y/January/9, 11.0, true );

    dout << "Contents of f2:\n";
    for ( auto row : f1 ) {
        dout << f1.column_name<0>() << "=" << row.get<0>() << ", "
            << f1.column_name<1>() << "=" << row.get<1>() << ", "
            << f1.column_name<2>() << "=" << row.get<2>() << "\n";
    }
}


