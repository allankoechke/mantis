# Compiler flags based on existing Makefile
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wmissing-field-initializers -Wuninitialized -Wsign-compare")

# Core source files
set(CORE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/cparse/shunting-yard.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/cparse/packToken.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/cparse/functions.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/cparse/containers.cpp
)

# Built-in features
set(BUILTIN_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/cparse/builtin-features.cpp
)

# Create static library
add_library(cparse STATIC ${CORE_SOURCES} ${BUILTIN_SOURCES})

# Include directories
target_include_directories(cparse PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/cparse
)

# Platform-specific optimizations
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(cparse PRIVATE -O1)
endif()

# Optional: Build test executable
#option(BUILD_TESTS "Build test executable" OFF)
#if(BUILD_TESTS)
#    add_executable(test-shunting-yard
#            ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/cparse/test-shunting-yard.cpp
#            ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/cparse/catch.cpp
#            ${CORE_SOURCES}
#            ${BUILTIN_SOURCES}
#    )
#
#    target_compile_definitions(test-shunting-yard PRIVATE DEBUG)
#    target_compile_options(test-shunting-yard PRIVATE -g)
#endif()

## Installation
#install(TARGETS cparse
#        ARCHIVE DESTINATION lib
#        LIBRARY DESTINATION lib
#        RUNTIME DESTINATION bin
#)
#
## Install headers
#file(GLOB HEADERS "*.h")
#install(FILES ${HEADERS} DESTINATION include/cparse)