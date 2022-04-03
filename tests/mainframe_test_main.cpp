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

TEST_CASE( "ctor" "[frame]" )
{
    frame<year_month_day, double, bool> f1;
    REQUIRE( f1.size() == 0 );
}

TEST_CASE( "ctor( const& )" "[frame]" )
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

TEST_CASE( "ctor( && )" "[frame]" )
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

TEST_CASE( "begin()/end()", "[frame]" )
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
    REQUIRE( f1i->get<0>() == 2022_y/January/2 );
    REQUIRE( f1i->get<1>() == 10.9 );
    REQUIRE( f1i->get<2>() == true );
    f1i+=2;
    REQUIRE( f1i->get<0>() == 2022_y/January/4 );
    REQUIRE( f1i->get<1>() == 11.1 );
    REQUIRE( f1i->get<2>() == false );
    ++f1i;
    REQUIRE( f1i->get<0>() == 2022_y/January/5 );
    REQUIRE( f1i->get<1>() == 11.2 );
    REQUIRE( f1i->get<2>() == true );
    f1i++;
    REQUIRE( f1i == f1e );
    f1i--;
    REQUIRE( f1i->get<0>() == 2022_y/January/5 );
    REQUIRE( f1i->get<1>() == 11.2 );
    REQUIRE( f1i->get<2>() == true );
    f1i-=2;
    REQUIRE( f1i->get<0>() == 2022_y/January/3 );
    REQUIRE( f1i->get<1>() == 11.0 );
    REQUIRE( f1i->get<2>() == true );
    --f1i;
    REQUIRE( f1i->get<0>() == 2022_y/January/2 );
    REQUIRE( f1i->get<1>() == 10.9 );
    REQUIRE( f1i->get<2>() == true );
}

TEST_CASE( "rbegin()/rend()", "[frame]" )
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

    auto f1i = f1.rbegin();
    auto f1e = f1.rend();
    REQUIRE( (f1e - f1i) == 4 );
    REQUIRE( f1i->get<0>() == 2022_y/January/5 );
    REQUIRE( f1i->get<1>() == 11.2 );
    REQUIRE( f1i->get<2>() == true );
    f1i+=2;
    REQUIRE( f1i->get<0>() == 2022_y/January/3 );
    REQUIRE( f1i->get<1>() == 11.0 );
    REQUIRE( f1i->get<2>() == true );
    ++f1i;
    REQUIRE( f1i->get<0>() == 2022_y/January/2 );
    REQUIRE( f1i->get<1>() == 10.9 );
    REQUIRE( f1i->get<2>() == true );
    f1i++;
    REQUIRE( f1i == f1e );
    f1i--;
    REQUIRE( f1i->get<0>() == 2022_y/January/2 );
    REQUIRE( f1i->get<1>() == 10.9 );
    REQUIRE( f1i->get<2>() == true );
    f1i-=2;
    REQUIRE( f1i->get<0>() == 2022_y/January/4 );
    REQUIRE( f1i->get<1>() == 11.1 );
    REQUIRE( f1i->get<2>() == false );
    --f1i;
    REQUIRE( f1i->get<0>() == 2022_y/January/5 );
    REQUIRE( f1i->get<1>() == 11.2 );
    REQUIRE( f1i->get<2>() == true );
}

TEST_CASE( "empty()", "[frame]" )
{
    frame<year_month_day, double, bool> f1;
    REQUIRE( f1.empty() );
    f1.set_column_names( "date", "temperature", "rain" );
    REQUIRE( f1.empty() );
    f1.push_back( 2022_y/January/2, 10.9, true );
    REQUIRE( !f1.empty() );
    f1.push_back( 2022_y/January/3, 11.0, true );
    REQUIRE( !f1.empty() );
}

TEST_CASE( "size()/clear()", "[frame]" )
{
    frame<year_month_day, double, bool> f1;
    REQUIRE( f1.size() == 0 );
    f1.push_back( 2022_y/January/2, 10.9, true );
    REQUIRE( f1.size() == 1 );
    f1.clear();
    REQUIRE( f1.size() == 0 );
    f1.set_column_names( "date", "temperature", "rain" );
    REQUIRE( f1.size() == 0 );
    f1.push_back( 2022_y/January/2, 10.9, true );
    REQUIRE( f1.size() == 1 );
    f1.push_back( 2022_y/January/3, 11.0, true );
    REQUIRE( f1.size() == 2 );
}

