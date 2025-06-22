//
// Created by allan on 11/05/2025.
//

#ifndef MANTIS_UTILS_H
#define MANTIS_UTILS_H

#include <string>
#include <filesystem>
#include <random>

#include "../core/logging.h"

namespace mantis
{
    namespace fs = std::filesystem;

    // ----------------------------------------------------------------- //
    // PATH UTILS
    // ----------------------------------------------------------------- //
    fs::path joinPaths(const std::string& path1, const std::string& path2);

    fs::path resolvePath(const std::string& input_path);

    bool createDirs(const fs::path& path);

    std::string dirFromPath(const std::string& path);

    // ----------------------------------------------------------------- //
    // STRING UTILS
    // ----------------------------------------------------------------- //
    void toLowerCase(std::string& str);

    void toUpperCase(std::string& str);

    std::string trim(const std::string& s);

    std::optional<json> tryParseJsonStr(const std::string& json_str);

    /*
    * First part = milliseconds since epoch
    * Last 4 digits = random component
    * Lexicographically sortable by time
     *
     * Sample Output: 17171692041233276
     */
    std::string generateTimeBasedId();

    /*
    * Sample Output: 20250531T221944517N3J
    * ISO-formatted time + milliseconds + short random suffix
    * Human-readable and sortable
    */
    std::string generateReadableTimeId();

    /*
     * Sample Output: Fz8xYc6a7LQw
     */
    inline std::string generateShortId(const size_t length = 12)
    {
        static constexpr char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dis(0, sizeof(charset) - 2);

        std::string id;
        id.reserve(length);
        for (size_t i = 0; i < length; ++i)
            id += charset[dis(gen)];

        return id;
    }

    std::vector<std::string> splitString(const std::string& input, const std::string& delimiter);

    inline std::string getEnvOrDefault(const std::string& key, const std::string& defaultValue)
    {
        const char* value = std::getenv(key.c_str());
        return value ? std::string(value) : defaultValue;
    }


    // ----------------------------------------------------------------- //
    // AUTH UTILS
    // ----------------------------------------------------------------- //
    std::string bcryptBase64Encode(const unsigned char* data, size_t len);

    std::string generateSalt(int cost = 12);

    json hashPassword(const std::string& password);

    json verifyPassword(const std::string& password, const std::string& stored_hash);

}

#endif // MANTIS_UTILS_H
