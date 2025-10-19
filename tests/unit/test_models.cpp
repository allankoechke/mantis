//
// Created by allan on 18/06/2025.
//
#include <catch2/catch_all.hpp>
#include "mantis/core/models/models.h"

TEST_CASE("FieldTest, CreateFieldWithValidation", "[unit]") {
    mantis::Field field("email", mantis::FieldType::STRING, true, false);
    field.regexPattern = "^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$";

    auto json_repr = field.to_json();
    REQUIRE(json_repr["name"] == "email");
    REQUIRE(json_repr["type"] == "string");
    REQUIRE(json_repr["required"]);
    REQUIRE_FALSE(json_repr["primaryKey"]);
}

TEST_CASE("TableTest, BaseFieldsExist", "[unit]") {
    REQUIRE(mantis::fieldExists(mantis::TableType::Base, "id"));
    REQUIRE(mantis::fieldExists(mantis::TableType::Base, "created"));
    REQUIRE(mantis::fieldExists(mantis::TableType::Base, "updated"));
}