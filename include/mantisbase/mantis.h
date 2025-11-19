//
// Created by allan on 17/05/2025.
//

#ifndef MANTIS_H
#define MANTIS_H

// Utility functions
#include "utils/utils.h"
#include "utils/uuidv7.h"
#include "utils/soci_wrappers.h"

// Core application
#include "mantisbase.h"

// Core components
#include "core/context_store_mgr.h"
#include "core/database_mgr.h"
#include "core/exceptions.h"
#include "core/expr_evaluator.h"
#include "core/files_mgr.h"
#include "core/http.h"
#include "core/jwt_mgr.h"
#include "core/logs_mgr.h"
#include "core/middlewares.h"
#include "core/route_registry.h"
#include "core/router.h"
#include "core/settings_mgr.h"
#include "core/types.h"

// Models and data structures
#include "core/models/validators.h"
#include "core/models/entity.h"
#include "core/models/entity_schema.h"
#include "core/models/entity_schema_field.h"

// For convenience to using json,
// lets include it here
#include <nlohmann/json.hpp>
namespace mantis {
    using json = nlohmann::json;
}

// For argparse lib
#include <argparse/argparse.hpp>

// Add soci include
#include <soci/soci.h>

#include <httplib.h>

#endif //MANTIS_H
