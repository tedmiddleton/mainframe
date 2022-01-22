
#          Copyright Ted Middleton 2022.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required( VERSION 3.0 )
project( miniframe )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( CMAKE_CXX_FLAGS "-g -O2 -Wall" )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set( CMAKE_BUILD_TYPE Debug )

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set( version 0.1.0 )

# miniframe_base =============================================================

add_library( miniframe_base STATIC
    miniframe/base.hpp 
    src/base.cpp 
    )

target_include_directories( miniframe_base
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

# miniframe_series ===========================================================

add_library( miniframe_series STATIC
    miniframe/series.hpp 
    miniframe/series_vector.hpp 
    miniframe/series_iterator.hpp 
    src/series.cpp 
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
    miniframe/frame.hpp 
    src/frame.cpp 
    src/csv.cpp 
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

set_property( TARGET miniframe PROPERTY VERSION ${version} )
set_property( TARGET miniframe PROPERTY SOVERSION 0 )
set_property( TARGET miniframe PROPERTY 
    INTERFACE_miniframe_MAJOR_VERSION 0 )
set_property( TARGET miniframe PROPERTY 
    COMPATIBL_INTERFACE_STRING miniframe_MAJOR_VERSION )

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/miniframe-config-version.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
    )

install( TARGETS miniframe miniframe_series miniframe_base
         EXPORT miniframe_targets
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         RUNTIME DESTINATION bin
         INCLUDES DESTINATION include
    )

# install header file
install(
    FILES 
        miniframe/frame.hpp 
        miniframe/series.hpp 
        miniframe/series_vector.hpp 
        miniframe/series_iterator.hpp 
        miniframe/base.hpp 
    DESTINATION include/miniframe 
    )

# generate and install export file
install( EXPORT miniframe_targets
         FILE miniframe_targets.cmake
         DESTINATION lib/cmake
    )

# create config file
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/miniframe-config.cmake"
    INSTALL_DESTINATION lib/cmake
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/miniframe-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/miniframe-config-version.cmake"
    DESTINATION lib/cmake
    )

    

