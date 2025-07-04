#pragma once  
#include "IDatabase.hpp"  
#include "IClientRepository.hpp"  
#include "IClientConnectionRepository.hpp"  
#include "ILogger.hpp"  
#include "IEventRepository.hpp"
#include "IRequestHandler.hpp"
#include "IResponse.hpp"

class VoidDatabase : public IDatabase  
{  
public:  
   bool connect(const std::string& connection_string) override { return true; };  
   void disconnect() override {};  
};  

class VoidClientRepository : public IClientRepository  
{  
public:  
   int add_client(int client_ip) override { return 0; }  
   bool remove_client(int client_id) override { return true; }  
   bool remove_all_clients() override { return true; }  
   bool increment_connection_count(int client_id) override { return true; }  
   bool increment_event_count(int client_id) override { return true; }  
   client get_client_by_id(int client_id) override { return client{}; }  
   client get_client_by_ip(int client_ip) override { return client{}; }  
   int get_client_id_by_ip(int client_ip) override { return 0; }  
   int get_client_ip_by_id(int client_id) override { return 0; }  
   int get_client_count() override { return 0; }  
   std::vector<client> get_all_clients() override { return std::vector<client>{}; }  
};  

class VoidEventRepository : public IEventRepository  
{  
public:  
   int add_event(int client_id, int connection_id, int timestamp, const std::string& event_data) override { return 0; }  
   bool remove_event(int event_id) override { return true; }  
   bool remove_all_events() override { return true; }  
   bool remove_events_by_client(int client_id) override { return true; }  
   std::vector<Event> get_events_by_client(int client_id) override { return std::vector<Event>(); }  
};  

class VoidClientConnectionRepository : public IClientConnectionRepository
{
public:
	int add_client_connection(int client_id, int connection_timestamp) override { return 0;};
	bool remove_connection(int connection_id) override { return true;};
	bool remove_client_connections(int client_id) override { return true;};
	bool remove_all_connections() override { return true;};
	int get_client_connection_count(int client_id) override { return 0;};
};

class VoidLogger : public ILogger  
{  
public:  
   void log(LogLevel level, const std::string& message) override {}  
   void trace(const std::string& message) override {}  
   void debug(const std::string& message) override {}  
   void info(const std::string& message) override {}  
   void error(const std::string& message) override {}  
   void critical(const std::string& message) override {}  
};

class VoidResponse : public IResponse
{
    ResponseType type() const override { return ResponseType::Unknown;};
    void setStatusCode(int code) override { return;};
    void setReasonPhrase(const std::string& reason) override { return; };
    void setHeader(const std::string& name, const std::string& value) override { return ;};
    void setBody(const std::string& body) override { return ;};
    std::string str() const override { return ""; };
};

class VoidRequestHandler : public IRequestHandler
{
public:
	std::unique_ptr<IResponse> handleRequest(const IRequest& request) override { return std::make_unique<VoidResponse>(); };
};

class VoidConfig : public IConfig
{
public:
	std::optional<ConfigValue> get_value(std::string_view key) override { return std::nullopt;};
};