macro(add_cache_test _TEST_NAME)
    add_executable(${_TEST_NAME}_tests
        ${_TEST_NAME}_tests.cpp)
    target_link_libraries(${_TEST_NAME}_tests
        ${GTEST_MAIN_LIBRARIES}
    )
    add_test(NAME cache_${_TEST_NAME}_tests COMMAND ${_TEST_NAME}_tests)
endmacro()

include_directories(
  ${GTEST_INCLUDE_DIRS},
  ${CACHE_INCLUDE_DIRS}
)

add_cache_test(lru_cache)
add_cache_test(fifo_cache)