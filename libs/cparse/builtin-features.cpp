
/**
 * Force the Startup classes to run
 * at static initialization time:
 */

#include "./builtin-features.inc"
#include "builtin_features.h"
#include <mutex>

namespace cparse
{
    std::once_flag init_flag;

    void cparse_init()
    {
        // Execute only once
        std::call_once(init_flag, []() {
            cparse_startup();
        });
    }
}
