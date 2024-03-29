//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <ostream>
#include <cmath>
#include <limits>
#include <complex>

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

template<class T>
class TD;

template<typename T>
std::ostream& operator<<( std::ostream& o, const std::vector<T>& v )
{
    o << "[";
    unsigned i = 0;
    for ( const auto& e : v ) {
        if ( i++ != 0 ) { o << ","; }
        o << e;
    }
    o << "]";
    return o;
}

template<typename T>
std::ostream& operator<<( std::ostream& o, const std::complex<T>& c )
{
    o << "{" << c.real() << "," << c.imag() << "}";
    return o;
}

static char*
mystrdup(const char* s)
{
    if (nullptr == s) {
        throw std::invalid_argument{ "mystrdup( nullptr )" };
    }
    auto len = strlen(s);
    void* p  = malloc(len + 1);
    if (nullptr == p) {
        throw std::runtime_error{ "malloc returned nullptr" };
    }
    memcpy(p, s, len);
    char* m = reinterpret_cast<char*>(p);
    m[len]  = '\0';
    return m;
}

class foo
{
public:
    foo(const char* str)
    {
        m_str = mystrdup(str);
    }
    foo(const foo& other)
    {
        m_str = mystrdup(other.m_str);
    }
    foo(foo&& other)
        : m_str(mystrdup("EMPTY"))
    {
        std::swap(m_str, other.m_str);
    }
    foo&
    operator=(const foo& other)
    {
        foo f{ other };
        std::swap(m_str, f.m_str);
        return *this;
    }
    bool
    operator==(const char* str) const
    {
        return std::strcmp(m_str, str) == 0;
    }
    bool
    operator==(const std::string& str) const
    {
        return std::string{ m_str } == str;
    }
    bool
    operator==(const foo& other) const
    {
        return std::string{ m_str } == std::string{ other.m_str };
    }

    virtual ~foo()
    {
        free(m_str);
    }

    friend std::ostream& operator<<(std::ostream& o, const foo& f);

    char* m_str;
};

class foo_with_ctor : public foo
{
public:
    using foo::foo;
    foo_with_ctor()
        : foo("default")
    {}
};

std::ostream&
operator<<(std::ostream& o, const foo& f)
{
    if (f.m_str == nullptr) {
        o << "foo{ m_str=nullptr }";
    }
    else {
        o << "foo{ m_str=\"" << f.m_str << "\" }";
    }
    return o;
}

TEST_CASE("ctor()", "[series]")
{
    series<foo> s1;
    REQUIRE(s1.name().empty());
    REQUIRE(s1.size() == 0);
}

TEST_CASE("ctor( count value )", "[series]")
{
    foo f1{ "f1" };
    series<foo> s1(3, f1);
    REQUIRE(s1.size() == 3);
}

TEST_CASE("ctor( count )", "[series]")
{
    series<double> s1(3);
    REQUIRE(s1.size() == 3);
}

TEST_CASE("ctor( begin end )", "[series]")
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    foo f4{ "f4" };
    vector<foo> v1{ f0, f1, f2, f3, f4 };
    series<foo> s1(v1.begin(), v1.end());
    REQUIRE(s1.size() == 5);
}

TEST_CASE("ctor( const& )", "[series]")
{
    series<double> s1(5);
    REQUIRE(s1.size() == 5);
    series<double> s2(s1);
    REQUIRE(s1.size() == 5);
    REQUIRE(s2.size() == 5);
    REQUIRE(s1.use_count() == 2);
    REQUIRE(s2.use_count() == 2);
}

TEST_CASE("ctor( && )", "[series]")
{
    series<double> s1(5);
    REQUIRE(s1.size() == 5);
    series<double> s2(std::move(s1));
    REQUIRE(s1.size() == 0);
    REQUIRE(s2.size() == 5);
    REQUIRE(s1.use_count() == 1);
    REQUIRE(s2.use_count() == 1);
}

TEST_CASE("ctor( initializer_list )", "[series]")
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    foo f4{ "f4" };
    series<foo> s1{ f0, f1, f2, f3, f4 };
    REQUIRE(s1.size() == 5);
}

TEST_CASE("operator=( const& )", "[series]")
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    series<foo> s1(66, f1);
    series<foo> s2(33, f2);
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
    REQUIRE(s1.use_count() == 2);
    REQUIRE(s2.use_count() == 2);
    REQUIRE(s1.at(0) == f1);
    REQUIRE(s1.at(65) == f1);
    REQUIRE(s2.at(0) == f1);
    REQUIRE(s2.at(65) == f1);
    REQUIRE(s1.use_count() == 1);
    REQUIRE(s2.use_count() == 1);
    const auto s3 = s1;
    REQUIRE(s1.size() == 66);
    REQUIRE(s3.size() == 66);
    REQUIRE(s1.use_count() == 2);
    REQUIRE(s3.use_count() == 2);
    REQUIRE(s3.at(0) == f1);
    REQUIRE(s3.at(65) == f1);
    REQUIRE(s1.use_count() == 2);
    REQUIRE(s3.use_count() == 2);
}

TEST_CASE("operator=( && )", "[series]")
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    series<foo> s1(66, f1);
    series<foo> s2(33, f2);
    REQUIRE(s1.size() == 66);
    REQUIRE(s2.size() == 33);
    REQUIRE(s1.at(0) == f1);
    REQUIRE(s1.at(65) == f1);
    REQUIRE(s2.at(0) == f2);
    REQUIRE(s2.at(32) == f2);
    REQUIRE(s1.use_count() == 1);
    REQUIRE(s2.use_count() == 1);
    s2 = std::move(s1);
    REQUIRE(s1.size() == 0);
    REQUIRE(s2.size() == 66);
    REQUIRE(s2.at(0) == f1);
    REQUIRE(s2.at(65) == f1);
    REQUIRE(s1.use_count() == 1);
    REQUIRE(s2.use_count() == 1);
}

TEST_CASE("operator=( initializer_list )", "[series]")
{
    series<int> ints1(66);
    REQUIRE(ints1.size() == 66);
    ints1 = { 1, 2, 3, 4, 5 };
    REQUIRE(ints1.size() == 5);
    REQUIRE(ints1.at(0) == 1);
    REQUIRE(ints1.at(4) == 5);
}

TEST_CASE("assign( count value )", "[series]")
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    {
        series<foo> s1(3, f1);
        s1.assign(2, f2);
        REQUIRE(s1.size() == 2);
        REQUIRE(s1.at(0) == f2);
        REQUIRE(s1.at(1) == f2);
    }
    {
        series<foo> s1(2, f1);
        s1.assign(3, f2);
        REQUIRE(s1.size() == 3);
        REQUIRE(s1.at(0) == f2);
        REQUIRE(s1.at(1) == f2);
        REQUIRE(s1.at(2) == f2);
    }
}

