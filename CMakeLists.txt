cmake_minimum_required(VERSION 3.28)

project(caches
        VERSION 0.2.0)

macro(add_coverage_flags target)
    if (CACHES_BUILD_TEST AND CACHES_ENABLE_COVERAGE)
        target_compile_options(${target} PRIVATE --coverage)

        if (NOT MSVC)
            target_link_options(${target} PRIVATE --coverage)
        endif ()
    endif ()
endmacro()

if (MSVC)
    add_definitions(/MP)
endif ()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CACHES_INSTALL_LIBRARY "Install caches library" OFF)
option(CACHES_BUILD_TEST "Build tests for the project" ON)
option(CACHES_ENABLE_COVERAGE "Build test executables with coverage support" OFF)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    configure_file(${PROJECT_SOURCE_DIR}/cmake/Doxygen.in
        ${PROJECT_SOURCE_DIR}/docs/doxygen/Doxyfile)
    configure_file(${PROJECT_SOURCE_DIR}/cmake/index.md.in
        ${PROJECT_SOURCE_DIR}/docs/doxygen/index.md)

    add_custom_target(docs doxygen Doxyfile
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/docs/doxygen
            COMMENT "Generate library Doxygen documentation")
endif ()

add_library(caches INTERFACE)
target_sources(caches INTERFACE
    FILE_SET caches_headers
    TYPE HEADERS
    BASE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include"
    FILES
    include/caches/core/cache.hpp
    include/caches/policies/fifo.hpp
    include/caches/policies/lfu.hpp
    include/caches/policies/lru.hpp
    include/caches/policies/no_eviction.hpp
    include/caches/policies/policy_interface.hpp
)

if (CACHES_BUILD_TEST)
    enable_testing()
    add_custom_target(coverage
            COMMAND gcovr
                -r ${CMAKE_CURRENT_LIST_DIR}
                --filter "${CMAKE_CURRENT_LIST_DIR}/include"
                -x coverage.info
    )
    add_custom_target(coverage-html
            COMMAND gcovr
                -r ${CMAKE_CURRENT_LIST_DIR}
                --filter "${CMAKE_CURRENT_LIST_DIR}/include"
                --html-details ${CMAKE_BINARY_DIR}/report/report.html
    )
endif ()

if (CACHES_INSTALL_LIBRARY)
    set(INSTALL_GTEST OFF CACHE STRING "")
endif ()

if (CACHES_BUILD_TEST)
    add_subdirectory(deps/)
    add_subdirectory(test)
endif ()

if (CACHES_INSTALL_LIBRARY)
    include(GNUInstallDirs)
    configure_file(${PROJECT_SOURCE_DIR}/cmake/pkg-config.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
    include(CMakePackageConfigHelpers)
    configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
            PATH_VARS CMAKE_INSTALL_LIBDIR)

    set(pc_file "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc")
    set(cmake_config_file "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake")

    install(
        TARGETS caches
        EXPORT caches_lib_targets
        FILE_SET caches_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(TARGETS caches EXPORT ${PROJECT_NAME}-targets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT ${PROJECT_NAME}-targets
        NAMESPACE ${PROJECT_NAME}:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME})
    install(FILES ${pc_file}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(FILES ${cmake_config_file}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME})
endif ()
