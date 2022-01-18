cmake_minimum_required( VERSION 3.0 )
project( miniframe )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( CMAKE_CXX_FLAGS "-g -O2 -Wall" )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set( CMAKE_BUILD_TYPE Debug )

include(GNUInstallDirs)

# miniframe_base =============================================================

add_library( miniframe_base STATIC
    miniframe_base.hpp 
    miniframe_base.cpp 
    )

target_include_directories( miniframe_base
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

# miniframe_series ===========================================================

add_library( miniframe_series STATIC
    miniframe_series.hpp 
    miniframe_series_vector.hpp 
    miniframe_series_iterator.hpp 
    miniframe_series.cpp 
    )

target_include_directories( miniframe_series
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

target_link_libraries( miniframe_series
    PUBLIC
        miniframe_base
    )

# miniframe ==================================================================

add_library( miniframe STATIC
    miniframe.hpp 
    miniframe.cpp 
    miniframe_csv.cpp 
    )

target_include_directories( miniframe
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

target_link_libraries( miniframe
    PUBLIC
        miniframe_base
        miniframe_series
    )

add_subdirectory( tests )