template<typename T>
class TD;

TEST_CASE( "insert( pos first last )", "[frame]" )
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names( "date", "temperature", "rain" );
    f1.push_back( 2022_y/January/2, 10.0, false );
    f1.push_back( 2022_y/January/3, 11.1, false );
    f1.push_back( 2022_y/January/4, 12.2, false );
    REQUIRE( f1.size() == 3 );
    frame<year_month_day, double, bool> f2;
    f2.push_back( 2021_y/March/12, -110.9, true );
    f2.push_back( 2021_y/March/13, -111.0, true );
    f2.push_back( 2021_y/March/14, -112.1, true );
    REQUIRE( f2.size() == 3 );

    auto resit = f1.insert( f1.begin()+1, f2.begin(), f2.end() );
    REQUIRE( f1.size() == 6 );
    auto b = f1.begin();
    REQUIRE( resit == b+1 );
    REQUIRE( b->get<0>() == 2022_y/January/2 );
    REQUIRE( b->get<1>() == 10.0 );
    REQUIRE( b->get<2>() == false );
    b++;
    REQUIRE( b->get<0>() == 2021_y/March/12 );
    REQUIRE( b->get<1>() == -110.9 );
    REQUIRE( b->get<2>() == true );
    ++b;
    REQUIRE( b->get<0>() == 2021_y/March/13 );
    REQUIRE( b->get<1>() == -111.0 );
    REQUIRE( b->get<2>() == true );
    b+=1;
    REQUIRE( b->get<0>() == 2021_y/March/14 );
    REQUIRE( b->get<1>() == -112.1 );
    REQUIRE( b->get<2>() == true );
    b++;
    REQUIRE( b->get<0>() == 2022_y/January/3 );
    REQUIRE( b->get<1>() == 11.1 );
    REQUIRE( b->get<2>() == false );
    b++;
    REQUIRE( b->get<0>() == 2022_y/January/4 );
    REQUIRE( b->get<1>() == 12.2 );
    REQUIRE( b->get<2>() == false );
    b-=3;
    REQUIRE( b->get<0>() == 2021_y/March/13 );
    REQUIRE( b->get<1>() == -111.0 );
    REQUIRE( b->get<2>() == true );
}

TEST_CASE( "insert( pos value )", "[frame]" )
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names( "date", "temperature", "rain" );
    f1.push_back( 2022_y/January/2, 10.0, false );
    f1.push_back( 2022_y/January/3, 11.1, false );
    f1.push_back( 2022_y/January/4, 12.2, false );
    REQUIRE( f1.size() == 3 );
    auto resit = f1.insert( f1.begin()+1, 2021_y/March/14, -112.1, true );
    REQUIRE( resit == f1.begin()+1 );
    REQUIRE( f1.size() == 4 );
    resit = f1.insert( f1.begin()+1, 2021_y/March/13, -111.0, true );
    REQUIRE( resit == f1.begin()+1 );
    REQUIRE( f1.size() == 5 );
    resit = f1.insert( f1.begin()+1, 2021_y/March/12, -110.9, true );
    REQUIRE( resit == f1.begin()+1 );
    REQUIRE( f1.size() == 6 );
    auto b = f1.begin();
    REQUIRE( resit == b+1 );
    REQUIRE( b->get<0>() == 2022_y/January/2 );
    REQUIRE( b->get<1>() == 10.0 );
    REQUIRE( b->get<2>() == false );
    b++;
    REQUIRE( b->get<0>() == 2021_y/March/12 );
    REQUIRE( b->get<1>() == -110.9 );
    REQUIRE( b->get<2>() == true );
    ++b;
    REQUIRE( b->get<0>() == 2021_y/March/13 );
    REQUIRE( b->get<1>() == -111.0 );
    REQUIRE( b->get<2>() == true );
    b+=1;
    REQUIRE( b->get<0>() == 2021_y/March/14 );
    REQUIRE( b->get<1>() == -112.1 );
    REQUIRE( b->get<2>() == true );
    b++;
    REQUIRE( b->get<0>() == 2022_y/January/3 );
    REQUIRE( b->get<1>() == 11.1 );
    REQUIRE( b->get<2>() == false );
    b++;
    REQUIRE( b->get<0>() == 2022_y/January/4 );
    REQUIRE( b->get<1>() == 12.2 );
    REQUIRE( b->get<2>() == false );
    b-=3;
    REQUIRE( b->get<0>() == 2021_y/March/13 );
    REQUIRE( b->get<1>() == -111.0 );
    REQUIRE( b->get<2>() == true );
}

