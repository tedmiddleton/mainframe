# mainframe

![mainframe](docs/img/mainframe-medium.jpg)

[![GitHub Latest Release](https://img.shields.io/badge/release-prerelease-blue)](https://github.com/tedmiddleton/mainframe/releases/latest)
[![GitHub License](https://img.shields.io/github/license/tedmiddleton/mainframe)](https://github.com/tedmiddleton/mainframe/blob/main/LICENSE)
![Github CMake Workflow Status](https://img.shields.io/github/workflow/status/tedmiddleton/mainframe/CMake)
![Github Open Issues](https://img.shields.io/github/issues/tedmiddleton/mainframe)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/591c1f4b9ba949a8bd8a97cac42c181a)](https://app.codacy.com/gh/tedmiddleton/mainframe)
[![Cpp Badge](https://img.shields.io/badge/C%2B%2B%20version-17-blue?logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/17)


mainframe is a C++ dataframe library. It features C++ container ergonomics like iterators, copy-on-write column data management, and a flexible expression syntax that allows for efficient row filtering and column population. 

mainframe supports flexible data of nearly any type for columns including user-defined types.

```
frame<year_month_day, mi<double>, bool> f1;
f1.set_column_names( "date", "temperature" , "rain" );
f1.push_back( 2022_y/January/1, 8.9, false );
f1.push_back( 2022_y/January/2, 10.0, false );
f1.push_back( 2022_y/January/3, missing, true );
f1.push_back( 2022_y/January/4, 12.2, false );
f1.push_back( 2022_y/January/5, 13.3, false );
f1.push_back( 2022_y/January/6, 14.4, true );
f1.push_back( 2022_y/January/7, missing, false );
f1.push_back( 2022_y/January/8, 9.1, true );
f1.push_back( 2022_y/January/9, 9.3, false );
std::cout << f1;
```
 
```
  |       date | temperature |  rain
__|____________|_____________|_______
 0| 2022-01-01 |         8.9 | false
 1| 2022-01-02 |          10 | false
 2| 2022-01-03 |     missing |  true
 3| 2022-01-04 |        12.2 | false
 4| 2022-01-05 |        13.3 | false
 5| 2022-01-06 |        14.4 |  true
 6| 2022-01-07 |     missing | false
 7| 2022-01-08 |         9.1 |  true
 8| 2022-01-09 |         9.3 | false
```
 
mainframe supports an expression format that allows for simple row filtering. Here, `fcoldandrain` is a new dataframe that will contain a single row for `2022/January/8`:

```
auto fcoldandrain = f1.rows( _1 <= 12 && _2 == true && _0 > 2022_y/January/4 );
// could also have used
// auto fcoldandrain = f1.rows( 
//  []( auto&/*b*/, auto& c, auto&/*e*/ ) { 
//      return c->at( _1 ) <= 12 && c->at( _2 ) == true && c->at( _0 ) > 2022_y/January/4; } );
// b, c, and e are frame iterators

std::cout << fcoldandrain;
```
 
```
 |       date | temperature |  rain
_|____________|_____________|_______
0| 2022-01-08 |         9.1 |  true
```

Expressions also work well for creating new data columns

```
auto f2 = f1.new_series<year_month_day>( "next year", _0 + years(1) );
std::cout << f2;
```

```
  |       date | temperature |  rain |  next year
__|____________|_____________|_______|____________
 0| 2022-01-01 |         8.9 | false | 2023-01-01
 1| 2022-01-02 |          10 | false | 2023-01-02
 2| 2022-01-03 |        11.1 |  true | 2023-01-03
 3| 2022-01-04 |        12.2 | false | 2023-01-04
 4| 2022-01-05 |        13.3 | false | 2023-01-05
 5| 2022-01-06 |        14.4 |  true | 2023-01-06
 6| 2022-01-07 |        15.5 | false | 2023-01-07
 7| 2022-01-08 |         9.1 |  true | 2023-01-08
 8| 2022-01-09 |         9.3 | false | 2023-01-09
```
 
```
// column references like _0, _1, and _2 can have indexers to previous or 
//  following rows like _0[3] or _1[-1]
auto f3 = f2.new_series<mi<bool>>( "temp predicted rain", _1[-1] <= 10.0 && _2 == true ) );
std::cout << f3;
```

```
  |       date | temperature |  rain |  next year | temp predicted rain
__|____________|_____________|_______|____________|_____________________
 0| 2022-01-01 |         8.9 | false | 2023-01-01 |             missing
 1| 2022-01-02 |          10 | false | 2023-01-02 |               false
 2| 2022-01-03 |        11.1 |  true | 2023-01-03 |                true
 3| 2022-01-04 |        12.2 | false | 2023-01-04 |               false         
 4| 2022-01-05 |        13.3 | false | 2023-01-05 |               false
 5| 2022-01-06 |        14.4 |  true | 2023-01-06 |               false
 6| 2022-01-07 |        15.5 | false | 2023-01-07 |               false
 7| 2022-01-08 |         9.1 |  true | 2023-01-08 |               false
 8| 2022-01-09 |         9.3 | false | 2023-01-09 |               false
```

