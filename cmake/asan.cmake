
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

function(enable_asan_for_target target)
    message(STATUS "Enabling ASan for target ${target}")
    target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(${target} PRIVATE -fsanitize=address)
endfunction()