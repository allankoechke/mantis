//
// Created by allan on 18/06/2025.
//

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <string>
#include <filesystem>

class TestDatabase {
public:
    static std::string getTestDbPath() {
        return std::filesystem::temp_directory_path() / "mantis_test.db";
    }

    static void cleanupTestDb() {
        std::filesystem::remove(getTestDbPath());
    }
};

#endif //TEST_HELPERS_H
