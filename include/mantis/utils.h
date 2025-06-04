//
// Created by allan on 11/05/2025.
//

#ifndef MANTIS_UTILS_H
#define MANTIS_UTILS_H

#include <string>
#include <algorithm>
#include <filesystem>
#include "core/logging.h"
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace mantis
{
    namespace fs = std::filesystem;

    inline void toLowerCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
                       [](const unsigned char c) { return std::tolower(c); });
    }

    inline void toUpperCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
                       [](const unsigned char c) { return std::toupper(c); });
    }

    inline fs::path joinPaths(const std::string& path1, const std::string& path2)
    {
        fs::path result = fs::path(path1) / fs::path(path2);
        return result;
    }

    inline fs::path resolvePath(const std::string& input_path)
    {
        fs::path path(input_path);

        if (!path.is_absolute())
        {
            // Resolve relative to app binary
            path = fs::absolute(path);
        }

        return path;
    }

    inline bool createDirs(const fs::path& path)
    {
        try
        {
            if (!fs::exists(path))
            {
                fs::create_directories(path); // creates all missing parent directories too
            }

            return true;
        }
        catch (const fs::filesystem_error& e)
        {
            Log::critical("Filesystem error while creating directory '{}', reason: {}",
                          path.string(), e.what());
            return false;
        }
    }

    inline std::string dirFromPath(const std::string& path)
    {
        if (const auto dir = resolvePath(path); createDirs(dir))
            return dir.string();

        return "";
    }

    inline std::string trim(const std::string& s)
    {
        auto start = std::ranges::find_if_not(s, ::isspace);
        auto end = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
        return (start < end) ? std::string(start, end) : "";
    }

    inline std::optional<json> tryParseJsonStr(const std::string& json_str)
    {
        try
        {
            auto res = json::parse(json_str);
            return res;
        }
        catch (const std::exception& e)
        {
            Log::critical("JSON parse error: {}", e.what());
            return std::nullopt;
        }
    }

    /*
    * First part = milliseconds since epoch
    * Last 4 digits = random component
    * Lexicographically sortable by time
     *
     * Sample Output: 17171692041233276
     */
    inline std::string generateTimeBasedId() // 17171692041233276
    {
        // Get current time since epoch in milliseconds
        using namespace std::chrono;
        auto now = system_clock::now();
        auto millis = duration_cast<milliseconds>(now.time_since_epoch()).count();

        // Generate 4-digit random suffix
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 9999);

        std::ostringstream oss;
        oss << millis << std::setw(4) << std::setfill('0') << dis(gen);
        return oss.str();
    }

    /*
    * Sample Output: 20250531T221944517N3J
    * ISO-formatted time + milliseconds + short random suffix
    * Human-readable and sortable
    */
    inline std::string generateReadableTimeId()
    {
        using namespace std::chrono;
        const auto now = system_clock::now();
        const auto tt = system_clock::to_time_t(now);
        const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        const std::tm tm = *std::localtime(&tt);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y%m%dT%H%M%S");
        oss << std::setw(3) << std::setfill('0') << ms.count();

        // Append 3-character random suffix
        static constexpr char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<> dis(0, 35);
        for (int i = 0; i < 3; ++i)
            oss << charset[dis(gen)];

        return oss.str();
    }

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

    inline std::vector<std::string> splitString(const std::string& input, const std::string& delimiter) {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = 0;

        while ((end = input.find(delimiter, start)) != std::string::npos) {
            tokens.push_back(input.substr(start, end - start));
            start = end + delimiter.length();
        }

        tokens.push_back(input.substr(start)); // last token
        return tokens;
    }
}

#endif // MANTIS_UTILS_H
