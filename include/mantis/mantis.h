//
// Created by allan on 17/05/2025.
//

#ifndef MANTIS_H
#define MANTIS_H

// Utility functions
#include "utils/utils.h"

// Core application
#include "app/app.h"

// Core components
#include "core/database.h"
#include "core/http.h"
#include "core/logging.h"
#include "core/router.h"
#include "core/expr_evaluator.h"

// CRUD and JWT
#include "core/crud/crud.h"
#include "core/jwt.h"

// Models and data structures
#include "core/models/models.h"

// Table operations
#include "core/tables/tables.h"
#include "core/tables/sys_tables.h"

// For convenience to using json,
// lets include it here
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// For argparse lib
#include <argparse/argparse.hpp>

// Add soci include
#include <soci/soci.h>

#endif //MANTIS_H
