//
// Created by allan on 12/05/2025.
//

#include "../../include/mantis/core/logging.h"
#include <spdlog/sinks/stdout_color_sinks-inl.h>
#include "spdlog/sinks/ansicolor_sink.h"

void mantis::LoggingUnit::close()
{
    // Causes SEGFAULT for `heap-use-after-free`, this method is automatically
    // called by app destructor
    // spdlog::shutdown();
}

void mantis::LoggingUnit::setLogLevel(const LogLevel& level)
{
    switch (level)
    {
    case LogLevel::TRACE: spdlog::set_level(spdlog::level::trace);
        break;
    case LogLevel::DEBUG: spdlog::set_level(spdlog::level::debug);
        break;
    case LogLevel::INFO: spdlog::set_level(spdlog::level::info);
        break;
    case LogLevel::WARN: spdlog::set_level(spdlog::level::warn);
        break;
    case LogLevel::CRITICAL: spdlog::set_level(spdlog::level::critical);
        break;
    }
}

void mantis::LoggingUnit::init()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%-8l] %v");

    // auto file_sink =
    //     std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/mantis.log", true);
    // file_sink->set_level(spdlog::level::trace);

    spdlog::logger logger("multi_sink", {console_sink});
    logger.set_level(spdlog::level::trace);
}
