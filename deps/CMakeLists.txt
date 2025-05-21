if(CACHES_BUILD_TEST)
  if (MINGW)
    set(gtest_disable_pthreads ON CACHE BOOL "Force gtest to disable pthreads")
  endif()

  if(MSVC)
    # By default, GTest compiles on Windows in CRT static linkage mode. We use this
    # variable to force it into using the CRT in dynamic linkage (DLL), just as CPR
    # does.
    set(gtest_force_shared_crt ON CACHE BOOL "Force gtest to use the shared c runtime")
  endif()

  add_subdirectory(googletest)

  set(GTEST_FOUND TRUE CACHE INTERNAL "Set if libgtest was found or built")
  set(GTEST_LIBRARIES gtest CACHE INTERNAL "Location of libgtest")
  set(GTEST_MAIN_LIBRARIES gtest_main CACHE INTERNAL "Location of libgtest-main")
  set(GTEST_BOTH_LIBRARIES gtest gtest_main CACHE INTERNAL "Location of both gtest libraries")
  set(GTEST_INCLUDE_DIRS ${gtest_SOURCE_DIR}/include CACHE INTERNAL "Location of gtest include files")
endif()
