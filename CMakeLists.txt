
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

if (MSVC)
    # C4456: declaration of *** hides previous local declaration
    # C4459: declaration of *** hides global declaration
    #  C445X aren't useful checks. Scoping is a *good* thing.
    # C4702: unreachable code
    #  C4702 is broken for "if constexpr ()" in msvc 17.1.2
    add_compile_options(/W3 /WX /wd4456 /wd4459 /wd4702)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

set( version 0.8.0 )

# mainframe ==================================================================

add_library( mainframe STATIC
    mainframe/detail/base.cpp 
    mainframe/detail/base.hpp 
    mainframe/detail/expression.hpp 
    mainframe/detail/frame.hpp 
    mainframe/detail/frame_indexer.hpp 
    mainframe/detail/group.hpp 
    mainframe/detail/row_proxy.hpp 
    mainframe/detail/series_vector.hpp 
    mainframe/detail/simd.hpp 
    mainframe/detail/uframe.hpp 
    mainframe/detail/useries.hpp 
    mainframe/impl/frame.hpp 
    mainframe/impl/series.hpp 
    mainframe/columnindex.hpp 
    mainframe/expression.hpp 
    mainframe/frame.hpp 
    mainframe/frame_iterator.hpp 
    mainframe/frame_row.hpp 
    mainframe/group.hpp 
    mainframe/join.hpp 
    mainframe/missing.hpp 
    mainframe/row_decl.hpp 
    mainframe/series.hpp 
    )

add_subdirectory( tests )

# config =====================================================================

# Note that we need the generators here because no matter what arguments we give 
# it (ie we can give it a relative dir), target_include_directories() will 
# transform it to an absolute path to populate the INTERFACE_INCLUDE_DIRECTORIES 
# target property. And because the include dir needs to be different based on 
# whether it's in the install location (which it will be for users) or the build 
# tree (for mainframe devs or people who want to kick the tires without 
# installing mainframe), we need the generator expressions to make different 
# absolute include paths in these two contexts.
target_include_directories( mainframe
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )


# cmake bric-a-brac ==========================================================

# Install the target - really, libmainframe.a. This does nothing for the 
# headers. Note that we add the EXPORT parameter which creates an "export" 
# object which will then work with install( EXPORT ) to create target export 
# files in the install cmake directory. 
install( TARGETS mainframe
         EXPORT mainframe-targets
    )

# Install header files. This really seems like it should be handled as part of 
# install( TARGETS ) - in fact this seems like a sort of vestige of the pre-2.8 
# days without target objects. These are already listed underneath the target in 
# add_library, although not with public/private demarcation. The PUBLIC_HEADER 
# property seems like it should help here, but install( TARGETS ) doesn't seem 
# to use more than one in a list. At any rate, this seems to be what you need 
# to do to get your header files where you want them.
install(
    FILES 
        mainframe/base.hpp 
        mainframe/series.hpp 
        mainframe/series_vector.hpp 
        mainframe/simd.hpp 
        mainframe/missing.hpp 
        mainframe/frame.hpp 
        mainframe/frame_iterator.hpp 
        mainframe/grouped_frame.hpp 
        mainframe/expression.hpp 
    DESTINATION include/mainframe 
    )

# Generate and install to the install directory (eg /usr/lib/cmake/mainframe) an 
# export file named mainframe-targets.cmake. This will allow client libraries to 
# use find_package( mainframe ) to get the mainframe target object to use with 
# target_link_libraries()
install( EXPORT mainframe-targets
         DESTINATION lib/cmake/mainframe
    )

set_property( TARGET mainframe PROPERTY VERSION ${version} )
set_property( TARGET mainframe PROPERTY SOVERSION 0 )
set_property( TARGET mainframe PROPERTY 
    INTERFACE_mainframe_MAJOR_VERSION 0 )
set_property( TARGET mainframe PROPERTY 
    COMPATIBLE_INTERFACE_STRING mainframe_MAJOR_VERSION )

# Generate an export file named mainframe-targets.cmake and put it in the build 
# tree. Note that this mainframe-targets.cmake file is very different than the 
# mainframe-targets.cmake file created by install( EXPORT ) - the latter creates 
# a target object that references library and header files in the installation 
# location (eg /usr/ or /usr/local) whereas export( EXPORT ) creates a handy 
# mainframe-targets.cmake whose target object references the headers in the 
# cmake source tree and the library/libraries in the cmake build tree. This is 
# really useful if you want to reference/use mainframe from your project without 
# actually installing it on your system - you build it and then use the 
# include() command to open it. 
export(EXPORT mainframe-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/mainframe-targets.cmake"
)

# Generate the package version file in the build dir
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mainframe-config-version.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
    )

# Generate a config file in the build dir
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/mainframe-config.cmake"
    INSTALL_DESTINATION lib/cmake/mainframe
    )

# Install the config files into the install dir. This is necessary for 
# find_package( mainframe ) to work (in config mode)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/mainframe-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/mainframe-config-version.cmake"
    DESTINATION lib/cmake/mainframe
    )

    

