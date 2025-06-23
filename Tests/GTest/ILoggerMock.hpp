#pragma once
#include "gmock/gmock.h"
#include "ILogger.hpp"

class ILoggerMock : public ILogger {
public:
    MOCK_METHOD(void, log, (LogLevel level, const std::string& message), (override));
    MOCK_METHOD(void, trace, (const std::string& message), (override));
    MOCK_METHOD(void, debug, (const std::string& message), (override));
    MOCK_METHOD(void, info, (const std::string& message), (override));
    MOCK_METHOD(void, error, (const std::string& message), (override));
    MOCK_METHOD(void, critical, (const std::string& message), (override));
};