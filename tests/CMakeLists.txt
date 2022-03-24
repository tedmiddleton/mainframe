cmake_minimum_required( VERSION 3.0 )

# mainframe_test =============================================================

add_executable( mainframe_test
    mainframe_test_main.cpp
    )

target_link_libraries( mainframe_test
    PRIVATE
        mainframe
    )

add_executable( mainframe_series_test
    mainframe_series_test_main.cpp
    )

target_link_libraries( mainframe_series_test
    PRIVATE
        mainframe_series
    )

add_executable( mainframe_series_vector_test
    mainframe_series_vector_test_main.cpp
    )

target_link_libraries( mainframe_series_vector_test
    PRIVATE
        mainframe_series
    )


