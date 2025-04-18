#pragma once
#include "gmock/gmock.h"
#include "IDatabase.hpp"

class MockDatabase : public IDatabase {
public:
    MOCK_METHOD(bool, connect, (const std::string&), (override));
    MOCK_METHOD(void, disconnect, (), (override));

    MOCK_METHOD(int, add_client, (int), (override));
    MOCK_METHOD(bool, remove_client, (int), (override));
    MOCK_METHOD(bool, update_client, (const client&), (override));
    MOCK_METHOD(client, get_client_by_id, (int), (override));
    MOCK_METHOD(client, get_client_by_ip, (int), (override));
    MOCK_METHOD(int, get_client_id_by_ip, (int), (override));
    MOCK_METHOD(int, get_client_ip_by_id, (int), (override));

    MOCK_METHOD(int, add_event, (const event&), (override));
    MOCK_METHOD(bool, remove_event, (int), (override));
    MOCK_METHOD(bool, update_event, (const event&), (override));
    MOCK_METHOD(bool, set_event_response, (int, const std::string&), (override));
    MOCK_METHOD(bool, delete_events_by_client, (int), (override));
    MOCK_METHOD(bool, delete_events_older_than, (int), (override));
    MOCK_METHOD(event, get_event, (int), (override));
    MOCK_METHOD(std::vector<event>, get_events_by_client, (int), (override));
    MOCK_METHOD(std::vector<event>, get_events_newer_than, (int), (override));
};