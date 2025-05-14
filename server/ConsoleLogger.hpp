#pragma once
#include <iostream>
#include "ILogger.hpp"

class ConsoleLogger : public ILogger {
public:
	void log(LogLevel level, const std::string& message) override {
		std::cout << "[" << level_to_string(level) << "] " << message << std::endl;
	}

	void debug(const std::string& message) override { log(LogLevel::Debug, message); }
	void trace(const std::string& message) override { log(LogLevel::Trace, message); }
	void info(const std::string& message)  override { log(LogLevel::Info, message); }
	void error(const std::string& message) override { log(LogLevel::Error, message); }
	void critical(const std::string& message) override { log(LogLevel::Critical, message); }

private:
	std::string level_to_string(LogLevel level) {
		switch (level) {
		case LogLevel::Trace: return "TRACE";
		case LogLevel::Debug: return "DEBUG";
		case LogLevel::Info: return "INFO";
		case LogLevel::Error: return "ERROR";
		case LogLevel::Critical: return "CRITICAL";
		default: return "UNKNOWN";
		}
	}
};