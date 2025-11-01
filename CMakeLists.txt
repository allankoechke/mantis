cmake_minimum_required(VERSION 3.22)
project(mantisapp VERSION 0.2.11)
set(CMAKE_CXX_STANDARD 20)

# Set output directories to match SOCI's
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Mantis Admin Dashboard Fetch Flags
# Mantis Admin dashboard version to embed
set(MANTIS_ADMIN_VERSION v0.1.7)
set(MANTIS_ADMIN_HASH 184fe54ab4d003de49cc75565258fafe098486b8a177fb8f9405398015e16081)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mantis/app/config.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mantis/app/config.hpp
)

# Flag for building deps as shared/static libs
set(MANTIS_SHARED_DEPS FALSE)
if(MANTIS_SHARED_DEPS)
    set(BUILD_SHARED_LIBS TRUE)
    set(MANTIS_LIB_TYPE SHARED)
    message("-- Building Mantis dynamic libs")
else()
    set(BUILD_SHARED_LIBS FALSE)
    set(MANTIS_LIB_TYPE STATIC)

    # For static builds, embed libstdgcc and libstdc++ to avoid binary loading fails
    # due to mismatch libstd* libs.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    message("-- Building Mantis static libs")
endif()

# Add main executable
add_executable(mantisapp
    src/mantisapp.cpp
)

# Add library
add_library(mantis
    ${MANTIS_LIB_TYPE}
    src/app/app.cpp
    src/core/database.cpp
    src/core/models/models.cpp
    src/core/logging.cpp
    src/core/router.cpp
    src/core/http.cpp
    src/core/jwt.cpp

    # All table operations
    src/core/tables/tables.cpp
    src/core/tables/tables_crud.cpp
    src/core/tables/tables_routes.cpp
    src/core/tables/tables_auth.cpp
    src/core/tables/tables_utils.cpp
    src/core/tables/tables_validation.cpp

    # System Tables
    src/core/tables/sys_tables.cpp
    src/core/tables/sys_tables_crud.cpp

    # Expression evaluator
    src/core/expr_evaluator.cpp

    # Utilities
    src/utils/auth_utils.cpp
    src/utils/string_utils.cpp
    src/utils/path_utils.cpp
    src/utils/type_utils.cpp
    src/utils/dukglue_utils_bindings.cpp

    include/mantis/core/private-impl/soci_custom_types.hpp
    include/mantis/app/config.hpp.in

    src/core/settings.cpp
    src/core/fileunit.cpp
    src/core/context_store.cpp

    src/core/private-impl/duktape_custom_types.cpp
    src/core/private-impl/duktape_request_wrapper.cpp
    src/core/private-impl/duktape_response_wrapper.cpp
)

# Add httplib
include(cmake/add-httplib.cmake)

# Add spdlog
if(MANTIS_SHARED_DEPS)
    # Build Shared lib for spdlog
    set(SPDLOG_BUILD_SHARED ON)
else()
    # Build static lib for spdlog
    set(SPDLOG_BUILD_SHARED OFF)
endif()

# Add SPDLOG subdir
add_subdirectory(3rdParty/spdlog)

# Add JSON
include(cmake/add-json.cmake)

# Add SOCI
include(cmake/add-soci.cmake)

# Add the cparse module for expr evaluation
include(cmake/cparse.cmake)

# Add wolfssl library
include(cmake/add-wolfssl.cmake)

# Add bcrypt to the project
include(cmake/add-bcrypt.cmake)

# Add CMRC for bundling assets
include(cmake/add-cmrc.cmake)

# Include address sanitizer for Linux debug/releaseWithDebug builds
include(cmake/add-asan.cmake)

# Include duktape for ECMASCRIPT scripting in Mantis
include(cmake/add-duktape.cmake)

# Link to libs
target_link_libraries(mantis
    PUBLIC
    httplib::httplib
    spdlog::spdlog
    cparse
)

# Include directories
target_include_directories(mantis
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/httplib-cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/argparse/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/spdlog/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/jwt-cpp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/cparse
    ${CMAKE_BINARY_DIR}/include
)

target_link_libraries(mantisapp
    PRIVATE mantis
)

# Check if Doxygen is installed, then build docs
include(cmake/add-doxygen.cmake)

# Add SEMVER
include(cmake/add-semver.cmake)

# Add Tests
option(MANTIS_BUILD_TESTS "Build Mantis Tests" OFF)
include(cmake/add-tests.cmake)