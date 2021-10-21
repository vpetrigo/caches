cmake_minimum_required(VERSION 3.0)

project(caches)

if(MSVC)
  add_definitions(/MP)
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(CACHE_INCLUDE_DIRS ${caches_SOURCE_DIR}/include CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD 11)

option(BUILD_TEST "Build tests for the project" ON)

add_library(caches INTERFACE)
target_include_directories(caches INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

if (BUILD_TEST)
  enable_testing()
endif()

add_subdirectory(deps/)

if (BUILD_TEST)
  add_subdirectory(test)
endif()