TEST_CASE("assign( begin end )", "[series]")
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    {
        series<foo> s1(3, f1);
        vector<foo> v1(4, f2);
        s1.assign(v1.begin(), v1.begin() + 2);
        REQUIRE(s1.size() == 2);
        REQUIRE(s1.at(0) == f2);
        REQUIRE(s1.at(1) == f2);
    }
    {
        series<foo> s1(3, f1);
        vector<foo> v1(4, f2);
        s1.assign(v1.begin(), v1.end());
        REQUIRE(s1.size() == 4);
        REQUIRE(s1.at(0) == f2);
        REQUIRE(s1.at(1) == f2);
        REQUIRE(s1.at(2) == f2);
        REQUIRE(s1.at(3) == f2);
    }
}

TEST_CASE("assign( initializer_list )", "[series]")
{
    {
        series<int> s1(3U, 123);
        s1.assign({ 1, 2 });
        REQUIRE(s1.size() == 2);
    }
    {
        series<int> s1(3U, 123);
        s1.assign({ 1, 2, 3, 4 });
        REQUIRE(s1.size() == 4);
    }
}

TEST_CASE("front()", "[series]")
{
    series<int> ints1{ 1, 2, 3, 4 };
    REQUIRE(ints1.size() == 4);
    REQUIRE(ints1.front() == 1);
}

TEST_CASE("back()", "[series]")
{
    series<int> ints1{ 1, 2, 3, 4 };
    REQUIRE(ints1.size() == 4);
    REQUIRE(ints1.back() == 4);
}

TEST_CASE("data()", "[series]")
{
    series<int> ints1{ 1, 2, 3, 4 };
    REQUIRE(ints1.size() == 4);
    REQUIRE(*(ints1.data()) == 1);
}

TEST_CASE("begin()/end()", "[series]")
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    series<foo> s1{ f1, f2, f3 };
    auto b = s1.begin();
    auto e = s1.end();
    REQUIRE((e - b) == 3);
    REQUIRE(*b == f1);
    REQUIRE(*(b + 1) == f2);
    REQUIRE(*(b + 2) == f3);
    REQUIRE(b + 3 == e);
    REQUIRE(*(e - 1) == f3);
    b++;
    ++b;
    b++;
    REQUIRE(b == e);
    b = s1.begin();
    e--;
    --e;
    e--;
    REQUIRE(b == e);
}

TEST_CASE("rbegin()/rend()", "[series]")
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    series<foo> s1{ f1, f2, f3 };
    auto b = s1.rbegin();
    auto e = s1.rend();
    REQUIRE((e - b) == 3);
    REQUIRE(*b == f3);
    REQUIRE(*(b + 1) == f2);
    REQUIRE(*(b + 2) == f1);
    REQUIRE(b + 3 == e);
    REQUIRE(*(e - 1) == f1);
    b++;
    ++b;
    b++;
    REQUIRE(b == e);
    b = s1.rbegin();
    e--;
    --e;
    e--;
    REQUIRE(b == e);
}

TEST_CASE("empty()", "[series]")
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    series<foo> sv1;
    REQUIRE(sv1.empty());
    sv1.assign({ f1, f2, f3 });
    REQUIRE(!sv1.empty());
}

TEST_CASE("reserve()/capacity()", "[series]")
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    series<foo> sv1{ f1, f2, f3 };
    auto c1 = sv1.capacity();
    REQUIRE(c1 >= 3);
    sv1.reserve(c1 * 10);
    auto c2 = sv1.capacity();
    REQUIRE(c2 >= (c1 * 10));
    REQUIRE(sv1.at(0) == f1);
    REQUIRE(sv1.at(1) == f2);
    REQUIRE(sv1.at(2) == f3);
}

TEST_CASE("size()/clear()", "[series]")
{
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    series<foo> sv1{ f1, f2, f3 };
    REQUIRE(sv1.size() == 3);
    sv1.clear();
    REQUIRE(sv1.size() == 0);
}

TEST_CASE("insert( pos first last )", "[series]")
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    foo f4{ "f4" };
    foo f5{ "f5" };
    SECTION("begin(), empty insert")
    {
        vector<foo> v2;
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin(), v2.begin(), v2.end());
        REQUIRE(it == sv1.begin());
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
    }
    SECTION("begin(), empty insert, use_count")
    {
        vector<foo> v2;
        series<foo> sv1{ f0, f1, f2 };
        series<foo> sv2{ sv1 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv2.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv2.at(0) == f0);
        REQUIRE(sv2.at(1) == f1);
        REQUIRE(sv2.at(2) == f2);
        REQUIRE(sv1 == sv2);
        auto it = sv1.insert(sv1.begin(), v2.begin(), v2.end());
        REQUIRE(it == sv1.begin());
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
    }
    SECTION("begin()+1, empty insert")
    {
        vector<foo> v2;
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin() + 1, v2.begin(), v2.end());
        REQUIRE(it == sv1.begin() + 1);
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
    }
    SECTION("end(), empty insert")
    {
        vector<foo> v2;
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.end(), v2.begin(), v2.end());
        REQUIRE(it == sv1.end());
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
    }
    SECTION("begin(), non-overlapping")
    {
        vector<foo> v1{ f3, f4, f5 };
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin(), v1.begin(), v1.end());
        REQUIRE(it == sv1.begin());
        REQUIRE(sv1.size() == 6);
        REQUIRE(sv1.at(0) == f3);
        REQUIRE(sv1.at(1) == f4);
        REQUIRE(sv1.at(2) == f5);
        REQUIRE(sv1.at(3) == f0);
        REQUIRE(sv1.at(4) == f1);
        REQUIRE(sv1.at(5) == f2);
    }
    SECTION("begin()+1, non-overlapping")
    {
        vector<foo> v1{ f3, f4, f5 };
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin() + 1, v1.begin(), v1.end());
        REQUIRE(it == sv1.begin() + 1);
        REQUIRE(sv1.size() == 6);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f3);
        REQUIRE(sv1.at(2) == f4);
        REQUIRE(sv1.at(3) == f5);
        REQUIRE(sv1.at(4) == f1);
        REQUIRE(sv1.at(5) == f2);
    }
    SECTION("end(), non-overlapping")
    {
        vector<foo> v1{ f3, f4, f5 };
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.end(), v1.begin(), v1.end());
        REQUIRE(it == sv1.begin() + 3);
        REQUIRE(sv1.size() == 6);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv1.at(3) == f3);
        REQUIRE(sv1.at(4) == f4);
        REQUIRE(sv1.at(5) == f5);
    }
    SECTION("begin(), overlapping")
    {
        vector<foo> v1{ f4, f5 };
        series<foo> sv1{ f0, f1, f2, f3 };
        REQUIRE(sv1.size() == 4);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv1.at(3) == f3);
        auto it = sv1.insert(sv1.begin(), v1.begin(), v1.end());
        REQUIRE(it == sv1.begin());
        REQUIRE(sv1.size() == 6);
        REQUIRE(sv1.at(0) == f4);
        REQUIRE(sv1.at(1) == f5);
        REQUIRE(sv1.at(2) == f0);
        REQUIRE(sv1.at(3) == f1);
        REQUIRE(sv1.at(4) == f2);
        REQUIRE(sv1.at(5) == f3);
    }
    SECTION("begin()+1, overlapping")
    {
        vector<foo> v1{ f4, f5 };
        series<foo> sv1{ f0, f1, f2, f3 };
        REQUIRE(sv1.size() == 4);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv1.at(3) == f3);
        auto it = sv1.insert(sv1.begin() + 1, v1.begin(), v1.end());
        REQUIRE(it == sv1.begin() + 1);
        REQUIRE(sv1.size() == 6);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f4);
        REQUIRE(sv1.at(2) == f5);
        REQUIRE(sv1.at(3) == f1);
        REQUIRE(sv1.at(4) == f2);
        REQUIRE(sv1.at(5) == f3);
    }
    SECTION("begin()+1, overlapping, use_count")
    {
        vector<foo> v1{ f4, f5 };
        series<foo> sv1{ f0, f1, f2, f3 };
        series<foo> sv2{ sv1 };
        REQUIRE(sv1.size() == 4);
        REQUIRE(sv2.size() == 4);
        REQUIRE(sv1.use_count() == 2);
        REQUIRE(sv2.use_count() == 2);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv1.at(3) == f3);
        REQUIRE(sv2.at(0) == f0);
        REQUIRE(sv2.at(1) == f1);
        REQUIRE(sv2.at(2) == f2);
        REQUIRE(sv2.at(3) == f3);
        REQUIRE(sv1.use_count() == 1);
        REQUIRE(sv2.use_count() == 1);
        auto it = sv1.insert(sv1.begin() + 1, v1.begin(), v1.end());
        REQUIRE(it == sv1.begin() + 1);
        REQUIRE(sv1.size() == 6);
        REQUIRE(sv2.size() == 4);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f4);
        REQUIRE(sv1.at(2) == f5);
        REQUIRE(sv1.at(3) == f1);
        REQUIRE(sv1.at(4) == f2);
        REQUIRE(sv1.at(5) == f3);
        REQUIRE(sv2.at(0) == f0);
        REQUIRE(sv2.at(1) == f1);
        REQUIRE(sv2.at(2) == f2);
        REQUIRE(sv2.at(3) == f3);
        REQUIRE(sv1.use_count() == 1);
        REQUIRE(sv2.use_count() == 1);
    }
    SECTION("end(), overlapping")
    {
        vector<foo> v1{ f4, f5 };
        series<foo> sv1{ f0, f1, f2, f3 };
        REQUIRE(sv1.size() == 4);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv1.at(3) == f3);
        auto it = sv1.insert(sv1.end(), v1.begin(), v1.end());
        REQUIRE(it == sv1.begin() + 4);
        REQUIRE(sv1.size() == 6);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv1.at(3) == f3);
        REQUIRE(sv1.at(4) == f4);
        REQUIRE(sv1.at(5) == f5);
    }
}

