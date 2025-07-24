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

# Create test executable
add_executable(mantis_tests
        main.cpp
        test_environment.h
        unit/test_database.cpp
        unit/test_jwt.cpp
        unit/test_models.cpp
        integration/test_api_endpoints.cpp
        integration/test_auth_flow.cpp
        integration/test_access_permissions_base.cpp
        integration/test_crud_add_permissions.cpp
        integration/test_crud_get_permissions.cpp
        integration/test_crud_delete_permissions.cpp
        integration/test_crud_list_permissions.cpp
        integration/test_crud_update_permissions.cpp
)

# Link libraries
target_link_libraries(mantis_tests
        PRIVATE
        mantis
        GTest::gtest
        GTest::gtest_main
        GTest::gmock_main
)
set(ENABLE_ASAN ON)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/asan.cmake)

# Include directories
target_include_directories(mantis_tests
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(mantis_tests PRIVATE
            -Wa,-mbig-obj  # Tell assembler to handle large objects
    )
endif()

enable_asan_for_target(mantis_tests)

# Register tests with CTest
include(GoogleTest)
add_test(NAME all_mantis_tests COMMAND mantis_tests)