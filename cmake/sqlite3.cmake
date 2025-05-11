# cmake/sqlite3.cmake

# Determine the base directory of the sqlite3 source
get_filename_component(SQLITE3_ROOT "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(SQLITE3_DIR "${SQLITE3_ROOT}/../3rdParty/soci/3rdParty/sqlite3")

# Define the sqlite3 static library
add_library(sqlite3 STATIC
        "${SQLITE3_DIR}/sqlite3.c"
)

target_include_directories(sqlite3 PUBLIC
        "${SQLITE3_DIR}"
)

set_target_properties(sqlite3 PROPERTIES POSITION_INDEPENDENT_CODE ON)