TEST_CASE( "insert( pos count value )", "[frame]" )
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names( "date", "temperature", "rain" );
    f1.push_back( 2022_y/January/2, 10.0, false );
    f1.push_back( 2022_y/January/3, 11.1, false );
    f1.push_back( 2022_y/January/4, 12.2, false );
    REQUIRE( f1.size() == 3 );
    auto resit = f1.insert( f1.begin()+1, 2, 2021_y/March/14, -112.1, true );
    REQUIRE( resit == f1.begin()+1 );
    REQUIRE( f1.size() == 5 );
    resit = f1.insert( f1.begin()+1, 1, 2021_y/March/13, -111.0, true );
    REQUIRE( resit == f1.begin()+1 );
    REQUIRE( f1.size() == 6 );
    resit = f1.insert( f1.begin()+1, 2, 2021_y/March/12, -110.9, true );
    REQUIRE( resit == f1.begin()+1 );
    REQUIRE( f1.size() == 8 );
    auto b = f1.begin();
    REQUIRE( resit == b+1 );
    REQUIRE( b->get<0>() == 2022_y/January/2 );
    REQUIRE( b->get<1>() == 10.0 );
    REQUIRE( b->get<2>() == false );
    b++;
    REQUIRE( b->get<0>() == 2021_y/March/12 );
    REQUIRE( b->get<1>() == -110.9 );
    REQUIRE( b->get<2>() == true );
    ++b;
    REQUIRE( b->get<0>() == 2021_y/March/12 );
    REQUIRE( b->get<1>() == -110.9 );
    REQUIRE( b->get<2>() == true );
    ++b;
    REQUIRE( b->get<0>() == 2021_y/March/13 );
    REQUIRE( b->get<1>() == -111.0 );
    REQUIRE( b->get<2>() == true );
    b+=1;
    REQUIRE( b->get<0>() == 2021_y/March/14 );
    REQUIRE( b->get<1>() == -112.1 );
    REQUIRE( b->get<2>() == true );
    b++;
    REQUIRE( b->get<0>() == 2021_y/March/14 );
    REQUIRE( b->get<1>() == -112.1 );
    REQUIRE( b->get<2>() == true );
    b++;
    REQUIRE( b->get<0>() == 2022_y/January/3 );
    REQUIRE( b->get<1>() == 11.1 );
    REQUIRE( b->get<2>() == false );
    b++;
    REQUIRE( b->get<0>() == 2022_y/January/4 );
    REQUIRE( b->get<1>() == 12.2 );
    REQUIRE( b->get<2>() == false );
    b-=4;
    REQUIRE( b->get<0>() == 2021_y/March/13 );
    REQUIRE( b->get<1>() == -111.0 );
    REQUIRE( b->get<2>() == true );
}

