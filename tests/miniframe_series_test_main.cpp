#include <iostream>
#include <ostream>
#include <x86intrin.h>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <miniframe/series.hpp>
#include "date.h"
#include "debug_cout.hpp"

using namespace std;
using namespace std::chrono;
using namespace std::literals;
using namespace date::literals;
using namespace date;
using namespace mf;

template<class T>
class TD;

class foo
{
public:
    foo( const char * str )
    {
        m_str = strdup( str );
    }
    foo( const foo& other )
    {
        m_str = strdup( other.m_str );
    }
    foo( foo&& other )
        : m_str( strdup( "EMPTY" ) )
    {
        std::swap( m_str, other.m_str );
    }
    foo& operator=( const foo& other )
    {
        foo f{ other };
        std::swap( m_str, f.m_str );
        return *this;
    }
    bool operator==( const char* str ) const
    {
        return std::strcmp( m_str, str ) == 0;
    }
    bool operator==( const std::string& str ) const
    {
        return std::string{ m_str } == str;
    }
    bool operator==( const foo& other ) const
    {
        return std::string{ m_str } == std::string{ other.m_str };
    }

    virtual ~foo()
    {
        free( m_str );
    }

    friend std::ostream& operator<<( std::ostream& o, const foo& f );

    char * m_str;
};

std::ostream& operator<<( std::ostream& o, const foo& f )
{
    if ( f.m_str == nullptr ) {
        o << "foo{ m_str=nullptr }";
    }
    else {
        o << "foo{ m_str=\"" << f.m_str << "\" }";
    }
    return o;
}

TEST_CASE( "ctor()", "[series]" )
{
    series<foo> s1;
    REQUIRE( s1.name().empty() );
    REQUIRE( s1.size() == 0 );
}

TEST_CASE( "ctor( count value )", "[series]" )
{
    foo f1{ "f1" };
    series<foo> s1( 3, f1 );
    REQUIRE( s1.size() == 3 );
}

TEST_CASE( "ctor( count )", "[series]" )
{
    series<double> s1( 3 );
    REQUIRE( s1.size() == 3 );
}

TEST_CASE( "ctor( begin end )", "[series]" )
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    foo f4{ "f4" };
    vector<foo> v1{ f0, f1, f2, f3, f4 };
    series<foo> s1( v1.begin(), v1.end() );
    REQUIRE( s1.size() == 5 );
}

TEST_CASE( "ctor( const& )", "[series]" )
{
    series<double> s1( 5 );
    REQUIRE( s1.size() == 5 );
    series<double> s2( s1 );
    REQUIRE( s1.size() == 5 );
    REQUIRE( s2.size() == 5 );
    REQUIRE( s1.use_count() == 2 );
    REQUIRE( s2.use_count() == 2 );
}

TEST_CASE( "ctor( && )", "[series]" )
{
    series<double> s1( 5 );
    REQUIRE( s1.size() == 5 );
    series<double> s2( std::move( s1 ) );
    REQUIRE( s1.size() == 0 );
    REQUIRE( s2.size() == 5 );
    REQUIRE( s1.use_count() == 1 );
    REQUIRE( s2.use_count() == 1 );
}

TEST_CASE( "ctor( initializer_list )", "[series]" )
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    foo f4{ "f4" };
    series<foo> s1{ f0, f1, f2, f3, f4 };
    REQUIRE( s1.size() == 5 );
}

TEST_CASE( "operator=( const& )", "[series]" )
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    series<foo> s1( 66, f1 );
    series<foo> s2( 33, f2 );
    REQUIRE(s1.size() == 66);
    REQUIRE(s1.at(0) == f1);
    REQUIRE(s1.at(65) == f1);
    REQUIRE(s2.at(0) == f2);
    REQUIRE(s2.at(32) == f2);
    REQUIRE(s2.size() == 33);
    REQUIRE(s1.use_count() == 1);
    REQUIRE(s2.use_count() == 1);
    s2 = s1;
    REQUIRE(s1.size() == 66);
    REQUIRE(s2.size() == 66);
    REQUIRE(s1.at(0) == f1);
    REQUIRE(s1.at(65) == f1);
    REQUIRE(s2.at(0) == f1);
    REQUIRE(s2.at(65) == f1);
    REQUIRE(s1.use_count() == 2);
    REQUIRE(s2.use_count() == 2);
}

