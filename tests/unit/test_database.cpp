//
// Created by allan on 18/06/2025.
//
#include <gtest/gtest.h>
#include <mantis/core/database.h>
#include <mantis/app/app.h>

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        app = std::make_unique<mantis::MantisApp>();
        app->setDbType(mantis::DbType::SQLITE);
        db = std::make_unique<mantis::DatabaseUnit>(app.get());
    }

    std::unique_ptr<mantis::MantisApp> app;
    std::unique_ptr<mantis::DatabaseUnit> db;
};

TEST_F(DatabaseTest, ConnectToSQLite) {
    EXPECT_TRUE(db->connect(mantis::DbType::SQLITE, ""));
    EXPECT_TRUE(db->isConnected());
}

TEST_F(DatabaseTest, MigrationCreatesSystemTables) {
    db->connect(mantis::DbType::SQLITE, "");
    // TODO
    // EXPECT_TRUE(db->migrate());
    // Verify __tables system table exists
}