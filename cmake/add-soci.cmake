# For configuration help for SOCI,
# check https://github.com/allankoechke/soci/blob/master/docs/installation.md

# Critical: Set SOCI_SHARED before adding subdirectory
set ( SOCI_SHARED OFF CACHE BOOL "Build SOCI as static library" FORCE )

# Disable all backends except SQLite
set ( SOCI_TESTS OFF CACHE BOOL "Disable SOCI tests" FORCE )
set ( WITH_BOOST OFF CACHE BOOL "Disable Boost dependency" FORCE )
set ( SOCI_SQLITE3 ON CACHE BOOL "Enable SQLite3 backend" FORCE )
set ( SOCI_SQLITE3_BUILTIN ON CACHE BOOL "Use builtin SQLite3" FORCE )

# Explicitly disable other backends
set ( SOCI_MYSQL OFF CACHE BOOL "Disable MySQL backend" FORCE )
set ( SOCI_POSTGRESQL OFF CACHE BOOL "Disable PostgreSQL backend" FORCE )
set ( SOCI_ORACLE OFF CACHE BOOL "Disable Oracle backend" FORCE )
set ( SOCI_ODBC OFF CACHE BOOL "Disable ODBC backend" FORCE )
set ( SOCI_DB2 OFF CACHE BOOL "Disable DB2 backend" FORCE )
set ( SOCI_FIREBIRD OFF CACHE BOOL "Disable Firebird backend" FORCE )
set ( SOCI_EMPTY OFF CACHE BOOL "Disable empty backend" FORCE )

# Add SOCI subdirectory - this should generate soci-config.h
add_subdirectory ( ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/soci )


target_link_libraries ( mantis
        PRIVATE
        soci_core
        soci_sqlite3
        #        soci_empty
        #        soci_odbc
)

# Include directories
target_include_directories ( mantis
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/soci/3rdParty
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/soci/include
        ${CMAKE_BINARY_DIR}/include                 # Generated headers
        ${CMAKE_BINARY_DIR}/3rdParty/soci/include   # Same as above, just in case it ends up here
)
