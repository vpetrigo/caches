if(CACHES_BUILD_TEST)
    set(BUILD_SHARED_LIBS OFF)
    add_subdirectory(catch2/)
endif ()
