cmake_minimum_required(VERSION 3.14)

include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.6.0 # pick a stable version
)
FetchContent_MakeAvailable(Catch2)

include(CTest)
include(Catch)
enable_testing()

file(GLOB TEST_FILES
        main.cpp
        unit/*.cpp
        integration/*.cpp
)

add_executable(mantisapp_tests ${TEST_FILES})
target_link_libraries(mantisapp_tests PRIVATE
        mantis
        Catch2::Catch2WithMain
)

if(NOT WIN32) # For now, only enable ASAN for non-windows based builds
    message("-- Enabling Address Sanitizer")
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/asan.cmake)
    include(../cmake/asan.cmake)
    set(ENABLE_ASAN ON)

    enable_asan_for_target(mantisapp_tests)
endif()

# Include directories
target_include_directories(mantisapp_tests
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_compile_definitions(mantisapp_tests PRIVATE
        TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(mantisapp_tests PRIVATE
            -Wa,-mbig-obj  # Tell assembler to handle large objects
    )
endif()

catch_discover_tests(mantisapp_tests)