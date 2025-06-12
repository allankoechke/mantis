//
// Created by allan on 17/05/2025.
//

#ifndef MANTIS_H
#define MANTIS_H

#include "utils.h"

#include "app/app.h"

#include "core/database.h"
#include "core/crud/crud.h"
#include "core/http.h"
#include "core/logging.h"
#include "core/router.h"

#include "core/models/models.h"
#include "core/tables/tables.h"

// For convenience to using json,
// lets include it here
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <argparse/argparse.hpp>
#endif //MANTIS_H
