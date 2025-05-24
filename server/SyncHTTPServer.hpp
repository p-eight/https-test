#include <iostream>
#include "IServer.hpp"
#include "asio/asio.hpp"
#include "ILogger.hpp"
#include "IClientRepository.hpp"
#include "IEventRepository.hpp"
#include "IClientConnectionRepository.hpp"
#include "IRequestHandler.hpp"

class SyncHTTPServer : public IServer 
{
public:
    SyncHTTPServer(\
		std::shared_ptr<ILogger> _logger, \
		std::shared_ptr<IClientRepository> _client_repo, \
		std::shared_ptr<IRequestHandler> _req_hdlr) : \
		acceptor_(io_context_), \
		m_logger(_logger), \
		m_client_repo(_client_repo), \
		m_req_handler(_req_hdlr)
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
		m_config = config;
	}

private:
	ServerConfig m_config{ 8081, 1, 30, false, "", "", "" };
	std::atomic<bool> m_server_running{ false };
	asio::io_context io_context_;
	asio::ip::tcp::acceptor acceptor_;
	std::thread m_WaitConnection_thread;

   std::shared_ptr<ILogger> m_logger;
   std::shared_ptr<IClientRepository> m_client_repo;
   std::shared_ptr<IRequestHandler> m_req_handler;

	void handle_client(asio::ip::tcp::socket& socket, asio::streambuf& asio_buffer, std::ostringstream& request_data);
};