#pragma once
#include "Core/ILogger.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include <spdlog/sinks/stdout_color_sinks.h>

class SPDLogger : public ILogger
{
public:
    SPDLogger() : m_thread_pool(nullptr)
    {
        // Step 1: Create a thread pool (queue size, number of threads)
        std::size_t queue_size = 8192; // Size of the async queue
        std::size_t num_threads = 1;   // Number of threads for logging

        m_thread_pool = std::make_shared<spdlog::details::thread_pool>(queue_size, num_threads);

        // Step 2: Create console sink (colored)
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        // Step 3: Create the async logger
        auto logger = std::make_shared<spdlog::async_logger>(
            "async_console", console_sink, m_thread_pool, spdlog::async_overflow_policy::block);

        // Step 4: Register and use the logger
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$]\t%v");
        spdlog::flush_every(std::chrono::milliseconds(500)); // Flush every second
    };

    void log(LogLevel level, std::string_view message) override
    {
        switch (level)
        {
        case LogLevel::Trace:
            spdlog::trace(message);
            break;
        case LogLevel::Debug:
            spdlog::debug(message);
            break;
        case LogLevel::Info:
            spdlog::info(message);
            break;
        case LogLevel::Error:
            spdlog::error(message);
            break;
        case LogLevel::Critical:
            spdlog::critical(message);
            break;
        }
    }

    void trace(std::string_view message) override { spdlog::trace(message); }
    void debug(std::string_view message) override { spdlog::debug(message); }
    void info(std::string_view message) override { spdlog::info(message); }
    void error(std::string_view message) override { spdlog::error(message); }
    void critical(std::string_view message) override { spdlog::critical(message); }

private:
    std::shared_ptr< spdlog::details::thread_pool> m_thread_pool;
};