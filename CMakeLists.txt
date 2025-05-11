cmake_minimum_required(VERSION 3.30)
project(mantis-ctl)

set(CMAKE_CXX_STANDARD 20)

add_executable(mantis-ctl
        src/main.cpp
        include/mantis/mantis.h src/mantis.cpp
        include/mantis/core/database.h src/database.cpp
        include/mantis/core/models.h src/models.cpp
        include/mantis/utils.h

        # Add AnyOption Source Files, no need of linking later
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/AnyOption/anyoption.h
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/AnyOption/anyoption.cpp
)

include(${CMAKE_SOURCE_DIR}/cmake/sqlite3.cmake)
#
# src/models.cpp

#include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)
#option(mantis-ctl_ENABLE_TESTS "enable testing" OFF)

#include(CTest)
#enable_testing()

#CPMAddPackage(Catch2
#  VERSION 3.7.0
#  GITHUB_REPOSITORY catchorg/Catch2
#  OPTIONS
#  "CATCH_INSTALL_DOCS Off"
#  "CATCH_INSTALL_EXTRAS Off"
#  "CATCH_ENABLE_REPRODUCIBLE_BUILD Off"
#)

# For configuration help for SOCI,
# check https://github.com/allankoechke/soci/blob/master/docs/installation.md
set(SOCI_SHARED OFF CACHE BOOL "Build static soci" )
set(SOCI_TESTS OFF CACHE BOOL "Disable building soci tests" )
set(WITH_BOOST OFF CACHE BOOL "Find boost files for soci" )
set(WITH_BOOST OFF CACHE BOOL "Dont find boost files for soci" )
set(SOCI_SQLITE3 ON CACHE BOOL "Enable SQLite3 Backend for soci" )
set(SOCI_SQLITE3_BUILTIN ON CACHE BOOL "Use builtin SQLite library" )
message(STATUS ${SOCI_SQLITE3_BUILTIN})
add_subdirectory(3rdParty/soci)

set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(3rdParty/json)

# Add httplib
set(HTTPLIB_COMPILE OFF CACHE BOOL "")
add_subdirectory(3rdParty/httplib-cpp)

target_link_libraries(mantis-ctl
    PRIVATE
        httplib::httplib
        nlohmann_json::nlohmann_json
        SOCI::soci
        sqlite3
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