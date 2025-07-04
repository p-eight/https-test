#pragma once
#include <iostream>
#include "IServer.hpp"
#include "asio/asio.hpp"
#include "ILogger.hpp"
#include "IClientRepository.hpp"
#include "IEventRepository.hpp"
#include "IClientConnectionRepository.hpp"
#include "IRequestHandler.hpp"
#include "Configurations/ConfigFactory.hpp"

class SyncHTTPServer : public IServer 
{
public:
    SyncHTTPServer(\
		std::shared_ptr<ILogger> _logger, \
		std::shared_ptr<IClientRepository> _client_repo, \
		std::shared_ptr<IClientConnectionRepository> _conn_repo, \
		std::shared_ptr<IRequestHandler> _req_hdlr, \
		std::shared_ptr<IConfig> _config = nullptr) : \
		acceptor_(io_context_), \
		m_logger(_logger), \
		m_client_repo(_client_repo), \
		m_client_conn_repo(_conn_repo), \
        m_req_handler(_req_hdlr), \
		m_config(_config)
   {
      if (!m_logger)
      {
         std::cerr << "ILogger cant be null" << std::endl;
		 throw std::runtime_error("ILogger cant be null");
      }

	  if (!m_client_repo)
	  {
		  m_logger->critical("[" __FUNCTION__ "] IClientRepository cant be null");
	  }

	  if (!m_req_handler)
	  {
		  m_logger->critical("[" __FUNCTION__ "] IRequestHandler cant be null");
	  }
	  
	  if (m_config)
	  {
          if (auto server = m_config->get_as<std::string>("server"))
		  {
			  auto server_config = ConfigFactory::get(*server, true);
			  if (!server)
			  {
                  m_logger->critical("[" __FUNCTION__ "] Server config is not set");
              }
			  else
			  {
				  auto server_type = server_config->get_as<std::string>("type");
				  auto server_port = server_config->get_as<int>("port");
				  auto server_timeout = server_config->get_as<int>("timeout");
				  auto server_max_connections = server_config->get_as<int>("max_connections");

				  if (!server_type || !server_port || !server_timeout || !server_max_connections)
				  {
					  m_logger->critical("[" __FUNCTION__ "] Missing configuration values in IConfig");
				  }
                  else if (server_type != "Http")
                  {
                      m_logger->critical("[" __FUNCTION__ "] Server type is not sync_http");
                  }

				  if (server_port) m_serverconfig.port = *server_port;
				  if (server_max_connections) m_serverconfig.max_connections = *server_max_connections;
				  if (server_timeout) m_serverconfig.timeout = *server_timeout;
			  }
		  }
		  else
		  {
              m_logger->critical("[" __FUNCTION__ "] Server config is not set in IConfig");
		  }
	  }
   };
	~SyncHTTPServer() override {
		if (m_server_running)
		{
			stop();
		}
	};

	void start() override;
	void stop() override;

	bool is_running() const override {
		return m_server_running;
	}

	void configure(const ServerConfig& config) override {
		if (m_server_running)
		{
			stop();
		}
		m_serverconfig = config;
	}

private:
	ServerConfig m_serverconfig{ 8081, 1, 30, false, "", "", "" };
	std::atomic<bool> m_server_running{ false };
	asio::io_context io_context_;
	asio::ip::tcp::acceptor acceptor_;
	std::thread m_WaitConnection_thread;

   std::shared_ptr<ILogger> m_logger;
   std::shared_ptr<IClientRepository> m_client_repo;
   std::shared_ptr<IClientConnectionRepository> m_client_conn_repo;
   std::shared_ptr<IRequestHandler> m_req_handler;
   std::shared_ptr<IConfig> m_config;

	void handle_client(asio::ip::tcp::socket& socket, asio::streambuf& asio_buffer, std::ostringstream& request_data);
};