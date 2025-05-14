#include "IServer.hpp"
#include "asio/asio.hpp"

class SyncHTTPServer : public IServer {
public:
	SyncHTTPServer() : acceptor_(io_context_) {};
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
	ServerConfig m_config{ 8080, 1, 30, false, "", "", "" };
	std::atomic<bool> m_server_running{ false };
	asio::io_context io_context_;
	asio::ip::tcp::acceptor acceptor_;
	std::thread m_WaitConnection_thread;

	void handle_client(asio::ip::tcp::socket& socket);
};