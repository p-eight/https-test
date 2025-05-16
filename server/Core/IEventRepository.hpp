#pragma once
#include <string>
#include <vector>

class IEventRepository
{
public:
	struct Event
	{
		int id;
		int client_id;
		int connection_id;
		int timestamp;
		std::string event_data;
		std::string event_response;
	};
	virtual ~IEventRepository() = default;
	virtual int add_event(int client_id, int connection_id, int timestamp, const std::string& event_data) = 0;
	virtual bool remove_event(int event_id) = 0;
	virtual bool remove_all_events() = 0;
	virtual bool remove_events_by_client(int client_id) = 0;
	virtual std::vector<Event> get_events_by_client(int client_id) = 0;
};