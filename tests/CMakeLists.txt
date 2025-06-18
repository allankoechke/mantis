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
        unit/test_database.cpp
        unit/test_jwt.cpp
        unit/test_models.cpp
        # unit/test_validators.cpp
        integration/test_api_endpoints.cpp
        integration/test_auth_flow.cpp
        integration/test_access_permissions.cpp
)

# Link libraries
target_link_libraries(mantis_tests
        PRIVATE
        mantis  # Link to main library
        GTest::gtest_main
        GTest::gmock_main
)

# Include directories
target_include_directories(mantis_tests
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(mantis_tests)