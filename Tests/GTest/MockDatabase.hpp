#pragma once
#include "gmock/gmock.h"
#include "IDatabase.hpp"

class MockDatabase : public IDatabase {
public:
    MOCK_METHOD(bool, connect, (const std::string&), (override));
    MOCK_METHOD(void, disconnect, (), (override));
};