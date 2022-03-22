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

TEST_CASE( "ctor( count, value )", "[series]" )
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

TEST_CASE( "ctor( begin, end )", "[series]" )
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

TEST_CASE( "assign( count, value )", "[series]" )
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

TEST_CASE( "assign( begin, end )", "[series]" )
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

TEST_CASE( "insert( pos, first, last )", "[series]" )
{
}

TEST_CASE( "insert( pos, value )", "[series]" )
{
}

//TEST_CASE( "make_series( name, data )", "[series]" )
//{
//    //series s1 = make_series<foo<double>>( "s1", { 1.1, 2.2, 3.3 } );
//    series s1 = make_series<double>( "s1", { 1.1, 2.2, 3.3 } );
//    REQUIRE( s1.name() == "s1" );
//    REQUIRE( s1.size() == 3 );
//}

//    SECTION( "Default" )
//    {
//        Series<double> s1;
//        REQUIRE( s1.size() == 0 );
//        Series<TestType> s2;
//        REQUIRE( s2.size() == 0 );
//    }
//
//    SECTION( "Copy" )
//    {
//        Series<double> s1{ 0.0, 1.0, 2.0 };
//        Series<double> s2{ s1 };
//        REQUIRE( s1.size() == 3 ); 
//        REQUIRE( s2.size() == 3 ); 
//        REQUIRE( s2[ 0 ] == 0.0 ); 
//        REQUIRE( s2[ 1 ] == 1.0 ); 
//        REQUIRE( s2[ 2 ] == 2.0 ); 
//    }
//
//    SECTION( "Move" )
//    {
//        Series<char> s1{ 0, 1, 2 };
//        Series<char> s2{ std::move( s1 ) };
//        REQUIRE( s1.size() == 0 ); 
//        REQUIRE( s2.size() == 3 ); 
//        REQUIRE( s2[ 0 ] == 0 ); 
//        REQUIRE( s2[ 1 ] == 1 ); 
//        REQUIRE( s2[ 2 ] == 2 ); 
//    }
//
//    SECTION( "Series( initializer_list )" )
//    {
//        Series<unsigned char> s1{ 0, 1, 2 };
//        REQUIRE( s1.size() == 3 ); 
//        REQUIRE( s1[ 0 ] == 0 ); 
//        REQUIRE( s1[ 1 ] == 1 ); 
//        REQUIRE( s1[ 2 ] == 2 ); 
//    }
//
//    SECTION( "Series( string, const vector<>& )" )
//    {
//        vector<float> v1{ 0.0f, 1.0f, 2.0f };
//        Series<float> s1{ "series", v1 };
//        REQUIRE( v1.size() == 3 ); 
//        REQUIRE( s1.size() == 3 ); 
//        REQUIRE( s1.name() == "series" );
//        REQUIRE( s1[ 0 ] == 0.0f ); 
//        REQUIRE( s1[ 1 ] == 1.0f ); 
//        REQUIRE( s1[ 2 ] == 2.0f ); 
//    }
//
//    SECTION( "Series( string, vector<>&& )" )
//    {
//        vector<unsigned long long> v1{ 0, 1, 2 };
//        Series<unsigned long long> s1{ "series", std::move( v1 ) };
//        REQUIRE( v1.size() == 0 ); 
//        REQUIRE( s1.size() == 3 ); 
//        REQUIRE( s1.name() == "series" );
//        REQUIRE( s1[ 0 ] == 0 ); 
//        REQUIRE( s1[ 1 ] == 1 ); 
//        REQUIRE( s1[ 2 ] == 2 ); 
//    }
//
//
//    SECTION( "Series( const vector<>& )" )
//    {
//        vector<int64_t> v1{ 0, 1, 2 };
//        Series<int64_t> s1{ v1 };
//        REQUIRE( v1.size() == 3 ); 
//        REQUIRE( s1.size() == 3 ); 
//        REQUIRE( s1[ 0 ] == 0 ); 
//        REQUIRE( s1[ 1 ] == 1 ); 
//        REQUIRE( s1[ 2 ] == 2 ); 
//    }
//
//    SECTION( "Series( vector<>&& )" )
//    {
//        vector<TestType> v1{ { 0 }, { 1 }, { 2 } };
//        Series<TestType> s1{ std::move( v1 ) };
//        REQUIRE( v1.size() == 0 ); 
//        REQUIRE( s1.size() == 3 ); 
//        REQUIRE( s1[ 0 ].getf1() == 0.0 ); 
//        REQUIRE( s1[ 1 ].getf1() == 1.0 ); 
//        REQUIRE( s1[ 2 ].getf1() == 2.0 ); 
//    }
//}


