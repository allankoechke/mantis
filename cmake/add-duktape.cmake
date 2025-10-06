
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/duktape)

# Link to libs
target_link_libraries(mantis
        PUBLIC
        duktape
)

# Include directories
target_include_directories(mantis
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/duktape
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/dukglue/include
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/dukglue/include)