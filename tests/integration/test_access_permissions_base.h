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
        // Just create the HTTP client - server is already running
        client = std::make_unique<httplib::Client>("http://localhost:7075");

        // Clean up any test data from previous tests
        // cleanupTestData();

        // Create test tables for this test suite
        // createTestTableWithRules();
    }

    void TearDown() override {
        // Clean up test data after each test
        cleanupTestData();
    }

    void cleanupTestData() const;

    void createTestTableWithRules() const;

    [[nodiscard]]
    std::string createUserAndGetToken(const std::string& email) const;

    [[nodiscard]]
    std::string createAdminAndGetToken() const;

    std::unique_ptr<httplib::Client> client;
};