TEST_CASE( "operator=( && )", "[series]" )
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    series<foo> s1( 66, f1 );
    series<foo> s2( 33, f2 );
    REQUIRE(s1.size() == 66);
    REQUIRE(s2.size() == 33);
    REQUIRE(s1.at(0) == f1);
    REQUIRE(s1.at(65) == f1);
    REQUIRE(s2.at(0) == f2);
    REQUIRE(s2.at(32) == f2);
    REQUIRE(s1.use_count() == 1);
    REQUIRE(s2.use_count() == 1);
    s2 = std::move( s1 );
    REQUIRE(s1.size() == 0);
    REQUIRE(s2.size() == 66);
    REQUIRE(s2.at(0) == f1);
    REQUIRE(s2.at(65) == f1);
    REQUIRE(s1.use_count() == 1);
    REQUIRE(s2.use_count() == 1);
}

TEST_CASE( "operator=( initializer_list )", "[series]" )
{
    series<int> ints1( 66 );
    REQUIRE( ints1.size() == 66 );
    ints1 = { 1, 2, 3, 4, 5 };
    REQUIRE( ints1.size() == 5 );
    REQUIRE( ints1.at(0) == 1 );
    REQUIRE( ints1.at(4) == 5 );
}

TEST_CASE( "assign( count value )", "[series]" )
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    {
        series<foo> s1( 3, f1 );
        s1.assign( 2, f2 );
        REQUIRE( s1.size() == 2 );
        REQUIRE( s1.at( 0 ) == f2 );
        REQUIRE( s1.at( 1 ) == f2 );
    }
    {
        series<foo> s1( 2, f1 );
        s1.assign( 3, f2 );
        REQUIRE( s1.size() == 3 );
        REQUIRE( s1.at( 0 ) == f2 );
        REQUIRE( s1.at( 1 ) == f2 );
        REQUIRE( s1.at( 2 ) == f2 );
    }
}

TEST_CASE( "assign( begin end )", "[series]" )
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    {
        series<foo> s1( 3, f1 );
        vector<foo> v1( 4, f2 );
        s1.assign( v1.begin(), v1.begin()+2 );
        REQUIRE( s1.size() == 2 );
        REQUIRE( s1.at( 0 ) == f2 );
        REQUIRE( s1.at( 1 ) == f2 );
    }
    {
        series<foo> s1( 3, f1 );
        vector<foo> v1( 4, f2 );
        s1.assign( v1.begin(), v1.end() );
        REQUIRE( s1.size() == 4 );
        REQUIRE( s1.at( 0 ) == f2 );
        REQUIRE( s1.at( 1 ) == f2 );
        REQUIRE( s1.at( 2 ) == f2 );
        REQUIRE( s1.at( 3 ) == f2 );
    }
}

TEST_CASE( "assign( initializer_list )", "[series]" )
{
    {
        series<int> s1( 3U, 123 );
        s1.assign( { 1, 2 } );
        REQUIRE( s1.size() == 2 );
    }
    {
        series<int> s1( 3U, 123 );
        s1.assign( { 1, 2, 3, 4 } );
        REQUIRE( s1.size() == 4 );
    }
}

TEST_CASE( "front()", "[series]" )
{
    series<int> ints1{ 1, 2, 3, 4 };
    REQUIRE(ints1.size() == 4);
    REQUIRE(ints1.front() == 1);
}

TEST_CASE( "back()", "[series]" )
{
    series<int> ints1{ 1, 2, 3, 4 };
    REQUIRE(ints1.size() == 4);
    REQUIRE(ints1.back() == 4);
}

TEST_CASE( "data()", "[series]" )
{
    series<int> ints1{ 1, 2, 3, 4 };
    REQUIRE(ints1.size() == 4);
    REQUIRE(*(ints1.data()) == 1);
}

TEST_CASE( "begin()/end()", "[series]" )
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    series<foo> s1{ f1, f2, f3 };
    auto b = s1.begin();
    auto e = s1.end();
    REQUIRE((e-b) == 3);
    REQUIRE(*b == f1);
    REQUIRE(*(b+1) == f2);
    REQUIRE(*(b+2) == f3);
    REQUIRE(b+3 == e);
    REQUIRE(*(e-1) == f3);
    b++; ++b; b++;
    REQUIRE( b == e );
    b = s1.begin();
    e--; --e; e--;
    REQUIRE( b == e );
}

TEST_CASE( "rbegin()/rend()", "[series]" )
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    series<foo> s1{ f1, f2, f3 };
    auto b = s1.rbegin();
    auto e = s1.rend();
    REQUIRE((e-b) == 3);
    REQUIRE(*b == f3);
    REQUIRE(*(b+1) == f2);
    REQUIRE(*(b+2) == f1);
    REQUIRE(b+3 == e);
    REQUIRE(*(e-1) == f1);
    b++; ++b; b++;
    REQUIRE( b == e );
    b = s1.rbegin();
    e--; --e; e--;
    REQUIRE( b == e );
}

