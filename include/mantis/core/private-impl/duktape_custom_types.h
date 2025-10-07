//
// Created by allan on 07/10/2025.
//

#ifndef MANTISAPP_DUKTAPE_CUSTOM_TYPES_H
#define MANTISAPP_DUKTAPE_CUSTOM_TYPES_H

#include <dukglue/dukglue.h>

namespace mantis
{
    static duk_ret_t native_console_info(duk_context* ctx);

    static duk_ret_t native_console_trace(duk_context* ctx);

    static duk_ret_t native_console_table(duk_context* ctx);
} // mantis

#endif //MANTISAPP_DUKTAPE_CUSTOM_TYPES_H