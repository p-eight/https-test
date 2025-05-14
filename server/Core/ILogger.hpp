#pragma once
#include <string>
#include <string_view>
#include <fmt/core.h>

class ILogger {
public:
    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Error,
        Critical
    };

    virtual ~ILogger() = default;

    // Base method
    virtual void log(LogLevel level, std::string_view message) = 0;

    // Overloads accepting dynamic strings
    virtual void trace(std::string_view message) = 0;
    virtual void debug(std::string_view message) = 0;
    virtual void info(std::string_view message) = 0;
    virtual void error(std::string_view message) = 0;
    virtual void critical(std::string_view message) = 0;

    // Overloads with fmt compile-time checked formatting
    template<typename... Args>
    void trace(fmt::format_string<Args...> fmt, Args&&... args) {
        trace(fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        debug(fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        info(fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) {
        error(fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void critical(fmt::format_string<Args...> fmt, Args&&... args) {
        critical(fmt::format(fmt, std::forward<Args>(args)...));
    }
};
