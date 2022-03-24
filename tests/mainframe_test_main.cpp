#include <iostream>
#include <ostream>
#include <x86intrin.h>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <mainframe/expression.hpp>
#include <mainframe/frame.hpp>
#include "date.h"

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

TEST_CASE( "mainframe" )
{
    frame f1 = make_frame<year_month_day, double, bool>( {"date", "temperature", "rain"} ); ;
    f1.push_back( 2022_y/January/2, 10.9, true );
    f1.push_back( 2022_y/January/3, 11.0, true );
    f1.push_back( 2022_y/January/4, 11.1, false );
    f1.push_back( 2022_y/January/5, 11.2, true );

    frame f2;
    f2.push_back<year_month_day, double, optional<bool>>( 2022_y/January/2, 10.9, true );
    f2.set_column_names( { "date", "temp", "weather" } );
    f2.push_back<year_month_day, double, optional<bool>>( 2022_y/January/3, 11.0, nullopt );
    f2.push_back<year_month_day, double, optional<bool>>( 2022_y/January/4, 11.1, false );
    f2.push_back<year_month_day, double, optional<bool>>( 2022_y/January/5, 11.2, true );
    f2.push_back<year_month_day, double, optional<bool>>( 2022_y/January/6, 11.3, true );
    f2.push_back<year_month_day, double, optional<bool>>( 2022_y/January/7, 11.2, true );
    f2.push_back<year_month_day, double, optional<bool>>( 2022_y/January/8, 11.1, true );
    f2.push_back<year_month_day, double, optional<bool>>( 2022_y/January/9, 11.0, true );

    frame temperaturef = f2.columns( "date", "temp" );
    frame rainf = f2.columns( "date", "weather" ).rows( col<optional<bool>>("weather") != nullopt );

    cout << std::boolalpha;
    cout << "f2 is " << f2.size() << "\n";
    for ( auto r : f2 ) 
    //for ( auto it = f2.begin(); it != f2.end(); ++it ) 
    {
        //auto r = *it;
        auto b = r.get<optional<bool>>( "weather" );
        if ( b != nullopt ) {
            cout << "Row: " << r.get<year_month_day>( "date" ) << ", " <<
                r.get<double>( "temp" ) << ", " << *b << "\n";
        }
        else {
            cout << "Row: " << r.get<year_month_day>( "date" ) << ", " <<
                r.get<double>( "temp" ) << ", " << "NULL weather" << "\n";
        }
    }

}

//TEST_CASE( "mainframe", "[experiments]" )
//{
//    vector<unsigned long>  idx_col1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
//    constexpr auto start = 2021_y/March/25;
//    vector<year_month_day> date_col;
//    for ( auto i = 0; i < 10; ++i ) {
//        date_col.push_back( start + date::days{ i } );
//    }
//
//    vector<int>            int_col1 = { 1, 2, -3, -4, 5, 6, 7, 8, 9, -10 };
//    vector<double>         dbl_col1 =
//        { 0.01, 0.02, 0.03, 0.03, 0.05, 0.06, 0.03, 0.08, 0.09, 0.03 };
//    vector<int>            int_col2( 20, -1 );
//
//    mf::Frame frame;
//    frame.add_series<int>( "Ints", { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } );
//    frame.add_series<double>( "Doubles", { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10 } );
//    frame.add_series<float>( "Other", { 2.0, 3.0, 5.0, 6.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0 } );
//    //auto ix = { int_col2.begin(), int_col2.begin()+10 };
//    //TD<decltype(ix)> ixx;
//    //frame.add_series<int>( "More Ints", ix );
//    //frame.add_series( "dates", Series<year_month_day>( date_col ) );
//    cout << frame << endl;
//
//    mf::Frame frame2;
//    frame2.add_series<double>( "Doobles", { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10 } );
//    frame2.add_series<double>( "Other2", { 2.0, 3.0, 5.0, 6.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0 } );
//    frame2.add_series<int>( "Ints", { 1, 2, 3, 8, 5, 6, 7, 8, 9, 10 } );
//    cout << frame2 << endl;
//
//    // loading one column at a time
// 
//    // loading multiple columns
//
//    // selecting into new table
//    
//    // selecting into view
//}

struct Foo 
{
    Foo() = default;
    Foo( int _i ) : i_(_i) {}
    int i_;
};

