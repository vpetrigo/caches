cmake_minimum_required(VERSION 3.0)

project(caches
    VERSION 0.0.2)

if(MSVC)
  add_definitions(/MP)
endif()

set(CMAKE_CXX_STANDARD 11)

configure_file(${PROJECT_SOURCE_DIR}/cmake/pkg-config.pc.in ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc)
include(CMakePackageConfigHelpers)
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
    PATH_VARS CMAKE_INSTALL_LIBDIR)
option(BUILD_TEST "Build tests for the project" ON)

add_library(caches INTERFACE)
target_include_directories(caches INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

if (BUILD_TEST)
  enable_testing()
endif()
set(INSTALL_GTEST OFF CACHE STRING "")
add_subdirectory(deps/)

if (BUILD_TEST)
  add_subdirectory(test)
endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ TYPE INCLUDE)
install(TARGETS caches EXPORT ${PROJECT_NAME}-targets INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ${PROJECT_NAME}-targets NAMESPACE ${PROJECT_NAME}:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME})