TEST_CASE("insert( pos value )", "[series]")
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    SECTION("begin()")
    {
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin(), f3);
        REQUIRE(it == sv1.begin());
        REQUIRE(sv1.size() == 4);
        REQUIRE(sv1.at(0) == f3);
        REQUIRE(sv1.at(1) == f0);
        REQUIRE(sv1.at(2) == f1);
        REQUIRE(sv1.at(3) == f2);
    }
    SECTION("begin()+1")
    {
        series<foo> sv1{ f0, f1, f2 };
        series<foo> sv2{ sv1 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.use_count() == 2);
        REQUIRE(sv2.use_count() == 2);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv2.at(0) == f0);
        REQUIRE(sv2.at(1) == f1);
        REQUIRE(sv2.at(2) == f2);
        auto it = sv2.insert(sv2.begin() + 1, f3);
        REQUIRE(it == sv2.begin() + 1);
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv2.size() == 4);
        REQUIRE(sv1.use_count() == 1);
        REQUIRE(sv2.use_count() == 1);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv2.at(0) == f0);
        REQUIRE(sv2.at(1) == f3);
        REQUIRE(sv2.at(2) == f1);
        REQUIRE(sv2.at(3) == f2);
    }
    SECTION("end()")
    {
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.end(), f3);
        REQUIRE(it == sv1.begin() + 3);
        REQUIRE(sv1.size() == 4);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv1.at(3) == f3);
    }
}

TEST_CASE("insert( pos &&value )", "[series]")
{
    SECTION("begin()")
    {
        foo f0{ "f0" };
        foo f1{ "f1" };
        foo f2{ "f2" };
        foo f3{ "f3" };
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin(), std::move(f3));
        REQUIRE(it == sv1.begin());
        REQUIRE(sv1.size() == 4);
        REQUIRE(f3 == "EMPTY");
        REQUIRE(string(sv1.at(0).m_str) == "f3");
        REQUIRE(sv1.at(1) == f0);
        REQUIRE(sv1.at(2) == f1);
        REQUIRE(sv1.at(3) == f2);
    }
    SECTION("begin()+1")
    {
        foo f0{ "f0" };
        foo f1{ "f1" };
        foo f2{ "f2" };
        foo f3{ "f3" };
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin() + 1, std::move(f3));
        REQUIRE(it == sv1.begin() + 1);
        REQUIRE(sv1.size() == 4);
        REQUIRE(f3 == "EMPTY");
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(string(sv1.at(1).m_str) == "f3");
        REQUIRE(sv1.at(2) == f1);
        REQUIRE(sv1.at(3) == f2);
    }
    SECTION("end()")
    {
        foo f0{ "f0" };
        foo f1{ "f1" };
        foo f2{ "f2" };
        foo f3{ "f3" };
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.end(), std::move(f3));
        REQUIRE(it == sv1.begin() + 3);
        REQUIRE(sv1.size() == 4);
        REQUIRE(f3 == "EMPTY");
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv1.at(3) == "f3");
    }
}

