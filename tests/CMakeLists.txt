cmake_minimum_required(VERSION 3.14)

# Try to find GTest first
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    # If not found, fetch it using FetchContent
    include(FetchContent)

    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.14.0  # Use latest stable version
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Don't build gmock/gtest apps
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    # Create aliases to match find_package targets
    add_library(GTest::gtest ALIAS gtest)
    add_library(GTest::gtest_main ALIAS gtest_main)
    add_library(GTest::gmock ALIAS gmock)
    add_library(GTest::gmock_main ALIAS gmock_main)
endif()

file(GLOB TEST_FILES
        main.cpp
        unit/*.cpp
        integration/*.cpp
)

add_executable(mantisbase_tests ${TEST_FILES})
target_link_libraries(mantisbase_tests PRIVATE
        mantisbase_lib
        GTest::gtest
        GTest::gtest_main
        GTest::gmock_main
)

if(NOT WIN32) # For now, only enable ASAN for non-windows based builds
    message("-- Enabling Address Sanitizer")
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/asan.cmake)
    include(../cmake/asan.cmake)
    set(ENABLE_ASAN ON)

    enable_asan_for_target(mantisbase_tests)
endif()

# Include directories
target_include_directories(mantisbase_tests
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_compile_definitions(mantisbase_tests PRIVATE
        TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(mantisbase_tests PRIVATE
            -Wa,-mbig-obj  # Tell assembler to handle large objects
    )
endif()

# Register tests with CTest
include(GoogleTest)
add_test(NAME all_mantisbase_tests COMMAND mantisbase_tests)