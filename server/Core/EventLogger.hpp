#ifndef __EventLogger_hpp__
#define __EventLogger_hpp__
#include "IDatabase.hpp"
#include <memory>
#include <stdexcept>

struct IDatabaseDeleter {
	void operator()(IDatabase* db) const {
		if (db) {
			db->disconnect();
			delete db;
		}
	}
};

class EventLogger
{
public:
	EventLogger(std::shared_ptr<IDatabase> db) 
	{
		if (!db)
		{
			throw std::invalid_argument("Shared pointer to IDatabase is null");
		}
		m_db = std::move(db);
	}

	~EventLogger();
	int	 get_client_id(int client_ip);
	int  add_client(int client_ip);
	int log_event(int client_ip, int event_type, const std::string& event_data);
	bool log_event_response(int event_id, const std::string& response);
	bool delete_events_by_client(int client_id);
	bool delete_events_older_than(int timestamp);
private:

	std::shared_ptr<IDatabase> m_db;

};

#endif // !__EventLogger_hpp__
