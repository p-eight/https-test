#ifndef __IDatabase_hpp__
#define __IDatabase_hpp__
#include <string>
#include <vector>

struct client
{
	int id;
	int ip;
	int connection_count;
	int event_count;
};

struct event
{
	int id;
	int client_id;
	int event_type;
	int event_time;
	std::string event_data;
	std::string event_response;
};

struct sys_event
{
	int id;
	int event_time;
	std::string event_data;
};

class  IDatabase
{
public:

	virtual ~IDatabase() = default;
	virtual bool connect(const std::string& connection_string) = 0;
	virtual void disconnect() = 0;
	virtual int add_client(int client_ip) = 0;
	virtual bool remove_client(int client_id) = 0;
	virtual bool update_client(const client& updated_client) = 0;
	virtual client get_client_by_id(int client_id) = 0;
	virtual client get_client_by_ip(int client_ip) = 0;
	virtual int get_client_id_by_ip(int client_ip) = 0;
	virtual int get_client_ip_by_id(int client_id) = 0;
	virtual int add_event(const event& new_event) = 0;
	virtual bool remove_event(int event_id) = 0;
	virtual bool update_event(const event& updated_event) = 0;
	virtual bool set_event_response(int event_id, const std::string& response) = 0;
	virtual bool delete_events_by_client(int client_id) = 0;
	virtual bool delete_events_older_than(int timestamp) = 0;
	virtual event get_event(int event_id) = 0;
	virtual std::vector<event> get_events_by_client(int client_id) = 0;
	virtual std::vector<event> get_events_newer_than(int timestamp) = 0;
};

#endif // !__IDatabase_hpp__