//TEST_CASE( "operator=", "[miniframe]" )
//{
//    SECTION( "operator=( const Series& )" )
//    {
//        Series<TestType> s1{ { 0 }, { 1 }, { 2 } };
//        Series<TestType> s2;
//        REQUIRE( s1.size() == 3 ); 
//        REQUIRE( s2.size() == 0 ); 
//        s2 = s1;
//        REQUIRE( s1.size() == 3 ); 
//        REQUIRE( s2.size() == 3 ); 
//        REQUIRE( s1[ 0 ] == s2[ 0 ] );
//        REQUIRE( s1[ 1 ] == s2[ 1 ] );
//        REQUIRE( s1[ 2 ] == s2[ 2 ] );
//    }
//
//    SECTION( "operator=( Series&& )" )
//    {
//        Series<TestType> s1{ { 0 }, { 1 }, { 2 } };
//        Series<TestType> s2;
//        REQUIRE( s1.size() == 3 ); 
//        REQUIRE( s2.size() == 0 ); 
//        s2 = std::move( s1 );
//        REQUIRE( s1.size() == 0 ); 
//        REQUIRE( s2.size() == 3 ); 
//        REQUIRE( s2[ 0 ].getf2() == 0 );
//        REQUIRE( s2[ 1 ].getf2() == 1 );
//        REQUIRE( s2[ 2 ].getf2() == 2 );
//    }
//
//    SECTION( "operator=( initializer_list )" )
//    {
//        Series<int> s1;
//        REQUIRE( s1.size() == 0 ); 
//        s1 = { 0, 1, 2 };
//        REQUIRE( s1.size() == 3 ); 
//        REQUIRE( s1[ 0 ] == 0 );
//        REQUIRE( s1[ 1 ] == 1 );
//        REQUIRE( s1[ 2 ] == 2 );
//    }
//}

//TEST_CASE( "assign", "[miniframe]" )
//{
//    SeriesOpt<int> s1;
//    s1.assign( { 0, 1, 2, nullopt, 4 } );
//    REQUIRE( s1[ 0 ] == 0 );
//    REQUIRE( s1[ 1 ] == 1 );
//    REQUIRE( s1[ 2 ] == 2 );
//    REQUIRE( !s1[ 3 ] );
//    REQUIRE( s1[ 4 ] == 4 );
//    dout << s1;
//}
//
//TEST_CASE( "to_string", "[miniframe]" )
//{
//    Series<int> s1( "s1", { 0, 1, 2, 3 } );
//    REQUIRE( s1[ 0 ] == 0 );
//    REQUIRE( s1[ 1 ] == 1 );
//    REQUIRE( s1[ 2 ] == 2 );
//    REQUIRE( s1[ 3 ] == 3 );
//    Series<string> ss1 = s1.to_string();
//    REQUIRE( ss1[ 0 ] == "0" );
//    REQUIRE( ss1[ 1 ] == "1" );
//    REQUIRE( ss1[ 2 ] == "2" );
//    REQUIRE( ss1[ 3 ] == "3" );
//    SeriesOpt<int> s2( "s2", { nullopt, 1, 2, 3 } );
//    REQUIRE( !s2[ 0 ] );
//    REQUIRE( s2[ 1 ] == 1 );
//    REQUIRE( s2[ 2 ] == 2 );
//    REQUIRE( s2[ 3 ] == 3 );
//    Series<string> ss2 = s2.to_string();
//    REQUIRE( ss2[ 0 ] == "nullopt" );
//    REQUIRE( ss2[ 1 ] == "1" );
//    REQUIRE( ss2[ 2 ] == "2" );
//    REQUIRE( ss2[ 3 ] == "3" );
//    dout << s1;
//    dout << ss1;
//    dout << s2;
//}