TEST_CASE("insert( pos count value )", "[series]")
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    SECTION("begin(), count == 0")
    {
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin(), 0, f3);
        REQUIRE(it == sv1.begin());
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
    }
    SECTION("begin(), count == 1")
    {
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin(), 1, f3);
        REQUIRE(it == sv1.begin());
        REQUIRE(sv1.size() == 4);
        REQUIRE(sv1.at(0) == f3);
        REQUIRE(sv1.at(1) == f0);
        REQUIRE(sv1.at(2) == f1);
        REQUIRE(sv1.at(3) == f2);
    }
    SECTION("begin(), count == 2")
    {
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin(), 2, f3);
        REQUIRE(it == sv1.begin());
        REQUIRE(sv1.size() == 5);
        REQUIRE(sv1.at(0) == f3);
        REQUIRE(sv1.at(1) == f3);
        REQUIRE(sv1.at(2) == f0);
        REQUIRE(sv1.at(3) == f1);
        REQUIRE(sv1.at(4) == f2);
    }
    SECTION("begin()+1, count == 0")
    {
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin() + 1, 0, f3);
        REQUIRE(it == sv1.begin() + 1);
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
    }
    SECTION("begin()+1, count == 1")
    {
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin() + 1, 1, f3);
        REQUIRE(it == sv1.begin() + 1);
        REQUIRE(sv1.size() == 4);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f3);
        REQUIRE(sv1.at(2) == f1);
        REQUIRE(sv1.at(3) == f2);
    }
    SECTION("begin()+1, count == 2")
    {
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.begin() + 1, 2, f3);
        REQUIRE(it == sv1.begin() + 1);
        REQUIRE(sv1.size() == 5);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f3);
        REQUIRE(sv1.at(2) == f3);
        REQUIRE(sv1.at(3) == f1);
        REQUIRE(sv1.at(4) == f2);
    }
    SECTION("end(), count == 0")
    {
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.end(), 0, f3);
        REQUIRE(it == sv1.begin() + 3);
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
    }
    SECTION("end(), count == 1")
    {
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        auto it = sv1.insert(sv1.end(), 1, f3);
        REQUIRE(it == sv1.begin() + 3);
        REQUIRE(sv1.size() == 4);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv1.at(3) == f3);
    }
    SECTION("end(), count == 2")
    {
        series<foo> sv1{ f0, f1, f2 };
        series<foo> sv2{ sv1 };
        series<foo> sv3{ sv2 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv2.size() == 3);
        REQUIRE(sv3.size() == 3);
        REQUIRE(sv1.use_count() == 3);
        REQUIRE(sv2.use_count() == 3);
        REQUIRE(sv3.use_count() == 3);
        REQUIRE(sv1 == sv2);
        REQUIRE(sv1 == sv3);
        REQUIRE(sv2 == sv3);
        auto it = sv2.insert(sv2.cend(), 2, f3);
        REQUIRE(it == sv2.cbegin() + 3);
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv2.size() == 5);
        REQUIRE(sv3.size() == 3);
        REQUIRE(sv1.use_count() == 2);
        REQUIRE(sv2.use_count() == 1);
        REQUIRE(sv3.use_count() == 2);
        REQUIRE(sv2.at(0) == f0);
        REQUIRE(sv2.at(1) == f1);
        REQUIRE(sv2.at(2) == f2);
        REQUIRE(sv2.at(3) == f3);
        REQUIRE(sv2.at(4) == f3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv3.at(0) == f0);
        REQUIRE(sv3.at(1) == f1);
        REQUIRE(sv3.at(2) == f2);
    }
}

TEST_CASE("emplace( pos args... )", "[series]")
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    SECTION("begin()")
    {
        series<foo> sv1{ f0, f1, f2 };
        series<foo> sv2{ sv1 };
        series<foo> sv3{ sv1 };
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv2.size() == 3);
        REQUIRE(sv3.size() == 3);
        REQUIRE(sv1.use_count() == 3);
        REQUIRE(sv2.use_count() == 3);
        REQUIRE(sv3.use_count() == 3);
        sv3.emplace(sv3.begin(), "f4");
        auto it = sv3.emplace(sv3.begin(), "f3");
        REQUIRE(it == sv3.begin());
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv2.size() == 3);
        REQUIRE(sv3.size() == 5);
        REQUIRE(sv1.use_count() == 2);
        REQUIRE(sv2.use_count() == 2);
        REQUIRE(sv3.use_count() == 1);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv2.at(0) == f0);
        REQUIRE(sv2.at(1) == f1);
        REQUIRE(sv2.at(2) == f2);
        REQUIRE(sv3.at(0) == "f3");
        REQUIRE(sv3.at(1) == "f4");
        REQUIRE(sv3.at(2) == f0);
        REQUIRE(sv3.at(3) == f1);
        REQUIRE(sv3.at(4) == f2);
    }
    SECTION("begin()+1")
    {
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        sv1.emplace(sv1.begin() + 1, "f4");
        auto it = sv1.emplace(sv1.begin() + 1, "f3");
        REQUIRE(it == sv1.begin() + 1);
        REQUIRE(sv1.size() == 5);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == "f3");
        REQUIRE(sv1.at(2) == "f4");
        REQUIRE(sv1.at(3) == f1);
        REQUIRE(sv1.at(4) == f2);
    }
    SECTION("end()")
    {
        series<foo> sv1{ f0, f1, f2 };
        REQUIRE(sv1.size() == 3);
        sv1.emplace(sv1.end(), "f3");
        auto it = sv1.emplace(sv1.end(), "f4");
        REQUIRE(it == sv1.begin() + 4);
        REQUIRE(sv1.size() == 5);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv1.at(3) == "f3");
        REQUIRE(sv1.at(4) == "f4");
    }
}

TEST_CASE("erase( first last )", "[series]")
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    SECTION("begin() 1")
    {
        series<foo> sv1{ f0, f1, f2, f3 };
        REQUIRE(sv1.size() == 4);
        auto it = sv1.erase(sv1.begin(), sv1.begin() + 1);
        REQUIRE(it == sv1.begin());
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f1);
        REQUIRE(sv1.at(1) == f2);
        REQUIRE(sv1.at(2) == f3);
    }
    SECTION("begin()+1 1")
    {
        series<foo> sv1{ f0, f1, f2, f3 };
        REQUIRE(sv1.size() == 4);
        auto it = sv1.erase(sv1.begin() + 1, sv1.begin() + 2);
        REQUIRE(it == sv1.begin() + 1);
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f2);
        REQUIRE(sv1.at(2) == f3);
    }
    SECTION("end() 1")
    {
        series<foo> sv1{ f0, f1, f2, f3 };
        series<foo> sv2{ sv1 };
        REQUIRE(sv1.size() == 4);
        REQUIRE(sv2.size() == 4);
        REQUIRE(sv1.use_count() == 2);
        REQUIRE(sv2.use_count() == 2);
        auto it = sv1.erase(sv1.end() - 1, sv1.end());
        REQUIRE(it == sv1.begin() + 3);
        REQUIRE(it == sv1.end());
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv2.size() == 4);
        REQUIRE(sv1.use_count() == 1);
        REQUIRE(sv2.use_count() == 1);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
        REQUIRE(sv2.at(0) == f0);
        REQUIRE(sv2.at(1) == f1);
        REQUIRE(sv2.at(2) == f2);
        REQUIRE(sv2.at(3) == f3);
    }
}

TEST_CASE("erase( pos )", "[series]")
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    SECTION("begin()")
    {
        series<foo> sv1{ f0, f1, f2, f3 };
        REQUIRE(sv1.size() == 4);
        auto it = sv1.erase(sv1.begin());
        REQUIRE(it == sv1.begin());
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f1);
        REQUIRE(sv1.at(1) == f2);
        REQUIRE(sv1.at(2) == f3);
    }
    SECTION("begin()+1")
    {
        series<foo> sv1{ f0, f1, f2, f3 };
        REQUIRE(sv1.size() == 4);
        auto it = sv1.erase(sv1.begin() + 1);
        REQUIRE(it == sv1.begin() + 1);
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f2);
        REQUIRE(sv1.at(2) == f3);
    }
    SECTION("end()-1")
    {
        series<foo> sv1{ f0, f1, f2, f3 };
        REQUIRE(sv1.size() == 4);
        auto it = sv1.erase(sv1.end() - 1);
        REQUIRE(it == sv1.begin() + 3);
        REQUIRE(it == sv1.end());
        REQUIRE(sv1.size() == 3);
        REQUIRE(sv1.at(0) == f0);
        REQUIRE(sv1.at(1) == f1);
        REQUIRE(sv1.at(2) == f2);
    }
}

