
if(MANTIS_SHARED_DEPS)
    # Enable multi-header mode (compiled lib)
    set(JSON_MultipleHeaders ON CACHE BOOL "Enable compiled mode for nlohmann/json")
else ()
    # Enable multi-header mode (compiled lib)
    set(JSON_MultipleHeaders OFF CACHE BOOL "Disable compiled mode for nlohmann/json")
endif ()

set(JSON_Install OFF CACHE INTERNAL "")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/json)

# Link to libs
target_link_libraries(mantis
        PUBLIC
        nlohmann_json::nlohmann_json
)

# Include directories
target_include_directories(mantis
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/json/single_include
)