#pragma once
#include <vector>

class IClientRepository
{
public:

	struct client
	{
		int id;
		int ip;
		int connection_count;
		int event_count;
	};

	virtual ~IClientRepository() = default;
	virtual int add_client(int client_ip) = 0;
	virtual bool remove_client(int client_id) = 0;
	virtual bool remove_all_clients() = 0;
	virtual bool increment_connection_count(int client_id) = 0;
	virtual bool increment_event_count(int client_id) = 0;
	virtual client get_client_by_id(int client_id) = 0;
	virtual client get_client_by_ip(int client_ip) = 0;
	virtual int get_client_id_by_ip(int client_ip) = 0;
	virtual int get_client_ip_by_id(int client_id) = 0;
	virtual int get_client_count() = 0;
	virtual std::vector<client> get_all_clients() = 0;
};