TEST_CASE("push_back() emplace_back() pop_back()", "[series]")
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo f3{ "f3" };
    series<foo> sv1{ f0, f1 };
    REQUIRE(sv1.size() == 2);
    sv1.push_back(std::move(f2));
    REQUIRE(sv1.size() == 3);
    REQUIRE(f2 == "EMPTY");
    sv1.push_back(f3);
    REQUIRE(sv1.size() == 4);
    REQUIRE(f3 == "f3");
    sv1.emplace_back("f4");
    sv1.emplace_back("f5");
    sv1.emplace_back("f6");
    REQUIRE(sv1.size() == 7);
    REQUIRE(sv1.at(0) == f0);
    REQUIRE(sv1.at(1) == f1);
    REQUIRE(sv1.at(2) == "f2");
    REQUIRE(sv1.at(3) == f3);
    REQUIRE(sv1.at(4) == "f4");
    REQUIRE(sv1.at(5) == "f5");
    REQUIRE(sv1.at(6) == "f6");
    sv1.pop_back();
    sv1.pop_back();
    sv1.pop_back();
    REQUIRE(sv1.size() == 4);
    REQUIRE(sv1.at(0) == f0);
    REQUIRE(sv1.at(1) == f1);
    REQUIRE(sv1.at(2) == "f2");
    REQUIRE(sv1.at(3) == f3);
}

TEST_CASE("resize()", "[series]")
{
    foo f0{ "f0" };
    foo f1{ "f1" };
    foo f2{ "f2" };
    foo_with_ctor fc0{ "fc0" };
    foo_with_ctor fc1{ "fc1" };
    series<foo> sv1{ f0, f1 };
    sv1.resize(1);
    REQUIRE(sv1.size() == 1);
    REQUIRE_THROWS(sv1.resize(3));
    sv1.resize(3, f2);
    REQUIRE(sv1.size() == 3);
    series<foo_with_ctor> sv2{ fc0, fc1 };
    sv2.resize(1);
    REQUIRE(sv2.size() == 1);
    REQUIRE_NOTHROW(sv2.resize(3));
    REQUIRE(sv2.size() == 3);
}

TEST_CASE("allow_missing()", "[series]")
{
    SECTION("convert")
    {
        foo f0{ "f0" };
        foo f1{ "f1" };
        foo f2{ "f2" };
        foo f3{ "f3" };
        series<foo> sv1{ f0, f1, f2 };
        auto sv2 = sv1.allow_missing();
        REQUIRE(sv2.size() == 3);
        REQUIRE(sv2[0] == f0);
        REQUIRE(sv2[1] == f1);
        REQUIRE(sv2[2] == f2);
        sv2.push_back(nullopt);
        sv2.push_back(f3);
        REQUIRE(sv2.size() == 5);
        REQUIRE(sv2[3] == nullopt);
        REQUIRE(sv2[4] == f3);
    }

    SECTION("no conversion")
    {
        foo f0{ "f0" };
        foo f1{ "f1" };
        foo f2{ "f2" };
        foo f3{ "f3" };
        series<mi<foo>> sv1{ f0, missing, f2 };
        auto sv2 = sv1.allow_missing();
        REQUIRE(sv2.size() == 3);
        REQUIRE(sv2[0] == f0);
        REQUIRE(sv2[1] == nullopt);
        REQUIRE(sv2[2] == f2);
        sv2.push_back(nullopt);
        sv2.push_back(f3);
        REQUIRE(sv2.size() == 5);
        REQUIRE(sv2[3] == nullopt);
        REQUIRE(sv2[4] == f3);
    }
}

TEST_CASE("disallow_missing()", "[series]")
{
    SECTION("convert")
    {
        foo_with_ctor f0{ "f0" };
        foo_with_ctor f1{ "f1" };
        foo_with_ctor f2{ "f2" };
        foo_with_ctor fdefault;
        series<mi<foo_with_ctor>> sv1{ f0, f1, f2 };
        sv1.push_back(missing);
        auto sv2 = sv1.disallow_missing();
        REQUIRE(sv2.size() == 4);
        REQUIRE(sv2[0] == f0);
        REQUIRE(sv2[1] == f1);
        REQUIRE(sv2[2] == f2);
        REQUIRE(sv2[3] == fdefault);
    }

    SECTION("no conversion")
    {
        foo f0{ "f0" };
        foo f1{ "f1" };
        foo f2{ "f2" };
        series<foo> sv1{ f0, f1, f2 };
        auto sv2 = sv1.disallow_missing();
        REQUIRE(sv2.size() == 3);
        REQUIRE(sv2[0] == f0);
        REQUIRE(sv2[1] == f1);
        REQUIRE(sv2[2] == f2);
    }
}

struct HoldsInt
{
    explicit HoldsInt(int f)
        : m_f(f)
    {}
    bool
    operator==(const HoldsInt& other) const
    {
        return m_f == other.m_f;
    }
    bool
    operator!=(const HoldsInt& other) const
    {
        return m_f != other.m_f;
    }
    bool
    operator<(const HoldsInt& other) const
    {
        return m_f < other.m_f;
    }
    bool
    operator<=(const HoldsInt& other) const
    {
        return m_f <= other.m_f;
    }
    bool
    operator>(const HoldsInt& other) const
    {
        return m_f > other.m_f;
    }
    bool
    operator>=(const HoldsInt& other) const
    {
        return m_f >= other.m_f;
    }


    int
    getf() const
    {
        return m_f;
    }

private:
    int m_f;
};

ostream&
operator<<(ostream& o, const HoldsInt& hi)
{
    o << "HoldsInt{" << hi.getf() << "}";
    return o;
}

TEST_CASE("missing", "[series]")
{
    mi<HoldsInt> mi1   = HoldsInt(1);
    mi<HoldsInt> mi1_1 = HoldsInt(1);
    mi<HoldsInt> mi2   = HoldsInt(2);
    mi<HoldsInt> mim1  = missing;
    mi<HoldsInt> mim2  = missing;

    REQUIRE(mi1 == mi1_1);
    REQUIRE(mi1 == HoldsInt(1));
    REQUIRE(HoldsInt(1) == mi1);
    REQUIRE(mi1 == mi1);
    REQUIRE(mim1 == mim2);
    REQUIRE(mim1 == missing);
    REQUIRE(missing == mim1);

    REQUIRE(mi1 != mi2);
    REQUIRE(mi1 != HoldsInt(2));
    REQUIRE(HoldsInt(2) != mi1);
    REQUIRE(mi1 != mim1);
    REQUIRE(mim1 != mi1);
    REQUIRE(HoldsInt(1) != mim1);
    REQUIRE(mim1 != HoldsInt(1));
    REQUIRE(mi1 != missing);
    REQUIRE(missing != mi1);

    REQUIRE(mi1 < mi2);
    REQUIRE(missing < mi2);
    REQUIRE(missing < mi1);

    REQUIRE(mi2 > mi1);
    REQUIRE(mi2 > missing);
    REQUIRE(mi1 > missing);

    REQUIRE(mi1 <= mi2);
    REQUIRE(missing <= mi2);
    REQUIRE(missing <= mi1);
    REQUIRE(mi1 <= mi1_1);
    REQUIRE(mi1 <= HoldsInt(1));
    REQUIRE(HoldsInt(1) <= mi1);
    REQUIRE(mi1 <= mi1);
    REQUIRE(mim1 <= mim2);
    REQUIRE(mim1 <= missing);
    REQUIRE(missing <= mim1);

    REQUIRE(mi2 >= mi1);
    REQUIRE(mi2 >= missing);
    REQUIRE(mi1 >= missing);
    REQUIRE(mi1 >= mi1_1);
    REQUIRE(mi1 >= HoldsInt(1));
    REQUIRE(HoldsInt(1) >= mi1);
    REQUIRE(mi1 >= mi1);
    REQUIRE(mim1 >= mim2);
    REQUIRE(mim1 >= missing);
    REQUIRE(missing >= mim1);
}

