#pragma once
#include <string>
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
    virtual void log(LogLevel level, const std::string& message) = 0;

    // Overloads accepting dynamic strings
    virtual void trace(const std::string& message) = 0;
    virtual void debug(const std::string& message) = 0;
    virtual void info(const std::string& message) = 0;
    virtual void error(const std::string& message) = 0;
    virtual void critical(const std::string& message) = 0;

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
