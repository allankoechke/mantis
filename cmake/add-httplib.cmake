# Build static lib for httplib
set(HTTPLIB_COMPILE OFF CACHE BOOL "")
if(WIN32)
    set(HTTPLIB_USE_NON_BLOCKING_GETADDRINFO OFF)
else (WIN32)
    set(HTTPLIB_REQUIRE_ZSTD TRUE)
    include(cmake/FindZSTD.cmake)
    find_package(zstd REQUIRED)

    if(zstd_FOUND)
        add_library(zstd::libzstd ALIAS ZSTD::ZSTD)
    endif()
endif()

# Disable SSL use for now, throws errors with WolfSSL library version
set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)

add_subdirectory(3rdParty/httplib-cpp)