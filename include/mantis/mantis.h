//
// Created by allan on 17/05/2025.
//

#ifndef MANTIS_H
#define MANTIS_H

// Utility functions
#include "utils/utils.h"

// Core application
#include "mantisbase.h"

// Core components
#include "core/database_mgr.h"
#include "core/expr_evaluator.h"
#include "core/route_registry.h"
#include "core/logs_mgr.h"
#include "core/router.h"
#include "core/context_store_mgr.h"
#include "core/files_mgr.h"
#include "core/settings_mgr.h"

// CRUD and JWT
#include "core/jwt_mgr.h"

// Models and data structures
#include "core/models/validators.h"
#include "core/models/entity.h"
#include "core/models/entity_schema.h"
#include "core/models/entity_schema_field.h"

// For convenience to using json,
// lets include it here
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// For argparse lib
#include <argparse/argparse.hpp>

// Add soci include
#include <soci/soci.h>

#endif //MANTIS_H
