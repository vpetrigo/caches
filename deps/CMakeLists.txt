if(${BUILD_TEST})
  add_subdirectory(googletest)
  
  set(GTEST_FOUND TRUE CACHE INTERNAL "Set if libgtest was found or built")
  set(GTEST_LIBRARIES gtest CACHE INTERNAL "Location of libgtest")
  set(GTEST_MAIN_LIBRARIES gtest_main CACHE INTERNAL "Location of libgtest-main")
  set(GTEST_BOTH_LIBRARIES gtest gtest_main CACHE INTERNAL "Location of both gtest libraries")
  set(GTEST_INCLUDE_DIRS ${gtest_SOURCE_DIR}/include CACHE INTERNAL "Location of gtest include files")
endif()