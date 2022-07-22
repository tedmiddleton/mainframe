cmake_minimum_required( VERSION 3.0 )

option( ENABLE_SIMD_IN_TESTS "Enable SIMD optimizations in tests (default)" ON )

if (ENABLE_SIMD_IN_TESTS)
    if (MSVC)
        add_compile_options(/arch:AVX)
    else()
        add_compile_options(-march=native)
    endif()
endif()

# mainframe_test =============================================================

add_executable( mainframe_series_vector_test
    mainframe_series_vector_test_main.cpp
    )

target_link_libraries( mainframe_series_vector_test
    PRIVATE
        mainframe
    )

add_test( mainframe_series_vector_test 
    mainframe_series_vector_test )

add_executable( mainframe_series_test
    mainframe_series_test_main.cpp
    )

target_link_libraries( mainframe_series_test
    PRIVATE
        mainframe
    )

add_test( mainframe_series_test mainframe_series_test )

add_executable( mainframe_test
    mainframe_test_main.cpp
    )

target_link_libraries( mainframe_test
    PRIVATE
        mainframe
    )

add_test( mainframe_test mainframe_test )

