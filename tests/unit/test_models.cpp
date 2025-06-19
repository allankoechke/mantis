//
// Created by allan on 18/06/2025.
//
#include <gtest/gtest.h>
#include "mantis/core/models/models.h"

TEST(FieldTest, CreateFieldWithValidation) {
    mantis::Field field("email", mantis::FieldType::STRING, true, false);
    field.regexPattern = "^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$";

    auto json_repr = field.to_json();
    EXPECT_EQ(json_repr["name"], "email");
    EXPECT_EQ(json_repr["type"], "string");
    EXPECT_TRUE(json_repr["required"]);
    EXPECT_FALSE(json_repr["primaryKey"]);
}

TEST(TableTest, BaseFieldsExist) {
    EXPECT_TRUE(mantis::fieldExists(mantis::TableType::Base, "id"));
    EXPECT_TRUE(mantis::fieldExists(mantis::TableType::Base, "created"));
    EXPECT_TRUE(mantis::fieldExists(mantis::TableType::Base, "updated"));
}