//
// Created by allan on 12/05/2025.
//

#ifndef MANTIS_LOGGER_H
#define MANTIS_LOGGER_H

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
using json = nlohmann::json;

namespace Mantis
{
    typedef enum class LogLevel : uint8_t
    {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        CRITICAL
    } LogLevel;


    class Logger
    {
    public:
        ~Logger() = default;

        static void Config();
        static void Shutdown();

        static void SetLogLevel(const LogLevel& level = LogLevel::INFO);

        template <typename... Args>
        static void Info(fmt::format_string<Args...> msg, Args&&... args)
        {
            spdlog::info(msg, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void Debug(fmt::format_string<Args...> msg, Args&&... args)
        {
            spdlog::debug(msg, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void Warn(fmt::format_string<Args...> msg, Args&&... args)
        {
            spdlog::warn(msg, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void Critical(fmt::format_string<Args...> msg, Args&&... args)
        {
            spdlog::critical(msg, std::forward<Args>(args)...);
        }

    private:
        Logger() = delete;
    };
}

#endif //MANTIS_LOGGER_H
