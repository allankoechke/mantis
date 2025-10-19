cmake_minimum_required(VERSION 3.22)
project(mantisapp VERSION 0.2.9)

set(CMAKE_CXX_STANDARD 20)

# Set output directories to match SOCI's
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5) # Chilbuff still on CMake 3.1

option(MANTIS_BUILD_TESTS "Build tests" ON)

# Mantis Admin Dashboard Fetch Flags
# Mantis Admin dashboard version to embed
set(MANTIS_ADMIN_VERSION v0.1.7)
set(MANTIS_ADMIN_HASH 184fe54ab4d003de49cc75565258fafe098486b8a177fb8f9405398015e16081)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mantis/app/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/mantis/app/config.hpp
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
add_library(mantis ${MANTIS_LIB_TYPE}
    include/mantis/mantis.h
    src/app/app.cpp
    include/mantis/app/app.h
    src/core/database.cpp
    include/mantis/core/database.h
    src/core/models/models.cpp
    include/mantis/core/models/models.h
    src/core/logging.cpp
    include/mantis/core/logging.h
    src/core/router.cpp
    include/mantis/core/router.h
    src/core/http.cpp
    include/mantis/core/http.h
    include/mantis/core/crud/crud.h
    include/mantis/core/jwt.h
    src/core/jwt.cpp

    # All table operations
    include/mantis/core/tables/tables.h
    src/core/tables/tables.cpp
    src/core/tables/tables_crud.cpp
    src/core/tables/tables_routes.cpp
    src/core/tables/tables_auth.cpp
    src/core/tables/tables_utils.cpp
    src/core/tables/tables_validation.cpp

    # System Tables
    include/mantis/core/tables/sys_tables.h
    src/core/tables/sys_tables.cpp
    src/core/tables/sys_tables_crud.cpp

    # Expression evaluator
    src/core/expr_evaluator.cpp
    include/mantis/core/expr_evaluator.h

    # Utilities
    include/mantis/utils/utils.h
    src/utils/auth_utils.cpp
    src/utils/string_utils.cpp
    src/utils/path_utils.cpp
    src/utils/type_utils.cpp
    src/utils/dukglue_utils_bindings.cpp

    include/mantis/core/private-impl/soci_custom_types.hpp
    include/mantis/app/config.hpp.in

    src/core/settings.cpp
    include/mantis/core/settings.h

    src/core/fileunit.cpp
    include/mantis/core/fileunit.h

    include/mantis/core/private-impl/duktape_custom_types.h
    src/core/private-impl/duktape_custom_types.cpp
    src/core/private-impl/duktape_request_wrapper.cpp
    src/core/private-impl/duktape_response_wrapper.cpp

    include/mantis/core/context_store.h
    src/core/context_store.cpp
)

# Add httplib
include(cmake/add-httplib.cmake)

# Disable
if(WIN32)
    set(HTTPLIB_USE_NON_BLOCKING_GETADDRINFO OFF)
endif(WIN32)
add_subdirectory(3rdParty/httplib-cpp)

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

add_subdirectory(3rdParty/l8w8jwt)

# Add the cparse module for expr evaluation
include(cmake/cparse.cmake)

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
    l8w8jwt
    cparse
)

# Include directories
target_include_directories(mantis
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/httplib-cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/argparse/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/spdlog/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/l8w8jwt/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/l8w8jwt/lib/mbedtls/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/cparse
    ${CMAKE_BINARY_DIR}/include
)

target_link_libraries(mantisapp
    PRIVATE mantis
)

# In your main CMakeLists.txt, after add_subdirectory(3rdParty/l8w8jwt)
if(WIN32)
    target_link_libraries(mantis PRIVATE bcrypt)
endif()

# Check if Doxygen is installed, then build docs
include(cmake/add-doxygen.cmake)

# Add SEMVER
include(cmake/add-semver.cmake)

# Add Tests
include(cmake/add-tests.cmake)