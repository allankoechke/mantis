//
// Created by allan on 18/06/2025.
//
#include "test_access_permissions_base.h"

// Add Rule Tests
TEST_F(AccessPermissionTest, AddRule_AllowsAuthorizedUsers) {
    const std::string token = createUserAndGetToken("creator@test.com");

    const nlohmann::json record = {{"title", "New Record"}, {"user_id", "creator123"}};
    const httplib::Headers headers = {{"Authorization", "Bearer " + token}};

    auto result = client->Post("/api/v1/test_permissions", headers,
        record.dump(), "application/json");

    EXPECT_EQ(result->status, 201);
    auto response = nlohmann::json::parse(result->body);
    EXPECT_EQ(response["status"], 201);
}

TEST_F(AccessPermissionTest, AddRule_DeniesGuestUsers) {
    const nlohmann::json record = {{"title", "Unauthorized Record"}, {"user_id", "guest123"}};

    auto result = client->Post("/api/v1/test_permissions",
        record.dump(), "application/json");

    EXPECT_EQ(result->status, 403);
}