cmake_minimum_required(VERSION 3.14)
project(SearchServer VERSION 2.0 LANGUAGES CXX)

option (TESTING "Compile and run tests" ON)

set (search_server
    "include/concurrent_map.h"

    "include/document.h"
    "src/document.cpp"

    "include/paginator.h"

    "include/process_queries.h"
    "src/process_queries.cpp"

    "include/read_input_functions.h"
    "src/read_input_functions.cpp"

    "include/request_queue.h"
    "src/request_queue.cpp"

    "include/remove_duplicates.h"
    "src/remove_duplicates.cpp"

    "include/search_server.h"
    "src/search_server.cpp"

    "include/string_processing.h"
    "src/string_processing.cpp"
    )

add_executable(SearchServer main.cpp ${search_server} ${test_utils} ${search_server_tests})

target_link_libraries(SearchServer PRIVATE -ltbb -lpthread)

target_include_directories(SearchServer PRIVATE "include")

set_target_properties(SearchServer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

if (MSVC)
    target_compile_options(SearchServer PRIVATE /W3 /WX)
else ()
    target_compile_options(SearchServer PRIVATE
        -ltbb
        -lpthread
        -Werror
        -Wall
        -Wextra
        -Wpedantic
        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wenum-compare
        -Wfloat-equal
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wsign-conversion
        -Wsign-promo
        -pedantic
        -pedantic-errors
        )
endif ()

if (TESTING)

set (test_utils
    "include/log_duration.h"

    "include/test_framework.h"
    "src/test_framework.cpp"
    )

set (search_server_tests
    "include/search_server_tests.h"
    "src/search_server_tests.cpp"

    "include/test_example_functions.h"
    "src/test_example_functions.cpp"
    )

add_executable(Search_server_tests "src/tests.cpp" ${search_server} ${test_utils} ${search_server_tests})
target_include_directories(Search_server_tests PRIVATE "include")

set_target_properties(Search_server_tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )

if (MSVC)
    target_compile_options(Search_server_tests PRIVATE /W3 /WX)
else ()
    target_compile_options(Search_server_tests PRIVATE
        -ltbb
        -lpthread
        -Werror
        -Wall
        -Wextra
        -Wpedantic
        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wenum-compare
        -Wfloat-equal
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wsign-conversion
        -Wsign-promo
        -pedantic
        -pedantic-errors
        )
endif ()

target_link_libraries(Search_server_tests PRIVATE -ltbb -lpthread)

enable_testing()
add_test (Tests Search_server_tests)
set_tests_properties (Tests PROPERTIES
        PASS_REGULAR_EXPRESSION "OK"
        FAIL_REGULAR_EXPRESSION "fail")

endif ()
