#include "IServer.hpp"
#include "IDatabase.hpp"
#include "ILogger.hpp"
#include "asio/asio.hpp"

class AsyncHTTPServer : public IServer {

public:
	AsyncHTTPServer(const std::shared_ptr<IDatabase> iDB, std::shared_ptr<ILogger> iLog) : acceptor_(io_context_), m_db(iDB), m_log(iLog) 
	{
	
		if (!iDB)
		{
			throw std::invalid_argument("Shared pointer to IDatabase is null");
		}

		if (!iLog)
		{
			throw std::invalid_argument("Shared pointer to ILogger is null");
		}

	};
	~AsyncHTTPServer() override {
		if (m_server_running)
		{
			stop();
		}
		else
		{
			m_log->error("Server not running");
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
	ServerConfig m_config{ 8080, 1, 30, false, "", "", "" };
	std::shared_ptr<IDatabase> m_db;
	std::shared_ptr<ILogger> m_log;
	std::atomic<bool> m_server_running{ false };
	asio::io_context io_context_;
	asio::ip::tcp::acceptor acceptor_;
	std::thread m_WaitConnection_thread;
	void handle_client(asio::ip::tcp::socket& socket);

};