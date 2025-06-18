//
// Created by allan on 18/06/2025.
//
#include <gtest/gtest.h>
#include <httplib.h>
#include "mantis/app/app.h"
#include "mantis/core/tables/tables.h"

class AccessPermissionTest : public ::testing::Test {
protected:
    void SetUp() override {
        app = std::make_unique<mantis::MantisApp>();
        app->initialize();

        // Create test table with specific access rules
        createTestTableWithRules();

        // Start server
        server_thread = std::thread([this]() {
            app->run();
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        client = std::make_unique<httplib::Client>("http://localhost:7070");
    }

    void createTestTableWithRules() {
        // Create a table with specific access rules for testing
        nlohmann::json table_schema = {
            {"name", "test_permissions"},
            {"type", "base"},
            {"listRule", "auth.table == 'users'"},           // Only users can list
            {"getRule", "auth.id == req.id"},                // Users can only get their own records
            {"addRule", "auth.table == 'users'"},            // Only users can add
            {"updateRule", "auth.id == req.id"},             // Users can only update their own records
            {"deleteRule", "auth.table == '__admin'"},       // Only admins can delete
            {"fields", nlohmann::json::array({
                {{"name", "title"}, {"type", "string"}, {"required", true}},
                {{"name", "user_id"}, {"type", "string"}, {"required", true}}
            })}
        };

        // Create table through system API
        auto result = client->Post("/api/v1/tables",
            table_schema.dump(), "application/json");
    }

    std::string createUserAndGetToken(const std::string& email) {
        // Create user
        nlohmann::json user = {
            {"name", "Test User"},
            {"email", email},
            {"password", "testpass123"}
        };

        client->Post("/api/v1/users", user.dump(), "application/json");

        // Login and get token
        nlohmann::json login = {{"email", email}, {"password", "testpass123"}};
        auto auth_result = client->Post("/api/v1/users/auth-with-password",
            login.dump(), "application/json");

        auto response = nlohmann::json::parse(auth_result->body);
        return response["data"]["token"].get<std::string>();
    }

    std::string createAdminAndGetToken() {
        // Create admin user
        nlohmann::json admin = {
            {"name", "Admin User"},
            {"email", "admin@test.com"},
            {"password", "adminpass123"}
        };

        client->Post("/api/v1/admins", admin.dump(), "application/json");

        // Login as admin
        nlohmann::json login = {{"email", "admin@test.com"}, {"password", "adminpass123"}};
        auto auth_result = client->Post("/api/v1/admins/auth-with-password",
            login.dump(), "application/json");

        auto response = nlohmann::json::parse(auth_result->body);
        return response["data"]["token"].get<std::string>();
    }

    std::unique_ptr<mantis::MantisApp> app;
    std::unique_ptr<httplib::Client> client;
    std::thread server_thread;
};

// List Rule Tests
TEST_F(AccessPermissionTest, ListRule_AllowsAuthorizedUsers) {
    std::string token = createUserAndGetToken("user@test.com");

    httplib::Headers headers = {{"Authorization", "Bearer " + token}};
    auto result = client->Get("/api/v1/test_permissions", headers);

    EXPECT_EQ(result->status, 200);
    auto response = nlohmann::json::parse(result->body);
    EXPECT_EQ(response["status"], 200);
}

TEST_F(AccessPermissionTest, ListRule_DeniesUnauthorizedUsers) {
    // Try to access without token
    auto result = client->Get("/api/v1/test_permissions");

    EXPECT_EQ(result->status, 403);
    auto response = nlohmann::json::parse(result->body);
    EXPECT_EQ(response["status"], 403);
    EXPECT_FALSE(response["error"].get<std::string>().empty());
}

TEST_F(AccessPermissionTest, ListRule_DeniesWrongUserType) {
    std::string admin_token = createAdminAndGetToken();

    httplib::Headers headers = {{"Authorization", "Bearer " + admin_token}};
    auto result = client->Get("/api/v1/test_permissions", headers);

    // Should be denied because rule requires auth.table == 'users', but admin has '__admin'
    EXPECT_EQ(result->status, 403);
}

// Get Rule Tests
TEST_F(AccessPermissionTest, GetRule_AllowsOwnerAccess) {
    std::string token = createUserAndGetToken("owner@test.com");

    // Create a record first
    nlohmann::json record = {{"title", "Test Record"}, {"user_id", "user123"}};
    httplib::Headers headers = {{"Authorization", "Bearer " + token}};

    auto create_result = client->Post("/api/v1/test_permissions",
        record.dump(), "application/json", headers);
    EXPECT_EQ(create_result->status, 201);

    auto create_response = nlohmann::json::parse(create_result->body);
    std::string record_id = create_response["data"]["id"];

    // Try to get the record - should succeed if auth.id matches
    auto get_result = client->Get("/api/v1/test_permissions/" + record_id, headers);
    EXPECT_EQ(get_result->status, 200);
}

TEST_F(AccessPermissionTest, GetRule_DeniesNonOwnerAccess) {
    std::string owner_token = createUserAndGetToken("owner@test.com");
    std::string other_token = createUserAndGetToken("other@test.com");

    // Create record with owner token
    nlohmann::json record = {{"title", "Private Record"}, {"user_id", "owner123"}};
    httplib::Headers owner_headers = {{"Authorization", "Bearer " + owner_token}};

    auto create_result = client->Post("/api/v1/test_permissions",
        record.dump(), "application/json", owner_headers);
    auto create_response = nlohmann::json::parse(create_result->body);
    std::string record_id = create_response["data"]["id"];

    // Try to access with different user token
    httplib::Headers other_headers = {{"Authorization", "Bearer " + other_token}};
    auto get_result = client->Get("/api/v1/test_permissions/" + record_id, other_headers);

    EXPECT_EQ(get_result->status, 403);
}

// Add Rule Tests
TEST_F(AccessPermissionTest, AddRule_AllowsAuthorizedUsers) {
    std::string token = createUserAndGetToken("creator@test.com");

    nlohmann::json record = {{"title", "New Record"}, {"user_id", "creator123"}};
    httplib::Headers headers = {{"Authorization", "Bearer " + token}};

    auto result = client->Post("/api/v1/test_permissions",
        record.dump(), "application/json", headers);

    EXPECT_EQ(result->status, 201);
    auto response = nlohmann::json::parse(result->body);
    EXPECT_EQ(response["status"], 201);
}

TEST_F(AccessPermissionTest, AddRule_DeniesGuestUsers) {
    nlohmann::json record = {{"title", "Unauthorized Record"}, {"user_id", "guest123"}};

    auto result = client->Post("/api/v1/test_permissions",
        record.dump(), "application/json");

    EXPECT_EQ(result->status, 403);
}

// Update Rule Tests
TEST_F(AccessPermissionTest, UpdateRule_AllowsOwnerUpdates) {
    std::string token = createUserAndGetToken("updater@test.com");
    httplib::Headers headers = {{"Authorization", "Bearer " + token}};

    // Create record
    nlohmann::json record = {{"title", "Original Title"}, {"user_id", "updater123"}};
    auto create_result = client->Post("/api/v1/test_permissions",
        record.dump(), "application/json", headers);

    auto create_response = nlohmann::json::parse(create_result->body);
    std::string record_id = create_response["data"]["id"];

    // Update record
    nlohmann::json update = {{"title", "Updated Title"}};
    auto update_result = client->Patch("/api/v1/test_permissions/" + record_id,
        update.dump(), "application/json", headers);

    EXPECT_EQ(update_result->status, 200);
}

TEST_F(AccessPermissionTest, UpdateRule_DeniesNonOwnerUpdates) {
    std::string owner_token = createUserAndGetToken("owner@test.com");
    std::string other_token = createUserAndGetToken("other@test.com");

    // Create record with owner
    nlohmann::json record = {{"title", "Owner's Record"}, {"user_id", "owner123"}};
    httplib::Headers owner_headers = {{"Authorization", "Bearer " + owner_token}};

    auto create_result = client->Post("/api/v1/test_permissions",
        record.dump(), "application/json", owner_headers);
    auto create_response = nlohmann::json::parse(create_result->body);
    std::string record_id = create_response["data"]["id"];

    // Try to update with different user
    nlohmann::json update = {{"title", "Hacked Title"}};
    httplib::Headers other_headers = {{"Authorization", "Bearer " + other_token}};

    auto update_result = client->Patch("/api/v1/test_permissions/" + record_id,
        update.dump(), "application/json", other_headers);

    EXPECT_EQ(update_result->status, 403);
}

// Delete Rule Tests
TEST_F(AccessPermissionTest, DeleteRule_AllowsAdminOnly) {
    std::string admin_token = createAdminAndGetToken();
    std::string user_token = createUserAndGetToken("user@test.com");

    // Create record with user token
    nlohmann::json record = {{"title", "To Be Deleted"}, {"user_id", "user123"}};
    httplib::Headers user_headers = {{"Authorization", "Bearer " + user_token}};

    auto create_result = client->Post("/api/v1/test_permissions",
        record.dump(), "application/json", user_headers);
    auto create_response = nlohmann::json::parse(create_result->body);
    std::string record_id = create_response["data"]["id"];

    // Try to delete with admin token - should succeed
    httplib::Headers admin_headers = {{"Authorization", "Bearer " + admin_token}};
    auto delete_result = client->Delete("/api/v1/test_permissions/" + record_id, admin_headers);

    EXPECT_EQ(delete_result->status, 204);
}

TEST_F(AccessPermissionTest, DeleteRule_DeniesRegularUsers) {
    std::string user_token = createUserAndGetToken("user@test.com");
    httplib::Headers headers = {{"Authorization", "Bearer " + user_token}};

    // Create record
    nlohmann::json record = {{"title", "Cannot Delete"}, {"user_id", "user123"}};
    auto create_result = client->Post("/api/v1/test_permissions",
        record.dump(), "application/json", headers);
    auto create_response = nlohmann::json::parse(create_result->body);
    std::string record_id = create_response["data"]["id"];

    // Try to delete with user token - should fail
    auto delete_result = client->Delete("/api/v1/test_permissions/" + record_id, headers);

    EXPECT_EQ(delete_result->status, 403);
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

    client->Post("/api/v1/tables", complex_table.dump(), "application/json");

    // Test with unverified user - should fail
    std::string token = createUserAndGetToken("unverified@test.com");
    httplib::Headers headers = {{"Authorization", "Bearer " + token}};

    auto result = client->Get("/api/v1/verified_only", headers);
    EXPECT_EQ(result->status, 403);
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

    client->Post("/api/v1/tables", admin_table.dump(), "application/json");

    std::string user_token = createUserAndGetToken("regular@test.com");
    std::string admin_token = createAdminAndGetToken();

    // Regular user should be denied
    httplib::Headers user_headers = {{"Authorization", "Bearer " + user_token}};
    auto user_result = client->Get("/api/v1/admin_only", user_headers);
    EXPECT_EQ(user_result->status, 403);

    auto user_response = nlohmann::json::parse(user_result->body);
    EXPECT_EQ(user_response["status"], 403);
    EXPECT_EQ(user_response["error"], "Admin auth required to access this resource.");

    // Admin user should be allowed
    httplib::Headers admin_headers = {{"Authorization", "Bearer " + admin_token}};
    auto admin_result = client->Get("/api/v1/admin_only", admin_headers);
    EXPECT_EQ(admin_result->status, 200);

    auto admin_response = nlohmann::json::parse(admin_result->body);
    EXPECT_EQ(admin_response["status"], 200);
}