TEST_CASE("mean", "[series]")
{
    SECTION("double")
    {
        series<double> s{ 1, 2, 3, 4, 5 };
        REQUIRE(s.mean() == 3.0);
    }

    SECTION("int")
    {
        series<int> s{ 1, 2, 3, 4, 5 };
        REQUIRE(s.mean() == 3.0);
    }

    SECTION("unsigned")
    {
        series<unsigned> s{ 1, 2, 3, 4, 5 };
        REQUIRE(s.mean() == 3.0);
    }
}

TEST_CASE("stddev", "[series]")
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

struct no_default_ctor
{
    no_default_ctor() = delete;
    explicit no_default_ctor(int n)
        : num( n )
    {}

    //int val() const { return num; }
    bool operator<(const no_default_ctor& other) const
    {
        return num < other.num;
    }

    bool operator==(const no_default_ctor& other) const
    {
        return num == other.num;
    }

private:
    int num;
};

TEST_CASE("minmax", "[series]")
{
    SECTION("double")
    {
        {
            series<double> s{ 1, 2, 3, 4, 5 };
            auto [minv, maxv] = s.minmax();
            REQUIRE(minv == 1);
            REQUIRE(maxv == 5);
        }

        {
            series<double> s{ 5, 2, 3, 1, 4 };
            auto [minv, maxv] = s.minmax();
            REQUIRE(minv == 1);
            REQUIRE(maxv == 5);
        }

        {
            series<double> s{ -1, -2, -3, -4, -5 };
            auto [minv, maxv] = s.minmax();
            REQUIRE(minv == -5);
            REQUIRE(maxv == -1);
        }

        {
            series<double> s{ -5, -2, -3, -1, -4 };
            auto [minv, maxv] = s.minmax();
            REQUIRE(minv == -5);
            REQUIRE(maxv == -1);
        }
    }

    SECTION("int")
    {
        {
            series<int> s{ 1, 2, 3, 4, 5 };
            auto [minv, maxv] = s.minmax();
            REQUIRE(minv == 1);
            REQUIRE(maxv == 5);
        }

        {
            series<int> s{ 5, 2, 3, 1, 4 };
            auto [minv, maxv] = s.minmax();
            REQUIRE(minv == 1);
            REQUIRE(maxv == 5);
        }

        {
            series<int> s{ -1, -2, -3, -4, -5 };
            auto [minv, maxv] = s.minmax();
            REQUIRE(minv == -5);
            REQUIRE(maxv == -1);
        }

        {
            series<int> s{ -5, -2, -3, -1, -4 };
            auto [minv, maxv] = s.minmax();
            REQUIRE(minv == -5);
            REQUIRE(maxv == -1);
        }
    }

    SECTION("year_month_day")
    {
        {
            series<date::year_month_day> s{ 
                2001_y/7/8, 
                2001_y/7/9, 
                2001_y/7/10, 
                2001_y/7/11, 
                2001_y/7/12, 
            };
            auto [minv, maxv] = s.minmax();
            REQUIRE(minv == 2001_y/7/8);
            REQUIRE(maxv == 2001_y/7/12);
        }
        {
            series<date::year_month_day> s{ 
                2001_y/7/12, 
                2001_y/7/11, 
                2001_y/7/10, 
                2001_y/7/9, 
                2001_y/7/8, 
            };
            auto [minv, maxv] = s.minmax();
            REQUIRE(minv == 2001_y/7/8);
            REQUIRE(maxv == 2001_y/7/12);
        }
        {
            series<date::year_month_day> s{ 
                2001_y/7/10, 
                2001_y/7/12, 
                2001_y/7/9, 
                2001_y/7/8, 
                2001_y/7/11, 
            };
            auto [minv, maxv] = s.minmax();
            REQUIRE(minv == 2001_y/7/8);
            REQUIRE(maxv == 2001_y/7/12);
        }
    }
    SECTION("no_default_ctor")
    {
        {
            series<no_default_ctor> s{ 
                no_default_ctor(10), 
                no_default_ctor(12), 
                no_default_ctor(9), 
                no_default_ctor(8), 
                no_default_ctor(11), 
            };
            auto [minv, maxv] = s.minmax();
            REQUIRE(minv == no_default_ctor(8));
            REQUIRE(maxv == no_default_ctor(12));
        }
        {
            series<no_default_ctor> s{};
            REQUIRE_THROWS(s.minmax());
        }
    }
}

