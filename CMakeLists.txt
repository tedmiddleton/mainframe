
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
# Rant:
# All of this should make referencing and using C++ libraries like miniframe 
# when located somewhere strange a breeze, but like with all things cmake, it's 
# more complicated than that. Really, with all of this machinery below, you'd 
# really think that this would produce a relocatable library that could be put 
# anywhere, that consuming targets would use INTERFACE_INCLUDE_DIR to populate 
# their command lines, and that you could put this lib wherever you wanted (ie 
# in a staging dir). Nope - you need to use CMAKE_PREFIX_DIR to find the 
# miniframe-config.cmake file (of course), but even though everything is set up 
# properly on the consuming side (INTERFACE* properties are a-ok in miniframe 
# target), for whatever mysterious reason cmake refuses to use that on the 
# compile command line of consuming targets. I guess this is because 
# @PACKAGE_INIT@ expands on invocation of cmake to generate the build system 
# (and it uses the system default prefix of course), but then cmake --install is 
# where we give it the install prefix, meaning the earlier bad prefix is baked 
# into the -config.cmake file? <sigh> This is beyond me. Maybe this just isn't 
# something cmake supports?

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

    

