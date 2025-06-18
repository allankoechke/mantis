//
// Created by allan on 18/06/2025.
//
#include <gtest/gtest.h>
#include "mantis/core/models/models.h"
#include <mantis/mantis.h>

TEST(FieldTest, CreateFieldWithValidation) {
    mantis::Field field("email", mantis::FieldType::STRING, true);
    field.regexPattern = "^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$";

    auto json_repr = field.to_json();
    EXPECT_EQ(json_repr["name"], "email");
    EXPECT_EQ(json_repr["type"], "text");
    EXPECT_TRUE(json_repr["required"]);
}

TEST(TableTest, BaseFieldsExist) {
    EXPECT_TRUE(mantis::fieldExists(mantis::TableType::Base, "id"));
    EXPECT_TRUE(mantis::fieldExists(mantis::TableType::Base, "created"));
    EXPECT_TRUE(mantis::fieldExists(mantis::TableType::Base, "updated"));
}