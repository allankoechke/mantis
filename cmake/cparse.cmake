# Compiler flags based on existing Makefile
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wmissing-field-initializers -Wuninitialized -Wsign-compare")

# Core source files
set(CORE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/cparse/shunting-yard.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/cparse/packToken.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/cparse/functions.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/cparse/containers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/cparse/builtin-features.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/cparse/builtin_features.h
)

# Create static library
add_library(cparse STATIC ${CORE_SOURCES})

# Include directories
target_include_directories(cparse PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/cparse
)

# Platform-specific optimizations
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(cparse PRIVATE -O1)
endif()