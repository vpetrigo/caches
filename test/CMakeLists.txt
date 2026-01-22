macro(add_cache_test _TEST_NAME)
    add_executable(${_TEST_NAME}_tests
            ${_TEST_NAME}_tests.cpp)
    add_coverage_flags(${_TEST_NAME}_tests)
    target_link_libraries(${_TEST_NAME}_tests
            ${GTEST_MAIN_LIBRARIES} caches)
    target_include_directories(${_TEST_NAME}_tests PRIVATE ${GTEST_INCLUDE_DIRS} ${parallel-hashmap_SOURCE_DIR})
    target_compile_definitions(${_TEST_NAME}_tests PRIVATE $<$<BOOL:${USE_CUSTOM_HASHMAP}>:CUSTOM_HASHMAP>)
    target_compile_options(${_TEST_NAME}_tests PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:$<IF:$<VERSION_GREATER_EQUAL:${MSVC_TOOLSET_VERSION},142>,/W4 /WX,>>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)
    add_test(NAME cache_${_TEST_NAME}_tests COMMAND ${_TEST_NAME}_tests)
endmacro()

option(USE_CUSTOM_HASHMAP "Use custom hashmap implementation for tests" OFF)

if (USE_CUSTOM_HASHMAP)
    include(FetchContent)
    FetchContent_Declare(
            parallel-hashmap
            GIT_REPOSITORY https://github.com/greg7mdp/parallel-hashmap.git
            GIT_TAG v2.0.0 # adjust tag/branch/commit as needed
    )
    FetchContent_MakeAvailable(parallel-hashmap)
endif ()

add_cache_test(general)
add_cache_test(fifo)
add_cache_test(lfu)
add_cache_test(lru)
add_cache_test(no_eviction)