TEST_CASE( "erase( first last )", "[frame]" )
{
    SECTION( "begin" )
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names( "date", "temperature", "rain" );
        f1.push_back( 2022_y/January/2, 10.0, false );
        f1.push_back( 2022_y/January/3, 11.1, false );
        f1.push_back( 2022_y/January/4, 12.2, false );
        REQUIRE( f1.size() == 3 );
        auto resit = f1.erase( f1.begin(), f1.begin()+1 );
        REQUIRE( resit == f1.begin() );
        REQUIRE( f1.size() == 2 );
        REQUIRE( f1.begin()->get<0>() == 2022_y/January/3 );
        REQUIRE( f1.begin()->get<1>() == 11.1 );
        REQUIRE( f1.begin()->get<2>() == false );
        REQUIRE( (f1.begin() + 1)->get<0>() == 2022_y/January/4 );
        REQUIRE( (f1.begin() + 1)->get<1>() == 12.2 );
        REQUIRE( (f1.begin() + 1)->get<2>() == false );
    }

    SECTION( "begin multiple" )
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names( "date", "temperature", "rain" );
        f1.push_back( 2022_y/January/2, 10.0, false );
        f1.push_back( 2022_y/January/3, 11.1, false );
        f1.push_back( 2022_y/January/4, 12.2, false );
        REQUIRE( f1.size() == 3 );
        auto resit = f1.erase( f1.begin(), f1.begin()+2 );
        REQUIRE( resit == f1.begin() );
        REQUIRE( f1.size() == 1 );
        REQUIRE( f1.begin()->get<0>() == 2022_y/January/4 );
        REQUIRE( f1.begin()->get<1>() == 12.2 );
        REQUIRE( f1.begin()->get<2>() == false );
    }

    SECTION( "end 1" )
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names( "date", "temperature", "rain" );
        f1.push_back( 2022_y/January/2, 10.0, false );
        f1.push_back( 2022_y/January/3, 11.1, false );
        f1.push_back( 2022_y/January/4, 12.2, false );
        REQUIRE( f1.size() == 3 );
        auto resit = f1.erase( f1.end()-1, f1.end() );
        REQUIRE( resit == f1.end() );
        REQUIRE( f1.size() == 2 );
        REQUIRE( f1.begin()->get<0>() == 2022_y/January/2 );
        REQUIRE( f1.begin()->get<1>() == 10.0 );
        REQUIRE( f1.begin()->get<2>() == false );
        REQUIRE( (f1.begin() + 1)->get<0>() == 2022_y/January/3 );
        REQUIRE( (f1.begin() + 1)->get<1>() == 11.1 );
        REQUIRE( (f1.begin() + 1)->get<2>() == false );
    }
}

TEST_CASE( "erase( pos )", "[frame]" )
{
    SECTION( "begin" )
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names( "date", "temperature", "rain" );
        f1.push_back( 2022_y/January/2, 10.0, false );
        f1.push_back( 2022_y/January/3, 11.1, false );
        f1.push_back( 2022_y/January/4, 12.2, false );
        REQUIRE( f1.size() == 3 );
        auto resit = f1.erase( f1.begin() );
        REQUIRE( resit == f1.begin() );
        REQUIRE( f1.size() == 2 );
        REQUIRE( f1.begin()->get<0>() == 2022_y/January/3 );
        REQUIRE( f1.begin()->get<1>() == 11.1 );
        REQUIRE( f1.begin()->get<2>() == false );
        REQUIRE( (f1.begin() + 1)->get<0>() == 2022_y/January/4 );
        REQUIRE( (f1.begin() + 1)->get<1>() == 12.2 );
        REQUIRE( (f1.begin() + 1)->get<2>() == false );
    }

    SECTION( "end -1" )
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names( "date", "temperature", "rain" );
        f1.push_back( 2022_y/January/2, 10.0, false );
        f1.push_back( 2022_y/January/3, 11.1, false );
        f1.push_back( 2022_y/January/4, 12.2, false );
        REQUIRE( f1.size() == 3 );
        auto resit = f1.erase( f1.end()-1 );
        REQUIRE( resit == f1.end() );
        REQUIRE( f1.size() == 2 );
        REQUIRE( f1.begin()->get<0>() == 2022_y/January/2 );
        REQUIRE( f1.begin()->get<1>() == 10.0 );
        REQUIRE( f1.begin()->get<2>() == false );
        REQUIRE( (f1.begin() + 1)->get<0>() == 2022_y/January/3 );
        REQUIRE( (f1.begin() + 1)->get<1>() == 11.1 );
        REQUIRE( (f1.begin() + 1)->get<2>() == false );
    }
}

