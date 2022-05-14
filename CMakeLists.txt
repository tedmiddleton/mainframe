
#          Copyright Ted Middleton 2022.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required( VERSION 3.0 )
project( mainframe )

enable_testing()

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

if( NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if (MSVC)
    # C4456: declaration of *** hides previous local declaration
    # C4459: declaration of *** hides global declaration
    #  C445X aren't useful checks. Scoping is a *good* thing.
    # C4702: unreachable code
    #  C4702 is broken for "if constexpr ()" in msvc 17.1.2
    add_compile_options(/W4 /WX /wd4456 /wd4459 /wd4702)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

set( version 0.4.0 )

# mainframe ==================================================================

add_library( mainframe STATIC
    mainframe/base.hpp 
    mainframe/base.cpp 
    mainframe/csv.hpp 
    mainframe/series.hpp 
    mainframe/series_vector.hpp 
    mainframe/missing.hpp 
    mainframe/frame.hpp 
    mainframe/expression.hpp 
    )

add_subdirectory( tests )

# config =====================================================================

target_include_directories( mainframe
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

# cmake bric-a-brac ==========================================================

# install the target and create the export-set
install( TARGETS mainframe
         EXPORT mainframe-targets
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         RUNTIME DESTINATION bin
         INCLUDES DESTINATION include
    )

# install header files
install(
    FILES 
        mainframe/base.hpp 
        mainframe/series.hpp 
        mainframe/series_vector.hpp 
        mainframe/missing.hpp 
        mainframe/frame.hpp 
        mainframe/expression.hpp 
    DESTINATION include/mainframe 
    )

# generate and install export file
install( EXPORT mainframe-targets
         FILE mainframe-targets.cmake
         DESTINATION lib/cmake/mainframe
    )

set_property( TARGET mainframe PROPERTY VERSION ${version} )
set_property( TARGET mainframe PROPERTY SOVERSION 0 )
set_property( TARGET mainframe PROPERTY 
    INTERFACE_mainframe_MAJOR_VERSION 0 )
set_property( TARGET mainframe PROPERTY 
    COMPATIBLE_INTERFACE_STRING mainframe_MAJOR_VERSION )

# generate the version file for the config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mainframe-config-version.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
    )

# generate the export targets for the build tree
export(EXPORT mainframe-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/mainframe-targets.cmake"
)

# create config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/mainframe-config.cmake"
    INSTALL_DESTINATION lib/cmake/mainframe
    )

# install the config files
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/mainframe-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/mainframe-config-version.cmake"
    DESTINATION lib/cmake/mainframe
    )

    

