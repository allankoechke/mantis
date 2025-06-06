

# For configuration help for SOCI,
# check https://github.com/allankoechke/soci/blob/master/docs/installation.md
# For now, disable all backends except SQLite
# set(SOCI_SHARED OFF CACHE BOOL "OFF" FORCE )
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

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/soci)

# Link to libs
target_link_libraries(mantisapp
        PRIVATE
        soci_core
        soci_sqlite3
        #        soci_empty
        #        soci_odbc
)

# Include directories
target_include_directories(mantisapp
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/soci/3rdParty
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/soci/include
)
