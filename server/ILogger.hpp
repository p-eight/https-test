#pragma once
#include <string>

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
    virtual void log(LogLevel level, const std::string& message) = 0;
	virtual void debug(const std::string& message) = 0;
	virtual void trace(const std::string& message) = 0;
	virtual void info(const std::string& message) = 0;
	virtual void error(const std::string& message) = 0;
	virtual void critical(const std::string& message) = 0;
};