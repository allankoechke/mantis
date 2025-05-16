//
// Created by allan on 12/05/2025.
//

#include "logging.h"
#include <spdlog/sinks/stdout_color_sinks-inl.h>

void mantis::LoggingUnit::close()
{
    spdlog::shutdown();
}

void mantis::LoggingUnit::setLogLevel(const LogLevel& level)
{
    switch (level)
    {
    case LogLevel::TRACE: spdlog::set_level(spdlog::level::trace); break;
    case LogLevel::DEBUG: spdlog::set_level(spdlog::level::debug); break;
    case LogLevel::INFO: spdlog::set_level(spdlog::level::info); break;
    case LogLevel::WARN: spdlog::set_level(spdlog::level::warn); break;
    case LogLevel::CRITICAL: spdlog::set_level(spdlog::level::critical); break;
    }
}

void mantis::LoggingUnit::init()
{
    // auto color_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
    //
    // // Set whole-line foreground color by log level (no background changes)
    // color_sink->set_color(spdlog::level::trace,    "\033[90m");  // gray
    // color_sink->set_color(spdlog::level::debug,    "\033[36m");  // cyan
    // color_sink->set_color(spdlog::level::info,     "\033[32m");  // green
    // color_sink->set_color(spdlog::level::warn,     "\033[33m");  // yellow
    // color_sink->set_color(spdlog::level::err,      "\033[31m");  // red
    // color_sink->set_color(spdlog::level::critical, "\033[91m");  // bright red
    //
    // // Set the log pattern
    // color_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%-8l] %v");
    //
    // // Set this as the default logger
    // auto logger = std::make_shared<spdlog::logger>("mantis_logger", color_sink);
    // spdlog::set_default_logger(logger);

    auto console = spdlog::stdout_color_mt("console");
    console->set_pattern("[%Y-%m-%d %H:%M:%S] [%-8l] %v");
    spdlog::set_default_logger(console);

    // spdlog::cfg::load_env_levels("MANTIS_LOG_LEVEL");
    // auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    //
    // console_sink->set_level(spdlog::level::trace);
    // console_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%-7l] %v");
    //
    // auto file_sink =
    //     std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/mantis.log", true);
    // file_sink->set_level(spdlog::level::trace);
    //
    // spdlog::logger logger("multi_sink", {console_sink, file_sink});
    // logger.set_level(spdlog::level::trace);
}
