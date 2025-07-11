//
// Created by allan on 18/06/2025.
//

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <string>
#include <filesystem>

class TestDatabase {
public:
    static std::filesystem::path getTestBasePath(const std::string& p = "") {
        namespace fs = std::filesystem;
        fs::path base = fs::temp_directory_path() / "_mantisapp";
        if (!p.empty()) base = base / p;

        try
        {
            fs::create_directories(base);
        } catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }

        return base;
    }

    static std::string getTestDataPath() {
        return (getTestBasePath() / "data").string();
    }

    static std::string getTestPublicPath() {
        return (getTestBasePath() / "public").string();
    }

    static void cleanupTestDb() {
        try
        {
            std::filesystem::remove_all(getTestBasePath());
        } catch (const std::exception& e){}
    }
};

#endif //TEST_HELPERS_H
