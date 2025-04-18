#ifndef __EventLogger_hpp__
#define __EventLogger_hpp__
#include "IDatabase.hpp"
#include <memory>

class EventLogger
{
public:
	EventLogger(IDatabase* db) : m_db(db) {}
	~EventLogger() {}
	int	 get_client_id(int client_ip);
	bool log_event(int client_id, int event_type, const std::string& event_data);
	bool log_event_response(int event_id, const std::string& response);
	bool delete_events_by_client(int client_id);
	bool delete_events_older_than(int timestamp);
private:
	std::unique_ptr<IDatabase> m_db;

};

#endif // !__EventLogger_hpp__
