macro(add_cache_test _TEST_NAME)
    add_executable(${_TEST_NAME}_tests
            ${_TEST_NAME}_tests.cpp)
    add_coverage_flags(${_TEST_NAME}_tests)
    set_property(TARGET ${_TEST_NAME}_tests PROPERTY CXX_STANDARD 20)
    target_link_libraries(${_TEST_NAME}_tests
            ${GTEST_MAIN_LIBRARIES} gtl caches)
    target_include_directories(${_TEST_NAME}_tests PRIVATE ${GTEST_INCLUDE_DIRS})
    target_compile_options(${_TEST_NAME}_tests PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:$<IF:$<VERSION_GREATER_EQUAL:${MSVC_TOOLSET_VERSION},142>,/W4 /WX,>>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
            $<$<CXX_COMPILER_ID:GNU>:-Wno-interference-size>)
    add_test(NAME cache_${_TEST_NAME}_tests COMMAND ${_TEST_NAME}_tests)
    target_sources(${_TEST_NAME}_tests
        PRIVATE
        FILE_SET test_helpers
        TYPE HEADERS
        FILES
        test_helper.hpp)
endmacro()

include(FetchContent)
FetchContent_Declare(
    gtl
    GIT_REPOSITORY https://github.com/greg7mdp/gtl.git
    GIT_TAG        v1.2.0 # adjust tag/branch/commit as needed
)
FetchContent_MakeAvailable(gtl)

add_cache_test(general)
add_cache_test(fifo)
add_cache_test(lfu)
add_cache_test(lru)
add_cache_test(no_eviction)
add_cache_test(wrapper_policy)
