cmake_minimum_required( VERSION 3.0 )

project(mftest)

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

option( USE_MAINFRAME_BUILD_DIR "Use the mainframe headers/libs in the build dir" OFF )

if( NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

add_compile_options(-Wall -Wextra -pedantic -Werror)
add_compile_options(-march=native)

# find_package will use /usr/local/lib/cmake/mainframe/mainframe-targets.cmake 
# and reference
# /usr/include/mainframe/frame.hpp
# /usr/include/mainframe/frame_iterator.hpp
# ...
# /usr/lib/libmainframe.a
#
# install will use /home/bob/mainframe/_build/cmake/mainframe-targets.cmake and 
# reference
# /home/bob/mainframe/mainframe/frame.hpp
# /home/bob/mainframe/mainframe/frame_iterator.hpp
# ...
# /home/bob/mainframe/_build/libmainframe.a
if (USE_MAINFRAME_BUILD_DIR)
include( /home/bob/mainframe/_build/cmake/mainframe-targets.cmake )
else()
find_package( mainframe REQUIRED )
endif()

add_executable( mftest
    mftest.cpp
    )

target_link_libraries( mftest
    PRIVATE
        mainframe
    )

