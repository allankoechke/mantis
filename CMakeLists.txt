cmake_minimum_required(VERSION 3.30)
project(mantis-ctl)

set(CMAKE_CXX_STANDARD 20)

# Set output directories to match SOCI's
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_executable(mantis-ctl
        src/main.cpp
        include/mantis/mantis.h src/mantis.cpp
        include/mantis/core/database.h src/database.cpp
        include/mantis/core/models.h src/models.cpp
        include/mantis/utils.h include/mantis/core/logger.h
        include/mantis/api/api-utils.hpp src/logger.cpp
        include/mantis/api/servermgr.h src/servermgr.cpp
        include/mantis/api/httpserver.h. src/httpserver.cpp

        # Add AnyOption Source Files, no need of linking later
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/AnyOption/anyoption.h
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/AnyOption/anyoption.cpp
)

# For configuration help for SOCI,
# check https://github.com/allankoechke/soci/blob/master/docs/installation.md
# For now, disable all backends except SQLite
set(SOCI_SHARED OFF CACHE BOOL "OFF" FORCE )
set(SOCI_TESTS OFF CACHE BOOL "OFF" FORCE )
set(WITH_BOOST OFF CACHE BOOL "OFF" FORCE )
set(WITH_BOOST OFF CACHE BOOL "OFF" FORCE )
set(SOCI_SQLITE3 ON CACHE BOOL "OFF" FORCE )
set(SOCI_SQLITE3_BUILTIN ON CACHE BOOL "OFF" FORCE )
set(SOCI_MYSQL OFF CACHE BOOL "" )
set(SOCI_POSTGRESQL OFF CACHE BOOL "" )
set(SOCI_ORACLE OFF CACHE BOOL "" )
set(SOCI_ODBC OFF CACHE BOOL "" )
set(SOCI_DB2 OFF CACHE BOOL "" )
set(SOCI_FIREBIRD OFF CACHE BOOL "" )
set(SOCI_EMPTY OFF CACHE BOOL "" )
message(STATUS ${SOCI_SQLITE3_BUILTIN})
add_subdirectory(3rdParty/soci)

set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(3rdParty/json)

# Add httplib
set(HTTPLIB_COMPILE OFF CACHE BOOL "")
add_subdirectory(3rdParty/httplib-cpp)

# Add spdlog
set(SPDLOG_BUILD_SHARED OFF)
add_subdirectory(3rdParty/spdlog)

# Link to libs
target_link_libraries(mantis-ctl
    PRIVATE
        httplib::httplib
        nlohmann_json::nlohmann_json
        soci_core
        soci_sqlite3
        soci_empty
        soci_odbc
        spdlog::spdlog
)

# Include directories
target_include_directories(mantis-ctl
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/httplib-cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/soci/3rdParty
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/soci/include
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/AnyOption
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/json/single_include
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/spdlog/include
)

#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#    message(STATUS "Debug build")
#elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
#    message(STATUS "Release build")
#elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
#    message(STATUS "RelWithDebInfo build")
#elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
#    message(STATUS "MinSizeRel build")
#else()
#    message(WARNING "Unknown or unset CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
#endif()