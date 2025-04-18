#include "EventLogger.hpp"

EventLogger::~EventLogger()
{
	m_db->disconnect();
}

int EventLogger::get_client_id(int client_ip)
{
	return m_db->get_client_id_by_ip(client_ip);
}

int EventLogger::add_client(int client_ip)
{
	return m_db->add_client(client_ip);
}

int EventLogger::log_event(int client_ip, int event_type, const std::string& event_data)
{
	auto client_id = m_db->get_client_id_by_ip(client_ip);
	if (-1 == client_id)
	{
		client_id = m_db->add_client(client_ip);
	}

	event evt = { 0, client_id, event_type, 0, event_data, "" };
	return m_db->add_event(evt);
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
