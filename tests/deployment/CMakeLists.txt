cmake_minimum_required( VERSION 3.0 )

project(mftest)

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

if( NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

add_compile_options(-Wall -Wextra -pedantic -Werror)
add_compile_options(-march=native)

# Will this work?
find_package( mainframe REQUIRED )

add_executable( mftest
    mftest.cpp
    )

target_link_libraries( mftest
    PRIVATE
        mainframe
    )

