# Build static lib for httplib
set(HTTPLIB_COMPILE OFF CACHE BOOL "")
if(NOT WIN32)
    set(HTTPLIB_REQUIRE_ZSTD TRUE)
    include(cmake/FindZSTD.cmake)
    find_package(zstd REQUIRED)

    if(zstd_FOUND)
        add_library(zstd::libzstd ALIAS ZSTD::ZSTD)
    endif()
endif()