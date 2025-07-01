# Compiler flags from the original Makefile
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wbad-function-cast -Wcast-align -Wcast-qual -Wmissing-prototypes -Wstrict-prototypes -Wshadow -Wundef -Wpointer-arith -O2 -fomit-frame-pointer -funroll-loops")

# Source files based on CRYPT_OBJS from crypt_blowfish/Makefile
set(CRYPT_BLOWFISH_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libbcrypt/crypt_blowfish/crypt_blowfish.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libbcrypt/crypt_blowfish/x86.S
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libbcrypt/crypt_blowfish/crypt_gensalt.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libbcrypt/crypt_blowfish/wrapper.c
)

# Main bcrypt wrapper
set(BCRYPT_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libbcrypt/libbcrypt.c
)

# Create static library
add_library(libbcrypt STATIC
        ${BCRYPT_SOURCES}
        ${CRYPT_BLOWFISH_SOURCES}
)

# Include directories
target_include_directories(libbcrypt PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libbcrypt
)

# Handle assembly file compilation
enable_language(ASM)
set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/libs/libbcrypt/crypt_blowfish/x86.S PROPERTY LANGUAGE ASM)

target_link_libraries ( mantis PUBLIC libbcrypt )