//Frame test_frame_copy()
//{
//    Frame f;
//    f.add_series<int>( "oneseries", {1, 2, 3, 4} );
//    f.add_series<int>( "oneseries", {1, 2, 3, 4} );
//    //f.add_series<string>( "twoseries", {"1", "2", "3", "4"} );
//    Series<Foo> s1{ 1, 2, 3, 4 };
//    //f.add_series<Foo>( "twoseries", {1, 2, 3, 4} );
//    f.add_series<Foo>( "twoseries", s1 );
//    return f;
//}

//TEST_CASE( "mainframe add_series tests", "[mainframe]" )
//{
//    SECTION( "multiple" )
//    {
//        Frame frame;
//        vector<optional<int>> col1 = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
//        auto s1 = Series<optional<int>>( std::move( col1 ) );
//        frame.add_series<optional<int>>( "col1", std::move( s1 ) );
//        auto s2 = Series<double>{ 1.4, 2.4, 3.4, 4.4, 5.4, 6.4, 7.4, 8.4, 9.5 } ;
//        frame.add_series<double>( "col2", s2 );
//        frame.add_series<long>( "col3", { 20, 21, 22, 23, 24, 25, 26, 27, 28 } );
//        frame.add_series<string>( "col4", 
//            { "30", "31", "32", "33", "34", "35", "36", "37", "38" } );
//        
//        REQUIRE_THROWS_AS( frame.at<int>( "col1", 10 ), std::out_of_range );
//        REQUIRE_THROWS_AS( frame.at<double>( "col1", 0 ), std::invalid_argument );
//        REQUIRE_THROWS_AS( frame.at<long>( "col1", 0 ), std::invalid_argument );
//        REQUIRE_THROWS_AS( frame.at<string>( "col1", 0 ), std::invalid_argument );
//
//        auto val4 = frame.at_opt<int>( "col1", 4 );
//        REQUIRE( val4.has_value() );
//        REQUIRE( val4 == 14 );
//
//        auto val9 = frame.at_opt<long>( "col3", 9 );
//        REQUIRE( !val9.has_value() );
//
//        cout << frame << endl;
//    }
//
//    SECTION( "basic double" )
//    {
//        mf::Frame frame;
//        Series<double> s1 = { 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0 };
//        frame.add_series( "col1", std::move( s1 ) );
//        auto val4 = frame.at<double>( "col1", 4 );
//        REQUIRE( val4 == 14.0 );
//        REQUIRE_THROWS_AS( frame.at<double>( "col1", 11 ), std::out_of_range );
//        REQUIRE_THROWS_AS( frame.at<int>( "col1", 4 ), std::invalid_argument );
//    }
//
//    SECTION( "user type with stream op" )
//    {
//        mf::Frame frame;
//        vector<optional<HasStreamOp>> col1 = { 
//            HasStreamOp{.d=10.0,.i=100},
//            HasStreamOp{.d=11.0,.i=110},
//            HasStreamOp{.d=12.0,.i=120},
//            HasStreamOp{.d=13.0,.i=130},
//            HasStreamOp{.d=14.0,.i=140},
//            HasStreamOp{.d=15.0,.i=150},
//            HasStreamOp{.d=16.0,.i=160},
//            HasStreamOp{.d=17.0,.i=170} };
//        frame.add_series( "col1", col1 );
//        auto val4 = frame.at_opt<HasStreamOp>( "col1", 4 );
//        REQUIRE( val4 );
//        REQUIRE( val4->i == 140 );
//        REQUIRE( val4->d == 14.0 );
//    }
//
//    SECTION( "user type no stream op" )
//    {
//        mf::Frame frame;
//        vector<optional<HasNoStreamOp>> col1 = { 
//            HasNoStreamOp{.d=10.0,.i=100},
//            HasNoStreamOp{.d=11.0,.i=110},
//            HasNoStreamOp{.d=12.0,.i=120},
//            HasNoStreamOp{.d=13.0,.i=130},
//            HasNoStreamOp{.d=14.0,.i=140},
//            HasNoStreamOp{.d=15.0,.i=150},
//            HasNoStreamOp{.d=16.0,.i=160},
//            HasNoStreamOp{.d=17.0,.i=170} };
//        frame.add_series( "col1", col1 );
//        auto val4 = frame.at_opt<HasNoStreamOp>( "col1", 4 );
//        REQUIRE( val4 );
//        REQUIRE( val4->i == 140 );
//        REQUIRE( val4->d == 14.0 );
//    }
//
//    SECTION( "year_month_day" )
//    {
//        vector<optional<int>> int_col1 = { 1, 2, -3, -4, 5, 6, 7, 8, 9, -10 };
//        int_col1[ 3 ] = nullopt;
//        vector<year_month_day> dt_col;
//        year_month_day ymd = 2021_y/March/25;
//        for ( auto i = 0; i < 10; ++i )
//            dt_col.push_back( ymd + date::days(i) );
//        mf::Frame frame;
//        frame.add_series( "dates", dt_col );
//        frame.add_series( "ints", SeriesOpt<int>{ int_col1 } );
//        REQUIRE( !frame.at<int>( "ints", 3 ) );
//        REQUIRE( frame.at<year_month_day>( "dates", 3 ) == 2021_y/March/28 );
//        cout << frame << endl;
//    }
//
//    SECTION( "copying" )
//    {
//        Frame frame;
//        frame.add_series<optional<int>>(    "col1", { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 } );
//        frame.add_series<optional<double>>( "col2", { 1.4, 2.4, 3.4, 4.4, 5.4, 6.4, 7.4, 8.4, 9.5 } );
//        frame.add_series<optional<long>>(   "col3", { 20, 21, 22, 23, 24, 25, 26, 27, 28 } );
//
//        Frame frame2 = frame;
//        REQUIRE( frame.size() == 10 );
//        REQUIRE( frame2.size() == 10 );
//        auto col2_3 = frame2.at_opt<double>( "col2", 3 );
//        auto col2_4 = frame2.at_opt<double>( "col2", 4 );
//        auto col2_5 = frame2.at_opt<double>( "col2", 5 );
//        auto col1_3 = frame2.at_opt<int>( "col1", 3 );
//        auto col1_4 = frame2.at_opt<int>( "col1", 4 );
//        auto col1_5 = frame2.at_opt<int>( "col1", 5 );
//        auto col3_3 = frame2.at_opt<long>( "col3", 3 );
//        auto col3_4 = frame2.at_opt<long>( "col3", 4 );
//        auto col3_5 = frame2.at_opt<long>( "col3", 5 );
//        REQUIRE( col1_3 );
//        REQUIRE( col1_4 );
//        REQUIRE( col1_5 );
//        REQUIRE( col2_3 );
//        REQUIRE( col2_4 );
//        REQUIRE( col2_5 );
//        REQUIRE( col3_3 );
//        REQUIRE( col3_4 );
//        REQUIRE( col3_5 );
//        REQUIRE( *col1_3 == 13 );
//        REQUIRE( *col1_4 == 14 );
//        REQUIRE( *col1_5 == 15 );
//        REQUIRE( *col2_3 == 4.4 );
//        REQUIRE( *col2_4 == 5.4 );
//        REQUIRE( *col2_5 == 6.4 );
//        REQUIRE( *col3_3 == 23 );
//        REQUIRE( *col3_4 == 24 );
//        REQUIRE( *col3_5 == 25 );
//    }
//
//    SECTION( "assignment" )
//    {
//        Frame frame;
//        frame.add_series<int>(    "col1", { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 } );
//        frame.add_series<double>( "col2", { 1.4, 2.4, 3.4, 4.4, 5.4, 6.4, 7.4, 8.4, 9.5 } );
//        frame.add_series<long>(   "col3", { 20, 21, 22, 23, 24, 25, 26, 27, 28 } );
//
//        Frame frame2{};
//        frame2.add_series<bool>( "gone", { true, false, true } );
//        REQUIRE( frame2.size() == 3 );
//        frame2 = frame;
//        REQUIRE( frame.size() == 10 );
//        REQUIRE( frame2.size() == 10 );
//    }
//
//    SECTION( "moving" )
//    {
//        Frame frame;
//        frame.add_series<int>(    "col1", { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 } );
//        frame.add_series<double>( "col2", { 1.4, 2.4, 3.4, 4.4, 5.4, 6.4, 7.4, 8.4, 9.5 } );
//        frame.add_series<long>(   "col3", { 20, 21, 22, 23, 24, 25, 26, 27, 28 } );
//
//        Frame frame2{ std::move( frame ) };
//        REQUIRE( frame.size() == 0 );
//        REQUIRE( frame2.size() == 10 );
//    }
//
//    SECTION( "vector" )
//    {
//        auto v = vector<Frame>{ Frame{}, Frame{}, Frame{} };
//    }
//
//    SECTION( "map" )
//    {
//        map<string, Frame> m;
//        m.insert( pair<string, Frame>( "one", Frame{} ) );
//        m.insert( pair<string, Frame>( "two", Frame{} ) );
//        m.insert( pair<string, Frame>( "three", Frame{} ) );
//        m[ "four" ] = Frame{};
//    }
//
//    //SECTION( "subframe" )
//    //{
//    //    vector<unsigned long>  ul_col1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
//    //    vector<year_month_day> dt_col;
//    //    auto sdate = date::sys_days{ year{2021}/March/25 };
//    //    for ( auto i = 0; i < 10; ++i )
//    //        dt_col.push_back( year_month_day{ sdate + date::days(i) } );
//    //    vector<int>            int_col1 = { 1, 2, -3, -4, 5, 6, 7, 8, 9, -10 };
//    //    vector<double>         dbl_col1 =
//    //        { 0.01, 0.02, 0.03, 0.03, 0.05, 0.06, 0.03, 0.08, 0.09, 0.03 };
//    //    vector<int>            int_col2( 10, -1 );
//    //    vector<bool>           missing1( 10, false );
//    //    missing1[ 3 ] = true;
//    //    vector<bool>           missing2( 10, false );
//    //    missing2[ 7 ] = true;
//    //    vector<HasStreamOp> hso_col1 = { 
//    //        HasStreamOp{.d=10.0,.i=100},
//    //        HasStreamOp{.d=11.0,.i=110},
//    //        HasStreamOp{.d=12.0,.i=120},
//    //        HasStreamOp{.d=13.0,.i=130},
//    //        HasStreamOp{.d=14.0,.i=140},
//    //        HasStreamOp{.d=15.0,.i=150},
//    //        HasStreamOp{.d=16.0,.i=160},
//    //        HasStreamOp{.d=17.0,.i=170},
//    //        HasStreamOp{.d=18.0,.i=180},
//    //        HasStreamOp{.d=19.0,.i=190} };
//
//    //    mf::Frame frame;
//    //    frame.add_series( "unsigned longs", ul_col1, missing1 );
//    //    frame.add_series( "dates", dt_col );
//    //    frame.add_series( "ints", int_col1, missing1 );
//    //    frame.add_series( "doubles", dbl_col1 );
//    //    frame.add_series( "more ints", int_col2 );
//    //    frame.add_series( "hso", hso_col1, missing2 );
//    //    REQUIRE( frame.at<int>( "ints", 2 ) == -3 );
//    //    REQUIRE( frame.at<int>( "ints", 3 ).has_value() );
//    //    REQUIRE( frame.at<double>( "doubles", 9 ) == 0.03 );
//    //    REQUIRE( frame.at<unsigned long>( 3, "unsigned longs" ).has_value() );
//    //    REQUIRE( frame.at<long>( 4, "unsigned longs" ).has_value() );
//    //    REQUIRE( frame.at<unsigned long>( 4, "unsigned longs" ) == 5UL );
//    //    REQUIRE( frame.at<HasStreamOp>( "hso", 4 )->i == 140 );
//    //    REQUIRE( frame.at<HasStreamOp>( "hso", 8 )->d == 18.0 );
//    //    REQUIRE( !frame.at<HasStreamOp>( "hso", 8 ).has_value() );
//    //    REQUIRE( frame.at<HasStreamOp>( "hso", 7 ).has_value() );
//
//    //    mf::ColumnSelector colsel{ .columns{ "dates", "ints", "hso" } };
//    //    mf::RowSelector rowsel{ .rows{ 1, 2, 3, 5, 8 } };
//    //    auto newframe = frame.at( rowsel, colsel );
//    //    REQUIRE( frame.at<int>( "ints", 1 ) == 2 );
//    //    REQUIRE( frame.at<int>( "ints", 2 ) == -3 );
//    //    REQUIRE( frame.at<int>( "ints", 3 ).has_value() );
//    //    REQUIRE( frame.at<HasStreamOp>( "hso", 4 )->i == 140 );
//    //    REQUIRE( frame.at<HasStreamOp>( "hso", 8 )->d == 18.0 );
//    //    REQUIRE( !frame.at<HasStreamOp>( "hso", 8 ).has_value() );
//    //    REQUIRE( frame.at<HasStreamOp>( "hso", 7 ).has_value() );
//    //}
//}

//TEST_CASE( "mainframe csv tests", "[mainframe]" )
//{
//    SECTION( "from_csv" )
//    {
//        auto csv = 
//            "Month,YYYY,ICS_ALL"
//            "November,1952,86.2"
//            "February,1953,90.7"
//            "August,1953,80.8"
//            "November,1953,80.7"
//            "February,1954,82"
//            "May,1954,82.9"
//            "November,1954,87"
//            "February,1955,95.9";
//        istringstream ss{ csv };
//        mf::Frame frame = mf::Frame::from_csv( ss );
//        REQUIRE( frame.size() == 8 );
//    }
//}

