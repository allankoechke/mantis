cmake_minimum_required(VERSION 3.22)
project(mantisapp VERSION 0.1.13)

set(CMAKE_CXX_STANDARD 20)

# Set output directories to match SOCI's
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Mantis Admin Dashboard Fetch Flags
# Mantis Admin dashboard version to embed
set(MANTIS_ADMIN_VERSION v0.1.6)
set(MANTIS_ADMIN_HASH 39a78d95a49f768ebb6f72453cc3c2726ce2c6f602669105e833920644f38f7a)

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

    include/mantis/core/private-impl/soci_custom_types.hpp
    include/mantis/app/config.hpp.in

    src/core/settings.cpp
    include/mantis/core/settings.h
)

# Add httplib
# Build static lib for httplib
set(HTTPLIB_COMPILE OFF CACHE BOOL "")
if(NOT WIN32)
    set(HTTPLIB_REQUIRE_ZSTD TRUE)
    include(cmake/FindZSTD.cmake)
    find_package(zstd REQUIRED)
    if(zstd_FOUND)
    add_library(zstd::libzstd ALIAS ZSTD::ZSTD)
    endif()
endif()

add_subdirectory(3rdParty/httplib-cpp)

# Add spdlog
if(MANTIS_SHARED_DEPS)
    # Build Shared lib for spdlog
    set(SPDLOG_BUILD_SHARED ON)
else()
    # Build static lib for spdlog
    set(SPDLOG_BUILD_SHARED OFF)
endif()

add_subdirectory(3rdParty/spdlog)

# Add JSON, SOCI
include(cmake/add-json.cmake)
include(cmake/add-soci.cmake)

add_subdirectory(3rdParty/l8w8jwt)

# Add the cparse module for expr evaluation
include(cmake/cparse.cmake)

# Add bcrypt to the project
include(cmake/add-bcrypt.cmake)

# Add CMRC for bundling assets
include(cmake/add-cmrc.cmake)

# Add to CMakeLists.txt
option(MANTIS_BUILD_TESTS "Build tests" OFF)

if(MANTIS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

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

# Inject semantic versioning
# Try to get the current Git commit hash
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

# Suffix for pre-release or metadata (e.g., "-dev", "+gabcdef")
set(MANTIS_VERSION_SUFFIX "-dev+g${GIT_COMMIT_HASH}")
set(MANTIS_GIT_COMMIT "${GIT_COMMIT_HASH}")

# Fallback if not in a Git repo
if(NOT GIT_COMMIT_HASH)
    set(MANTIS_VERSION_SUFFIX "")
    set(MANTIS_GIT_COMMIT "unknown")
endif()

# Add docs option, only if Doxygen is installed
find_package(Doxygen)
if(DOXYGEN_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        doxygen-awesome-css
        URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(doxygen-awesome-css)

    # Save the location the files were cloned into
    # This allows us to get the path to doxygen-awesome.css
    FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

    # Generate the Doxyfile
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    set(DOXY_MAINPAGE ${CMAKE_SOURCE_DIR}/doc/QuickStart.md)
    set(DOXYGEN_LOGO ${CMAKE_SOURCE_DIR}/assets/mantis.png)

    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )

    add_custom_command(TARGET doc POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_CURRENT_BINARY_DIR}/docs/html/assets
    )
else()
    message("Doxygen not installed, skipping building docs")
endif()
