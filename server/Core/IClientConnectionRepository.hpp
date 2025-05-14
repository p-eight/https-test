#pragma once

class IClientConnectionRepository
{
	struct ClientConnection
	{
		int id;
		int client_id;
		int connection_timestamp;
	};

	virtual ~IClientConnectionRepository() = default;
	virtual int add_client_connection(int client_id, int connection_timestamp) = 0;
	virtual bool remove_client_connection(int connection_id) = 0;
	virtual bool remove_all_client_connections() = 0;
	virtual int get_client_connection_count(int client_id) = 0;
};
