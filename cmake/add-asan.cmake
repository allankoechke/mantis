if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if(NOT WIN32) # For now, only enable ASAN for non-windows based builds
        message("Enabling Address Sanitizer")
        include(cmake/asan.cmake)
        set(ENABLE_ASAN ON)

        enable_asan_for_target(mantis)
        enable_asan_for_target(mantisapp)
    endif()
endif()