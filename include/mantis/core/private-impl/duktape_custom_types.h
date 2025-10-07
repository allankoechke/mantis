//
// Created by allan on 07/10/2025.
//

#ifndef MANTISAPP_DUKTAPE_CUSTOM_TYPES_H
#define MANTISAPP_DUKTAPE_CUSTOM_TYPES_H

#include <dukglue/dukglue.h>

namespace mantis
{
    class DuktapeImpl
    {
    public:
        static duk_ret_t nativeConsoleInfo(duk_context* ctx);

        static duk_ret_t nativeConsoleTrace(duk_context* ctx);

        static duk_ret_t nativeConsoleTable(duk_context* ctx);
    };
} // mantis

#endif //MANTISAPP_DUKTAPE_CUSTOM_TYPES_H