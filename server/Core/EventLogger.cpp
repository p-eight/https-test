#include "EventLogger.hpp"

int EventLogger::get_client_id(int client_ip)
{
	return 0;
}

bool EventLogger::log_event(int client_id, int event_type, const std::string& event_data)
{
	return false;
}

bool EventLogger::log_event_response(int event_id, const std::string& response)
{
	return false;
}

bool EventLogger::delete_events_by_client(int client_id)
{
	return false;
}

bool EventLogger::delete_events_older_than(int timestamp)
{
	return false;
}
