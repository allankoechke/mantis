//
// Created by allan on 22/06/2025.
//
#include "../../include/mantis/utils/utils.h"
#include <algorithm>

namespace mantis
{
    std::optional<json> tryParseJsonStr(const std::string& json_str)
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

    bool strToBool(const std::string& value)
    {
        std::string s = value;
        std::ranges::transform(s, s.begin(), ::tolower);
        return (s == "1" || s == "true" || s == "yes" || s == "on");
    }

    void toLowerCase(std::string& str)
    {
        std::ranges::transform(str, str.begin(),
                               [](const unsigned char c) { return std::tolower(c); });
    }

    void toUpperCase(std::string& str)
    {
        std::ranges::transform(str, str.begin(),
                               [](const unsigned char c) { return std::toupper(c); });
    }

    std::string trim(const std::string& s)
    {
        auto start = std::ranges::find_if_not(s, ::isspace);
        auto end = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
        return (start < end) ? std::string(start, end) : "";
    }

    std::string generateTimeBasedId()
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

    std::string generateReadableTimeId()
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

    std::string generateShortId(const size_t length)
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

    std::vector<std::string> splitString(const std::string& input, const std::string& delimiter)
    {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = 0;

        while ((end = input.find(delimiter, start)) != std::string::npos)
        {
            tokens.push_back(input.substr(start, end - start));
            start = end + delimiter.length();
        }

        tokens.push_back(input.substr(start)); // last token
        return tokens;
    }

    std::string getEnvOrDefault(const std::string& key, const std::string& defaultValue)
    {
        const char* value = std::getenv(key.c_str());
        return value ? std::string(value) : defaultValue;
    }

    std::string sanitizeFilename(const std::string& name)
    {
        std::string sanitized;
        for (const char ch : name)
        {
            if (ch == ' ' || ch == '\t')
            {
                sanitized += '_';
            }
            else if (ch == ',')
            {
                continue; // skip commas
            }
            else
            {
                sanitized += ch;
            }
        }
        return sanitized;
    }
}
