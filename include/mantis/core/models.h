//
// Created by allan on 08/05/2025.
//

#ifndef MODELS_H
#define MODELS_H

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Enum of the table type created,
// base table types provide `index`, `created`, `updated`
// auth table type provide `base` type + `email`, `password`, `name`
// view table type provide readonly `sql`
typedef enum TableType
{
    BaseTable = 1,
    AuthTable,
    ViewTable
} TableType;

struct BaseTable
{
    std::string id;
    uint64_t created_at;
    uint64_t updated_at;

    json listRule = "";
    json getRule = "";
    json addRule = "";
    json updateRule = "";
    json deleteRule = "";
};

struct AuthTable: BaseTable
{
    std::string email;
    std::string name;
    std::string password;
};

struct ViewTable
{
    std::string sql;

    json listRule = "";
    json getRule = "";
};



#endif //MODELS_H