//TEST_CASE( "clone", "[miniframe]" )
//{
//    SeriesOpt<double> s1( "s1", { 0.0, nullopt, 2.0, 3.0 } );
//    REQUIRE( s1[ 0 ] == 0.0 );
//    REQUIRE( !s1[ 1 ] );
//    REQUIRE( s1[ 2 ] == 2.0 );
//    REQUIRE( s1[ 3 ] == 3.0 );
//
//    auto s2 = unique_ptr<SeriesBase>( s1.clone() );
//    REQUIRE( s2->name() == "s1" );
//
//    dout << s1;
//}

//TEST_CASE( "push_back", "[miniframe]" )
//{
//    SeriesOpt<double> s1{}; 
//    for ( double i = -10.0; i < 10.0; i += 1.1 ) {
//        s1.push_back( i );
//    }
//    s1.push_back( nullopt );
//    s1.push_back( nullopt );
//
//    REQUIRE( s1.size() == 21 );
//    for ( auto i = 0; i < 19; ++i ) {
//        REQUIRE( s1[ i ] );
//    }
//    REQUIRE( !s1[ 19 ] );
//    REQUIRE( !s1[ 20 ] );
//
//    dout << s1 << endl;
//}
//
//TEST_CASE( "copying" )
//{
//    SeriesOpt<double> s1{}; 
//    for ( double i = -10.0; i < 10.0; i += 1.1 ) {
//        s1.push_back( i );
//    }
//    s1.push_back( nullopt );
//    s1.push_back( nullopt );
//
//    auto s2 = s1;
//
//    REQUIRE( s2.size() == 21 );
//    REQUIRE( s1.size() == 21 );
//    for ( auto i = 0; i < 19; ++i ) {
//        REQUIRE( s2[ i ] );
//    }
//    REQUIRE( !s2[ 19 ] );
//    REQUIRE( !s2[ 20 ] );
//
//    dout << s2 << endl;
//}

//TEST_CASE( "misc" )
//{
//    SECTION( "cloning" )
//    {
//        auto s1 = make_unique<SeriesOpt<double>>( "foo" ); 
//        for ( double i = -10.0; i < 10.0; i += 1.1 ) {
//            s1->push_back( i );
//        }
//        s1->push_back( nullopt );
//        s1->push_back( nullopt );
//
//        std::unique_ptr<SeriesBase> sb{ std::move( s1 ) };
//        REQUIRE( !s1 );
//        REQUIRE( sb );
//        std::unique_ptr<SeriesBase> ssb{ sb->clone() };
//        REQUIRE( ssb );
//        std::unique_ptr<SeriesBase> sssb{ std::move( ssb ) };
//        REQUIRE( !ssb );
//        REQUIRE( sssb );
//        REQUIRE( sssb->name() == "foo" );
//    }
//
//    SECTION( "moving" )
//    {
//        SeriesOpt<double> s1{}; 
//        for ( double i = -10.0; i < 10.0; i += 1.1 ) {
//            s1.push_back( i );
//        }
//        s1.push_back( nullopt );
//        s1.push_back( nullopt );
//
//        SeriesOpt<double> s2{ std::move( s1 ) };
//
//        REQUIRE( s2.size() == 21 );
//        REQUIRE( s1.size() == 0 );
//        for ( auto i = 0; i < 19; ++i ) {
//            REQUIRE( s2[ i ] );
//        }
//        REQUIRE( !s2[ 19 ] );
//        REQUIRE( !s2[ 20 ] );
//
//        dout << s2 << endl;
//        dout << s1 << endl;
//    }
//
//}





