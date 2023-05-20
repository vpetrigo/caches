macro(add_cache_test _TEST_NAME)
    add_executable(${_TEST_NAME}_tests
            ${_TEST_NAME}_tests.cpp)
    target_link_libraries(${_TEST_NAME}_tests
            ${GTEST_MAIN_LIBRARIES} caches)
    target_include_directories(${_TEST_NAME}_tests PRIVATE ${GTEST_INCLUDE_DIRS})
    target_compile_options(${_TEST_NAME}_tests PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)
    add_test(NAME cache_${_TEST_NAME}_tests COMMAND ${_TEST_NAME}_tests)
endmacro()

add_cache_test(lru_cache)
add_cache_test(fifo_cache)
add_cache_test(lfu_cache)
add_cache_test(nopolicy_cache)
