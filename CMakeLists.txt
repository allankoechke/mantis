cmake_minimum_required(VERSION 3.30)
project(mantisapp)

set(CMAKE_CXX_STANDARD 20)

# Flag for building deps as shared/static libs
option(MANTIS_SHARED_DEPS "Build mantis dependencies as shared libs" ON)
if(MANTIS_SHARED_DEPS)
    # Optional: choose static/shared globally
    set(BUILD_SHARED_LIBS ON)
else ()
    # Optional: choose static/shared globally
    set(BUILD_SHARED_LIBS OFF)
endif ()

# Set output directories to match SOCI's
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_executable(mantisapp
        src/main.cpp
        include/mantis/mantis.h
        src/app/app.cpp
        include/mantis/app/app.h
        src/core/database.cpp
        include/mantis/core/database.h
        src/core/models/models.cpp
        include/mantis/core/models/models.h
        include/mantis/utils.h
        src/core/logging.cpp
        include/mantis/core/logging.h
        src/core/router.cpp
        include/mantis/core/router.h
        src/core/http.cpp
        include/mantis/core/http.h
        include/mantis/core/crud/crud.h
        include/mantis/core/jwtprovider.h

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

        src/core/expr_evaluator.cpp
        include/mantis/core/expr_evaluator.h
)

# Add httplib
if(MANTIS_SHARED_DEPS)
    # Build Shared lib for httplib
    set(HTTPLIB_COMPILE ON CACHE BOOL "")
else ()
    # Build static lib for httplib
    set(HTTPLIB_COMPILE OFF CACHE BOOL "")
endif ()
add_subdirectory(3rdParty/httplib-cpp)

# Add spdlog
if(MANTIS_SHARED_DEPS)
    # Build Shared lib for spdlog
    set(SPDLOG_BUILD_SHARED ON)
else ()
    # Build static lib for spdlog
    set(SPDLOG_BUILD_SHARED OFF)
endif ()

add_subdirectory(3rdParty/spdlog)

# Add JSON, SOCI
include(cmake/add-json.cmake)
include(cmake/add-soci.cmake)

# Add cpp-jwt library
#set(CPP_JWT_BUILD_EXAMPLES OFF)
#set(CPP_JWT_BUILD_TESTS OFF)
add_subdirectory(3rdParty/l8w8jwt)

# Add the cparse module for expr evaluation
include(cmake/cparse.cmake)

# Link to libs
target_link_libraries(mantisapp
    PRIVATE
        httplib::httplib
        spdlog::spdlog
        l8w8jwt
        cparse
)

# Include directories
target_include_directories(mantisapp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/httplib-cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/argparse/include
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/spdlog/include
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/l8w8jwt/include
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/cparse
)