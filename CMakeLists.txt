
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
#set( CMAKE_BUILD_TYPE Debug )

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
    miniframe/expression.hpp 
    src/frame.cpp 
    src/csv.cpp 
    )

#add_subdirectory( tests )

# config =====================================================================

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

# cmake bric-a-brac ==========================================================
# This creates 4 files and installs our libs and headers in our prefix dir 
# location. The 4 files are
# miniframe-config.cmake - this is what find_package() looks for, and it 
#   includes <lib>-targets.cmake and checks for required components in the 
#   find_package( ... COMPONENTS <component> [<component>...] ) list
# miniframe-config-version.cmake - find_package() looks for this to check 
#   against the requested version in find_package( ... VERSION <ver> ... )
# miniframe-targets.cmake - this sets the actual import targets with 
#   add_library( <lib> ... IMPORTED ... ) and attaches properties to the targets
# miniframe-targets-noconfig.cmake - this adds config-specific lib paths 
#   (Debug/Release/etc)
# All of this should make referencing and using C++ libraries like miniframe 
# when located somewhere strange a breeze, but like with all things cmake, it's 
# more complicated than that. In particular, the fact that I've used #include 
# <miniframe/HDR.hpp> rather than #include "miniframe/HDR.hpp" all over the 
# place seems to have doomed consuming list files to never finding these headers 
# when they're located outside the system header locations (usr/include etc).

# install the target and create the export-set
install( TARGETS miniframe miniframe_series miniframe_base
         EXPORT miniframe-targets
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         RUNTIME DESTINATION bin
         INCLUDES DESTINATION include
    )

# install header files
install(
    FILES 
        miniframe/frame.hpp 
        miniframe/expression.hpp 
        miniframe/series.hpp 
        miniframe/series_vector.hpp 
        miniframe/series_iterator.hpp 
        miniframe/base.hpp 
    DESTINATION include/miniframe 
    )

# generate and install export file
install( EXPORT miniframe-targets
         FILE miniframe-targets.cmake
         DESTINATION lib/cmake/miniframe
    )

set_property( TARGET miniframe PROPERTY VERSION ${version} )
set_property( TARGET miniframe PROPERTY SOVERSION 0 )
set_property( TARGET miniframe PROPERTY 
    INTERFACE_miniframe_MAJOR_VERSION 0 )
set_property( TARGET miniframe PROPERTY 
    COMPATIBLE_INTERFACE_STRING miniframe_MAJOR_VERSION )

# generate the version file for the config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/miniframe-config-version.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
    )

# generate the export targets for the build tree
export(EXPORT miniframe-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/miniframe-targets.cmake"
)

# create config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/miniframe-config.cmake"
    INSTALL_DESTINATION lib/cmake/miniframe
    )

# install the config files
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/miniframe-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/miniframe-config-version.cmake"
    DESTINATION lib/cmake/miniframe
    )

    