TEST_CASE( "push_back() pop_back()", "[frame]" )
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names( "date", "temperature", "rain" );
    f1.push_back( 2022_y/January/2, 10.0, false );
    f1.push_back( 2022_y/January/3, 11.1, false );
    f1.push_back( 2022_y/January/4, 12.2, false );
    REQUIRE( f1.size() == 3 );
    REQUIRE( f1.begin()->get<0>() == 2022_y/January/2 );
    REQUIRE( f1.begin()->get<1>() == 10.0 );
    REQUIRE( f1.begin()->get<2>() == false );
    REQUIRE( (f1.begin() + 1)->get<0>() == 2022_y/January/3 );
    REQUIRE( (f1.begin() + 1)->get<1>() == 11.1 );
    REQUIRE( (f1.begin() + 1)->get<2>() == false );
    REQUIRE( (f1.begin() + 2)->get<0>() == 2022_y/January/4 );
    REQUIRE( (f1.begin() + 2)->get<1>() == 12.2 );
    REQUIRE( (f1.begin() + 2)->get<2>() == false );
    f1.pop_back();
    f1.pop_back();
    REQUIRE( f1.size() == 1 );
    REQUIRE( f1.begin()->get<0>() == 2022_y/January/2 );
    REQUIRE( f1.begin()->get<1>() == 10.0 );
    REQUIRE( f1.begin()->get<2>() == false );
}

TEST_CASE( "resize()", "[frame]" )
{
    SECTION( "bigger" )
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names( "date", "temperature", "rain" );
        f1.push_back( 2022_y/January/2, 10.0, false );
        f1.push_back( 2022_y/January/3, 11.1, false );
        f1.push_back( 2022_y/January/4, 12.2, false );
        REQUIRE( f1.size() == 3 );
        f1.resize( f1.size() + 1 );
        REQUIRE( f1.size() == 4 );
        REQUIRE( (f1.begin() + 3)->get<0>() == 0_y/0/0 );
        REQUIRE( (f1.begin() + 3)->get<1>() == 0.0 );
        REQUIRE( (f1.begin() + 3)->get<2>() == false );
    }

    SECTION( "smaller" )
    {
        frame<year_month_day, double, bool> f1;
        f1.set_column_names( "date", "temperature", "rain" );
        f1.push_back( 2022_y/January/2, 10.0, false );
        f1.push_back( 2022_y/January/3, 11.1, false );
        f1.push_back( 2022_y/January/4, 12.2, false );
        REQUIRE( f1.size() == 3 );
        f1.resize( f1.size() - 1 );
        REQUIRE( f1.size() == 2 );
        REQUIRE( f1.begin()->get<0>() == 2022_y/January/2 );
        REQUIRE( f1.begin()->get<1>() == 10.0 );
        REQUIRE( f1.begin()->get<2>() == false );
        REQUIRE( (f1.begin() + 1)->get<0>() == 2022_y/January/3 );
        REQUIRE( (f1.begin() + 1)->get<1>() == 11.1 );
        REQUIRE( (f1.begin() + 1)->get<2>() == false );
    }
}

TEST_CASE( "columns() to_frame()", "[frame]" )
{
    frame<year_month_day, double, bool> f1;
    f1.set_column_names( "date", "temperature", "rain" );
    f1.push_back( 2022_y/January/2, 10.0, false );
    f1.push_back( 2022_y/January/3, 11.1, false );
    f1.push_back( 2022_y/January/4, 12.2, false );
    auto f2 = f1.columns( "temperature", "date" )
        .to_frame<double, year_month_day>();
    REQUIRE( f2.num_columns() == 2 );
    REQUIRE( f2.column_name<0>() == "temperature" );
    REQUIRE( f2.column_name<1>() == "date" );
    REQUIRE( f2.begin()->get<0>() == 10.0 );
    REQUIRE( f2.begin()->get<1>() == 2022_y/January/2 );
    REQUIRE( (f2.begin() + 1)->get<0>() == 11.1 );
    REQUIRE( (f2.begin() + 1)->get<1>() == 2022_y/January/3 );
    REQUIRE( (f2.begin() + 2)->get<0>() == 12.2 );
    REQUIRE( (f2.begin() + 2)->get<1>() == 2022_y/January/4 );
}