TEST_CASE("arithmetic", "[series]")
{
    SECTION("plus eq scalar")
    {
        series<double> s1{ 1, 2, 3, 4, 5 };
        unsigned int i = 11;
        s1 += i;
        REQUIRE(s1.size() == 5);
        REQUIRE(s1[0] == 12.0);
        REQUIRE(s1[1] == 13.0);
        REQUIRE(s1[2] == 14.0);
        REQUIRE(s1[3] == 15.0);
        REQUIRE(s1[4] == 16.0);
    }

    SECTION("minus eq scalar")
    {
        series<int> s1{ 1, 2, 3, 4, 5 };
        int i = 11;
        s1 -= i;
        REQUIRE(s1.size() == 5);
        REQUIRE(s1[0] == -10);
        REQUIRE(s1[1] ==  -9);
        REQUIRE(s1[2] ==  -8);
        REQUIRE(s1[3] ==  -7);
        REQUIRE(s1[4] ==  -6);
    }

    SECTION("multiply eq scalar")
    {
        series<double> s1{ 1, 2, 3, 4, 5 };
        unsigned int i = 11;
        s1 *= i;
        REQUIRE(s1.size() == 5);
        REQUIRE(s1[0] == 11.0);
        REQUIRE(s1[1] == 22.0);
        REQUIRE(s1[2] == 33.0);
        REQUIRE(s1[3] == 44.0);
        REQUIRE(s1[4] == 55.0);
    }

    SECTION("divide eq scalar")
    {
        series<double> s1{ 1, 2, 3, 4, 5 };
        unsigned int i = 10;
        s1 /= i;
        REQUIRE(s1.size() == 5);
        REQUIRE(s1[0] == Approx(0.1));
        REQUIRE(s1[1] == Approx(0.2));
        REQUIRE(s1[2] == Approx(0.3));
        REQUIRE(s1[3] == Approx(0.4));
        REQUIRE(s1[4] == Approx(0.5));
    }

    SECTION("modulus eq scalar")
    {
        series<int> s0{ 1, 2, 3, 4, 5 };
        series<int> s1 = s0;
        int i = 3;
        s1 %= i;
        REQUIRE(s0.size() == 5);
        REQUIRE(s1.size() == 5);
        REQUIRE(s0[0] == 1);
        REQUIRE(s0[1] == 2);
        REQUIRE(s0[2] == 3);
        REQUIRE(s0[3] == 4);
        REQUIRE(s0[4] == 5);
        REQUIRE(s1[0] == 1);
        REQUIRE(s1[1] == 2);
        REQUIRE(s1[2] == 0);
        REQUIRE(s1[3] == 1);
        REQUIRE(s1[4] == 2);
    }

    SECTION("plus eq vector")
    {
        series<double> s0{ 1, 2, 3, 4, 5 };
        series<double> s1 = s0;
        series<double> s2{ 10, 20, 30, 40, 50 };
        s1 += s2;
        REQUIRE(s0.size() == 5);
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s0[0] ==  1.0);
        REQUIRE(s0[1] ==  2.0);
        REQUIRE(s0[2] ==  3.0);
        REQUIRE(s0[3] ==  4.0);
        REQUIRE(s0[4] ==  5.0);
        REQUIRE(s1[0] == 11.0);
        REQUIRE(s1[1] == 22.0);
        REQUIRE(s1[2] == 33.0);
        REQUIRE(s1[3] == 44.0);
        REQUIRE(s1[4] == 55.0);
        REQUIRE(s2[0] == 10.0);
        REQUIRE(s2[1] == 20.0);
        REQUIRE(s2[2] == 30.0);
        REQUIRE(s2[3] == 40.0);
        REQUIRE(s2[4] == 50.0);
    }

    SECTION("minus eq vector")
    {
        series<double> s0{ 10, 20, 30, 40, 50 };
        series<double> s1 = s0;
        series<double> s2{ 1, 2, 3, 4, 5 };
        s1 -= s2;
        REQUIRE(s0.size() == 5);
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s0[0] == 10.0);
        REQUIRE(s0[1] == 20.0);
        REQUIRE(s0[2] == 30.0);
        REQUIRE(s0[3] == 40.0);
        REQUIRE(s0[4] == 50.0);
        REQUIRE(s1[0] ==  9.0);
        REQUIRE(s1[1] == 18.0);
        REQUIRE(s1[2] == 27.0);
        REQUIRE(s1[3] == 36.0);
        REQUIRE(s1[4] == 45.0);
        REQUIRE(s2[0] ==  1.0);
        REQUIRE(s2[1] ==  2.0);
        REQUIRE(s2[2] ==  3.0);
        REQUIRE(s2[3] ==  4.0);
        REQUIRE(s2[4] ==  5.0);
    }

    SECTION("multiply eq vector")
    {
        series<double> s0{ 1, 2, 3, 4, 5 };
        series<double> s1 = s0;
        series<double> s2{ 10, 20, 30, 40, 50 };
        s1 *= s2;
        REQUIRE(s0.size() == 5);
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s0[0] ==   1.0);
        REQUIRE(s0[1] ==   2.0);
        REQUIRE(s0[2] ==   3.0);
        REQUIRE(s0[3] ==   4.0);
        REQUIRE(s0[4] ==   5.0);
        REQUIRE(s1[0] ==  10.0);
        REQUIRE(s1[1] ==  40.0);
        REQUIRE(s1[2] ==  90.0);
        REQUIRE(s1[3] == 160.0);
        REQUIRE(s1[4] == 250.0);
        REQUIRE(s2[0] ==  10.0);
        REQUIRE(s2[1] ==  20.0);
        REQUIRE(s2[2] ==  30.0);
        REQUIRE(s2[3] ==  40.0);
        REQUIRE(s2[4] ==  50.0);
    }

    SECTION("divide eq vector")
    {
        series<double> s0{ 1, 2, 3, 4, 5 };
        series<double> s1 = s0;
        series<double> s2{ 10, 20, 30, 40, 50 };
        s1 /= s2;
        REQUIRE(s0.size() == 5);
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s0[0] ==  1.0);
        REQUIRE(s0[1] ==  2.0);
        REQUIRE(s0[2] ==  3.0);
        REQUIRE(s0[3] ==  4.0);
        REQUIRE(s0[4] ==  5.0);
        REQUIRE(s1[0] ==  0.1);
        REQUIRE(s1[1] ==  0.1);
        REQUIRE(s1[2] ==  0.1);
        REQUIRE(s1[3] ==  0.1);
        REQUIRE(s1[4] ==  0.1);
        REQUIRE(s2[0] == 10.0);
        REQUIRE(s2[1] == 20.0);
        REQUIRE(s2[2] == 30.0);
        REQUIRE(s2[3] == 40.0);
        REQUIRE(s2[4] == 50.0);
    }

    SECTION("modulus eq vector")
    {
        series<int> s0{ 11, 13, 17, 19, 23 };
        series<int> s1 = s0;
        series<int> s2{ 1, 2, 3, 5, 8 };
        s1 %= s2;
        REQUIRE(s0.size() == 5);
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s0[0] == 11);
        REQUIRE(s0[1] == 13);
        REQUIRE(s0[2] == 17);
        REQUIRE(s0[3] == 19);
        REQUIRE(s0[4] == 23);
        REQUIRE(s1[0] == 0);
        REQUIRE(s1[1] == 1);
        REQUIRE(s1[2] == 2);
        REQUIRE(s1[3] == 4);
        REQUIRE(s1[4] == 7);
        REQUIRE(s2[0] == 1);
        REQUIRE(s2[1] == 2);
        REQUIRE(s2[2] == 3);
        REQUIRE(s2[3] == 5);
        REQUIRE(s2[4] == 8);
    }

    SECTION("plus scalar")
    {
        series<double> s1{ 1, 2, 3, 4, 5 };
        unsigned int i = 11;
        series<double> s2 = s1 + i;
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s1[0] ==  1.0);
        REQUIRE(s1[1] ==  2.0);
        REQUIRE(s1[2] ==  3.0);
        REQUIRE(s1[3] ==  4.0);
        REQUIRE(s1[4] ==  5.0);
        REQUIRE(s2[0] == 12.0);
        REQUIRE(s2[1] == 13.0);
        REQUIRE(s2[2] == 14.0);
        REQUIRE(s2[3] == 15.0);
        REQUIRE(s2[4] == 16.0);
    }

    SECTION("minus scalar")
    {
        series<double> s1{ 1, 2, 3, 4, 5 };
        unsigned int i = 11;
        series<double> s2 = s1 - i;
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s1[0] ==  1.0);
        REQUIRE(s1[1] ==  2.0);
        REQUIRE(s1[2] ==  3.0);
        REQUIRE(s1[3] ==  4.0);
        REQUIRE(s1[4] ==  5.0);
        REQUIRE(s2[0] == -10.0);
        REQUIRE(s2[1] ==  -9.0);
        REQUIRE(s2[2] ==  -8.0);
        REQUIRE(s2[3] ==  -7.0);
        REQUIRE(s2[4] ==  -6.0);
    }

    SECTION("multiply scalar")
    {
        series<double> s1{ 1, 2, 3, 4, 5 };
        unsigned int i = 11;
        series<double> s2 = s1 * i;
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s1[0] ==  1.0);
        REQUIRE(s1[1] ==  2.0);
        REQUIRE(s1[2] ==  3.0);
        REQUIRE(s1[3] ==  4.0);
        REQUIRE(s1[4] ==  5.0);
        REQUIRE(s2[0] == 11.0);
        REQUIRE(s2[1] == 22.0);
        REQUIRE(s2[2] == 33.0);
        REQUIRE(s2[3] == 44.0);
        REQUIRE(s2[4] == 55.0);
    }

    SECTION("divide scalar")
    {
        series<double> s1{ 1, 2, 3, 4, 5 };
        unsigned int i = 10;
        series<double> s2 = s1 / i;
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s1[0] == 1.0);
        REQUIRE(s1[1] == 2.0);
        REQUIRE(s1[2] == 3.0);
        REQUIRE(s1[3] == 4.0);
        REQUIRE(s1[4] == 5.0);
        REQUIRE(s2[0] == Approx(0.1));
        REQUIRE(s2[1] == Approx(0.2));
        REQUIRE(s2[2] == Approx(0.3));
        REQUIRE(s2[3] == Approx(0.4));
        REQUIRE(s2[4] == Approx(0.5));
    }

    SECTION("modulus scalar")
    {
        series<int> s1{ 1, 2, 3, 4, 5 };
        int i = 3;
        series<int> s2 = s1 % i;
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s1[0] == 1);
        REQUIRE(s1[1] == 2);
        REQUIRE(s1[2] == 3);
        REQUIRE(s1[3] == 4);
        REQUIRE(s1[4] == 5);
        REQUIRE(s2[0] == 1);
        REQUIRE(s2[1] == 2);
        REQUIRE(s2[2] == 0);
        REQUIRE(s2[3] == 1);
        REQUIRE(s2[4] == 2);
    }

    SECTION("plus vector")
    {
        series<double> s1{ 1, 2, 3, 4, 5 };
        series<double> s2{ 10, 20, 30, 40, 50 };
        auto s3 = s1 + s2;
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s3.size() == 5);
        REQUIRE(s1[0] ==  1.0);
        REQUIRE(s1[1] ==  2.0);
        REQUIRE(s1[2] ==  3.0);
        REQUIRE(s1[3] ==  4.0);
        REQUIRE(s1[4] ==  5.0);
        REQUIRE(s2[0] == 10.0);
        REQUIRE(s2[1] == 20.0);
        REQUIRE(s2[2] == 30.0);
        REQUIRE(s2[3] == 40.0);
        REQUIRE(s2[4] == 50.0);
        REQUIRE(s3[0] == 11.0);
        REQUIRE(s3[1] == 22.0);
        REQUIRE(s3[2] == 33.0);
        REQUIRE(s3[3] == 44.0);
        REQUIRE(s3[4] == 55.0);
    }

    SECTION("minus vector")
    {
        series<double> s1{ 1, 2, 3, 4, 5 };
        series<double> s2{ 10, 20, 30, 40, 50 };
        auto s3 = s2 - s1;
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s3.size() == 5);
        REQUIRE(s1[0] ==  1.0);
        REQUIRE(s1[1] ==  2.0);
        REQUIRE(s1[2] ==  3.0);
        REQUIRE(s1[3] ==  4.0);
        REQUIRE(s1[4] ==  5.0);
        REQUIRE(s2[0] == 10.0);
        REQUIRE(s2[1] == 20.0);
        REQUIRE(s2[2] == 30.0);
        REQUIRE(s2[3] == 40.0);
        REQUIRE(s2[4] == 50.0);
        REQUIRE(s3[0] ==  9.0);
        REQUIRE(s3[1] == 18.0);
        REQUIRE(s3[2] == 27.0);
        REQUIRE(s3[3] == 36.0);
        REQUIRE(s3[4] == 45.0);
    }

    SECTION("multiply vector")
    {
        series<float> s1{ 1, 2, 3, 4, 5 };
        series<float> s2{ 10, 20, 30, 40, 50 };
        series<float> s3 = s1 * s2;
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s3.size() == 5);
        REQUIRE(s1[0] ==   1.0);
        REQUIRE(s1[1] ==   2.0);
        REQUIRE(s1[2] ==   3.0);
        REQUIRE(s1[3] ==   4.0);
        REQUIRE(s1[4] ==   5.0);
        REQUIRE(s2[0] ==  10.0);
        REQUIRE(s2[1] ==  20.0);
        REQUIRE(s2[2] ==  30.0);
        REQUIRE(s2[3] ==  40.0);
        REQUIRE(s2[4] ==  50.0);
        REQUIRE(s3[0] ==  10.0);
        REQUIRE(s3[1] ==  40.0);
        REQUIRE(s3[2] ==  90.0);
        REQUIRE(s3[3] == 160.0);
        REQUIRE(s3[4] == 250.0);
    }

    SECTION("divide vector")
    {
        series<double> s1{ 1, 2, 3, 4, 5 };
        series<double> s2{ 10, 20, 30, 40, 50 };
        series<double> s3 = s1 / s2;
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s3.size() == 5);
        REQUIRE(s1[0] ==  1.0);
        REQUIRE(s1[1] ==  2.0);
        REQUIRE(s1[2] ==  3.0);
        REQUIRE(s1[3] ==  4.0);
        REQUIRE(s1[4] ==  5.0);
        REQUIRE(s2[0] == 10.0);
        REQUIRE(s2[1] == 20.0);
        REQUIRE(s2[2] == 30.0);
        REQUIRE(s2[3] == 40.0);
        REQUIRE(s2[4] == 50.0);
        REQUIRE(s3[0] ==  0.1);
        REQUIRE(s3[1] ==  0.1);
        REQUIRE(s3[2] ==  0.1);
        REQUIRE(s3[3] ==  0.1);
        REQUIRE(s3[4] ==  0.1);
    }

    SECTION("modulus vector")
    {
        series<int> s1{ 11, 13, 17, 19, 23 };
        series<int> s2{ 1, 2, 3, 5, 8 };
        series<int> s3 = s1 % s2;
        REQUIRE(s1.size() == 5);
        REQUIRE(s2.size() == 5);
        REQUIRE(s3.size() == 5);
        REQUIRE(s1[0] == 11);
        REQUIRE(s1[1] == 13);
        REQUIRE(s1[2] == 17);
        REQUIRE(s1[3] == 19);
        REQUIRE(s1[4] == 23);
        REQUIRE(s2[0] == 1);
        REQUIRE(s2[1] == 2);
        REQUIRE(s2[2] == 3);
        REQUIRE(s2[3] == 5);
        REQUIRE(s2[4] == 8);
        REQUIRE(s3[0] == 0);
        REQUIRE(s3[1] == 1);
        REQUIRE(s3[2] == 2);
        REQUIRE(s3[3] == 4);
        REQUIRE(s3[4] == 7);
    }
}


