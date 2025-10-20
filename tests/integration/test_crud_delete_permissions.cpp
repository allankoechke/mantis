//
// Created by allan on 18/06/2025.
//
#include "test_access_permissions_base.h"

// Delete Rule Tests
TEST_F(AccessPermissionTest, DeleteRule_AllowsAdminOnly) {
    // const std::string admin_token = createAdminAndGetToken();
    // const std::string user_token = createUserAndGetToken("user@test.com");
    //
    // // Create record with user token
    // const nlohmann::json record = {{"title", "To Be Deleted"}, {"user_id", "user123"}};
    // httplib::Headers user_headers = {{"Authorization", "Bearer " + user_token}};
    //
    // auto create_result = client->Post("/api/v1/test_permissions", user_headers ,
    //     record.dump(), "application/json");
    // auto create_response = nlohmann::json::parse(create_result->body);
    // std::string record_id = create_response["data"]["id"];
    //
    // // Try to delete with admin token - should succeed
    // httplib::Headers admin_headers = {{"Authorization", "Bearer " + admin_token}};
    // auto delete_result = client->Delete("/api/v1/test_permissions/" + record_id, admin_headers);

    // EXPECT_EQ(delete_result->status, 204);
}

TEST_F(AccessPermissionTest, DeleteRule_DeniesRegularUsers) {
    // const std::string user_token = createUserAndGetToken("user@test.com");
    // const httplib::Headers headers = {{"Authorization", "Bearer " + user_token}};
    //
    // // Create record
    // const nlohmann::json record = {{"title", "Cannot Delete"}, {"user_id", "user123"}};
    // auto create_result = client->Post("/api/v1/test_permissions", headers ,
    //     record.dump(), "application/json");
    // auto create_response = nlohmann::json::parse(create_result->body);
    // const std::string record_id = create_response["data"]["id"];
    //
    // // Try to delete with user token - should fail
    // auto delete_result = client->Delete("/api/v1/test_permissions/" + record_id, headers);

    // EXPECT_EQ(delete_result->status, 403);
}

// Complex Rule Expression Tests
TEST_F(AccessPermissionTest, ComplexRules_MultipleConditions) {
    // Test table with complex rule: "auth.table == 'users' && auth.verified == true"
    nlohmann::json complex_table = {
        {"name", "verified_only"},
        {"type", "base"},
        {"listRule", "auth.table == 'users' && auth.verified == true"},
        {"fields", nlohmann::json::array({
            {{"name", "content"}, {"type", "string"}, {"required", true}}
        })}
    };

    // client->Post("/api/v1/tables", complex_table.dump(), "application/json");
    //
    // // Test with unverified user - should fail
    // const std::string token = createUserAndGetToken("unverified@test.com");
    // const httplib::Headers headers = {{"Authorization", "Bearer " + token}};
    //
    // auto result = client->Get("/api/v1/verified_only", headers);
    // EXPECT_EQ(result->status, 403);
}

TEST_F(AccessPermissionTest, EmptyRule_RequiresAdminAccess)
{
    // Test table with empty rule (should default to admin-only access)
    nlohmann::json admin_table = {
        {"name", "admin_only"},
        {"type", "base"},
        {"listRule", ""},  // Empty rule
        {"fields", nlohmann::json::array({
            {{"name", "secret"}, {"type", "string"}, {"required", true}}
        })}
    };

    // client->Post("/api/v1/tables", admin_table.dump(), "application/json");
    //
    // std::string user_token = createUserAndGetToken("regular@test.com");
    // std::string admin_token = createAdminAndGetToken();
    //
    // // Regular user should be denied
    // httplib::Headers user_headers = {{"Authorization", "Bearer " + user_token}};
    // auto user_result = client->Get("/api/v1/admin_only", user_headers);
    // // EXPECT_EQ(user_result->status, 403);
    //
    // auto user_response = nlohmann::json::parse(user_result->body);
    // // EXPECT_EQ(user_response["status"], 403);
    // // EXPECT_EQ(user_response["error"], "Admin auth required to access this resource.");
    //
    // // Admin user should be allowed
    // httplib::Headers admin_headers = {{"Authorization", "Bearer " + admin_token}};
    // auto admin_result = client->Get("/api/v1/admin_only", admin_headers);
    // // EXPECT_EQ(admin_result->status, 200);
    //
    // auto admin_response = nlohmann::json::parse(admin_result->body);
    // EXPECT_EQ(admin_response["status"], 200);
}
