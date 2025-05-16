//
// Created by allan on 12/05/2025.
//

#ifndef MANTIS_LOGGER_H
#define MANTIS_LOGGER_H

#include <../../3rdParty/json/single_include/nlohmann/json.hpp>
#include <../../3rdParty/spdlog/include/spdlog/spdlog.h>
#include <../../3rdParty/spdlog/include/spdlog/sinks/basic_file_sink.h>
using json = nlohmann::json;

namespace mantis
{
    typedef enum class LogLevel : uint8_t
    {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        CRITICAL
    } LogLevel;

    class LoggingUnit
    {
    public:
        LoggingUnit() = default;
        ~LoggingUnit() = default;

        static void init();
        static void close();

        static void setLogLevel(const LogLevel& level = LogLevel::INFO);

        template <typename... Args>
        static void info(fmt::format_string<Args...> msg, Args&&... args)
        {
            spdlog::info(msg, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void debug(fmt::format_string<Args...> msg, Args&&... args)
        {
            spdlog::debug(msg, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void warn(fmt::format_string<Args...> msg, Args&&... args)
        {
            spdlog::warn(msg, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void critical(fmt::format_string<Args...> msg, Args&&... args)
        {
            spdlog::critical(msg, std::forward<Args>(args)...);
        }
    };

    using Log = LoggingUnit;
}

#endif //MANTIS_LOGGER_H
