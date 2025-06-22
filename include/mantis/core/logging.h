/**
 * @file logging.h
 * @brief Wrapper around spdlog's functionality.
 *
 * Created by allan on 12/05/2025.
 */

#ifndef MANTIS_LOGGER_H
#define MANTIS_LOGGER_H

#include <../../3rdParty/json/single_include/nlohmann/json.hpp>
#include <../../3rdParty/spdlog/include/spdlog/spdlog.h>
#include <../../3rdParty/spdlog/include/spdlog/sinks/basic_file_sink.h>

namespace mantis
{
    using json = nlohmann::json;

    /**
     * Enum for the different logging levels.
     */
    typedef enum class LogLevel : uint8_t
    {
        TRACE = 0,  ///> Trace logging level
        DEBUG,      ///> Debug Logging Level
        INFO,       ///> Info Logging Level
        WARN,       ///> Warning Logging Level
        CRITICAL    ///> Critical Logging Level
    } LogLevel;

    /**
     * A wrapper class around the `spdlog's` logging functions.
     * For more info, check docs here: @see https://github.com/gabime/spdlog
     */
    class LoggingUnit
    {
    public:
        LoggingUnit() = default;
        ~LoggingUnit() = default;

        static void init();
        static void close();

        static void setLogLevel(const LogLevel& level = LogLevel::INFO);

        template <typename... Args>
        static void trace(fmt::format_string<Args...> msg, Args&&... args)
        {
            spdlog::trace(msg, std::forward<Args>(args)...);
        }

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
