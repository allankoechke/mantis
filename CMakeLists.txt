cmake_minimum_required(VERSION 3.30)
project(mantis-ctl)

set(CMAKE_CXX_STANDARD 20)

add_executable(mantis-ctl
        src/main.cpp
        include/mantis/mantis.h src/mantis.cpp
        include/mantis/core/database.h src/database.cpp
        include/mantis/core/models.h
)

include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)
option(mantis-ctl_ENABLE_TESTS "enable testing" OFF)

include(CTest)
enable_testing()

CPMAddPackage(Catch2
  VERSION 3.7.0
  GITHUB_REPOSITORY catchorg/Catch2
  OPTIONS
  "CATCH_INSTALL_DOCS Off"
  "CATCH_INSTALL_EXTRAS Off"
  "CATCH_ENABLE_REPRODUCIBLE_BUILD Off"
)

# Add SQLiteCpp
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(SQLITECPP_USE_STATIC_RUNTIME ON CACHE BOOL "Link to static SQlite")
add_subdirectory(3rdParty/SQLiteCpp)


# Add httplib
set(HTTPLIB_COMPILE OFF CACHE BOOL "")
add_subdirectory(3rdParty/httplib-cpp)

# Add AnyOption
# set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# add_subdirectory(3rdParty/AnyOption)

add_library(anyoption_static STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/AnyOption/anyoption.h
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/AnyOption/anyoption.cpp
)

target_include_directories(anyoption_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/AnyOption>
)

set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(3rdParty/json)

target_link_libraries(mantis-ctl
    PRIVATE
        SQLiteCpp
        httplib::httplib
        anyoption_static
        nlohmann_json::nlohmann_json
)

# Include directories
target_include_directories(mantis-ctl
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/httplib-cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/SQLiteCpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/SQLiteCpp/sqlite3      # Make
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/AnyOption
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/json/include
)
