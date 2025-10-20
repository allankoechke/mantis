//
// Created by allan on 18/06/2025.
//

#include <gtest/gtest.h>
#include <mantis/core/database.h>
#include <mantis/app/app.h>
#include "mantis/core/tables/tables.h"

class DatabaseTest : public ::testing::Test {

};

TEST_F(DatabaseTest, ConnectToDB) {
    EXPECT_TRUE(mantis::MantisApp::instance().db().isConnected());
}

TEST_F(DatabaseTest, MigrationForSystemTablesSuccessful) {
    // Verify we have tables
    // const auto sql = mantis::MantisApp::instance().db().session();
    // const soci::rowset<soci::row> rs = (sql->prepare << "SELECT id, name FROM __tables");
    //
    // // int count = 0;
    // for (const auto& row : rs)
    // {
    //     const auto id = row.get<std::string>(0);
    //     const auto name = row.get<std::string>(1);
    //
    //     std::cout << id << " " << name << std::endl;
    //
    //     EXPECT_EQ(id, mantis::TableUnit::generateTableId(name));
    //     // count++;
    // }

    // EXPECT_TRUE(count > 0);
}