TEST_CASE( "empty()", "[series]" )
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    series<foo> sv1;
    REQUIRE(sv1.empty());
    sv1.assign( { f1, f2, f3 } );
    REQUIRE(!sv1.empty());
}

TEST_CASE( "reserve()/capacity()", "[series]" )
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    series<foo> sv1{ f1, f2, f3 };
    auto c1 = sv1.capacity();
    REQUIRE( c1>=3 );
    sv1.reserve( c1*10 );
    auto c2 = sv1.capacity();
    REQUIRE( c2 >= (c1*10) );
    REQUIRE( sv1.at( 0 ) == f1 );
    REQUIRE( sv1.at( 1 ) == f2 );
    REQUIRE( sv1.at( 2 ) == f3 );
}

TEST_CASE( "size()/clear()", "[series]" )
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    series<foo> sv1{ f1, f2, f3 };
    REQUIRE( sv1.size() == 3 );
    sv1.clear();
    REQUIRE( sv1.size() == 0 );
}

TEST_CASE( "insert( pos first last )", "[series]" )
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    foo f4{ "f4" };
    foo f5{ "f5" };
    SECTION( "begin(), empty insert" )
    {
        vector<foo> v2;
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE( sv1.size() == 3 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        auto it = sv1.insert( sv1.begin(), v2.begin(), v2.end() );
        REQUIRE( it == sv1.begin() );
        REQUIRE( sv1.size() == 3 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
    }
    SECTION( "begin(), empty insert, use_count" )
    {
        vector<foo> v2;
        series<foo> sv1{ f0, f1, f2 };
        series<foo> sv2{ sv1 };
        REQUIRE( sv1.size() == 3 );
        REQUIRE( sv2.size() == 3 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        REQUIRE( sv2.at( 0 ) == f0 );
        REQUIRE( sv2.at( 1 ) == f1 );
        REQUIRE( sv2.at( 2 ) == f2 );
        REQUIRE( sv1.use_count() == 2 );
        REQUIRE( sv2.use_count() == 2 );
        auto it = sv1.insert( sv1.begin(), v2.begin(), v2.end() );
        REQUIRE( it == sv1.begin() );
        REQUIRE( sv1.size() == 3 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        REQUIRE( sv1.use_count() == 2 );
        REQUIRE( sv2.use_count() == 2 );
    }
    SECTION( "begin()+1, empty insert" )
    {
        vector<foo> v2;
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE( sv1.size() == 3 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        auto it = sv1.insert( sv1.begin()+1, v2.begin(), v2.end() );
        REQUIRE( it == sv1.begin()+1 );
        REQUIRE( sv1.size() == 3 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
    }
    SECTION( "end(), empty insert" )
    {
        vector<foo> v2;
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE( sv1.size() == 3 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        auto it = sv1.insert( sv1.end(), v2.begin(), v2.end() );
        REQUIRE( it == sv1.end() );
        REQUIRE( sv1.size() == 3 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
    }
    SECTION( "begin(), non-overlapping" )
    {
        vector<foo> v1{ f3, f4, f5 };
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE( sv1.size() == 3 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        auto it = sv1.insert( sv1.begin(), v1.begin(), v1.end() );
        REQUIRE( it == sv1.begin() );
        REQUIRE( sv1.size() == 6 );
        REQUIRE( sv1.at( 0 ) == f3 );
        REQUIRE( sv1.at( 1 ) == f4 );
        REQUIRE( sv1.at( 2 ) == f5 );
        REQUIRE( sv1.at( 3 ) == f0 );
        REQUIRE( sv1.at( 4 ) == f1 );
        REQUIRE( sv1.at( 5 ) == f2 );
    }
    SECTION( "begin()+1, non-overlapping" )
    {
        vector<foo> v1{ f3, f4, f5 };
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE( sv1.size() == 3 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        auto it = sv1.insert( sv1.begin()+1, v1.begin(), v1.end() );
        REQUIRE( it == sv1.begin()+1 );
        REQUIRE( sv1.size() == 6 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f3 );
        REQUIRE( sv1.at( 2 ) == f4 );
        REQUIRE( sv1.at( 3 ) == f5 );
        REQUIRE( sv1.at( 4 ) == f1 );
        REQUIRE( sv1.at( 5 ) == f2 );
    }
    SECTION( "end(), non-overlapping" )
    {
        vector<foo> v1{ f3, f4, f5 };
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE( sv1.size() == 3 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        auto it = sv1.insert( sv1.end(), v1.begin(), v1.end() );
        REQUIRE( it == sv1.begin()+3 );
        REQUIRE( sv1.size() == 6 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        REQUIRE( sv1.at( 3 ) == f3 );
        REQUIRE( sv1.at( 4 ) == f4 );
        REQUIRE( sv1.at( 5 ) == f5 );
    }
    SECTION( "begin(), overlapping" )
    {
        vector<foo> v1{ f4, f5 };
        series<foo> sv1{ f0, f1, f2, f3 };
        REQUIRE( sv1.size() == 4 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        REQUIRE( sv1.at( 3 ) == f3 );
        auto it = sv1.insert( sv1.begin(), v1.begin(), v1.end() );
        REQUIRE( it == sv1.begin() );
        REQUIRE( sv1.size() == 6 );
        REQUIRE( sv1.at( 0 ) == f4 );
        REQUIRE( sv1.at( 1 ) == f5 );
        REQUIRE( sv1.at( 2 ) == f0 );
        REQUIRE( sv1.at( 3 ) == f1 );
        REQUIRE( sv1.at( 4 ) == f2 );
        REQUIRE( sv1.at( 5 ) == f3 );
    }
    SECTION( "begin()+1, overlapping" )
    {
        vector<foo> v1{ f4, f5 };
        series<foo> sv1{ f0, f1, f2, f3 };
        REQUIRE( sv1.size() == 4 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        REQUIRE( sv1.at( 3 ) == f3 );
        auto it = sv1.insert( sv1.begin()+1, v1.begin(), v1.end() );
        REQUIRE( it == sv1.begin()+1 );
        REQUIRE( sv1.size() == 6 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f4 );
        REQUIRE( sv1.at( 2 ) == f5 );
        REQUIRE( sv1.at( 3 ) == f1 );
        REQUIRE( sv1.at( 4 ) == f2 );
        REQUIRE( sv1.at( 5 ) == f3 );
    }
    SECTION( "begin()+1, overlapping, use_count" )
    {
        vector<foo> v1{ f4, f5 };
        series<foo> sv1{ f0, f1, f2, f3 };
        series<foo> sv2{ sv1 };
        REQUIRE( sv1.size() == 4 );
        REQUIRE( sv2.size() == 4 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        REQUIRE( sv1.at( 3 ) == f3 );
        REQUIRE( sv2.at( 0 ) == f0 );
        REQUIRE( sv2.at( 1 ) == f1 );
        REQUIRE( sv2.at( 2 ) == f2 );
        REQUIRE( sv2.at( 3 ) == f3 );
        REQUIRE( sv1.use_count() == 2 );
        REQUIRE( sv2.use_count() == 2 );
        auto it = sv1.insert( sv1.begin()+1, v1.begin(), v1.end() );
        REQUIRE( it == sv1.begin()+1 );
        REQUIRE( sv1.size() == 6 );
        REQUIRE( sv2.size() == 4 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f4 );
        REQUIRE( sv1.at( 2 ) == f5 );
        REQUIRE( sv1.at( 3 ) == f1 );
        REQUIRE( sv1.at( 4 ) == f2 );
        REQUIRE( sv1.at( 5 ) == f3 );
        REQUIRE( sv2.at( 0 ) == f0 );
        REQUIRE( sv2.at( 1 ) == f1 );
        REQUIRE( sv2.at( 2 ) == f2 );
        REQUIRE( sv2.at( 3 ) == f3 );
        REQUIRE( sv1.use_count() == 1 );
        REQUIRE( sv2.use_count() == 1 );
    }
    SECTION( "end(), overlapping" )
    {
        vector<foo> v1{ f4, f5 };
        series<foo> sv1{ f0, f1, f2, f3 };
        REQUIRE( sv1.size() == 4 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        REQUIRE( sv1.at( 3 ) == f3 );
        auto it = sv1.insert( sv1.end(), v1.begin(), v1.end() );
        REQUIRE( it == sv1.begin()+4 );
        REQUIRE( sv1.size() == 6 );
        REQUIRE( sv1.at( 0 ) == f0 );
        REQUIRE( sv1.at( 1 ) == f1 );
        REQUIRE( sv1.at( 2 ) == f2 );
        REQUIRE( sv1.at( 3 ) == f3 );
        REQUIRE( sv1.at( 4 ) == f4 );
        REQUIRE( sv1.at( 5 ) == f5 );
    }
}

TEST_CASE( "insert( pos, value )", "[series]" )
{
}






