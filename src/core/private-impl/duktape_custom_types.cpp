//
// Created by allan on 07/10/2025.
//

#include <mantis/core/private-impl/duktape_custom_types.h>
#include <iostream>

namespace mantis
{
    static duk_ret_t native_console_info(duk_context* ctx)
    {
        duk_idx_t n = duk_get_top(ctx);
        std::cout << "[INFO] ";
        for (duk_idx_t i = 0; i < n; i++) {
            if (i > 0) std::cout << " ";
            std::cout << duk_safe_to_string(ctx, i);
        }
        std::cout << std::endl;
        return 0;
    }

    static duk_ret_t native_console_trace(duk_context* ctx)
    {
        duk_idx_t n = duk_get_top(ctx);
        std::cout << "[TRACE] ";
        for (duk_idx_t i = 0; i < n; i++) {
            if (i > 0) std::cout << " ";
            std::cout << duk_safe_to_string(ctx, i);
        }
        std::cout << std::endl;
        return 0;
    }

    static duk_ret_t native_console_table(duk_context* ctx)
    {
        duk_idx_t n = duk_get_top(ctx);
        std::cout << "[TABLE] ";
        for (duk_idx_t i = 0; i < n; i++) {
            if (i > 0) std::cout << " ";
            std::cout << duk_safe_to_string(ctx, i);
        }
        std::cout << std::endl;
        return 0;
    }
} // mantis