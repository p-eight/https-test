#include "SyncHTTPServer.hpp"
#include <iostream>

void SyncHTTPServer::start()
{
	if (m_server_running)
	{
		return;
	}
	try
	{
		asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), m_config.port);
		acceptor_.open(endpoint.protocol());
		acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
		acceptor_.bind(endpoint);
		acceptor_.listen(m_config.max_connections);
		m_server_running = true;
		m_WaitConnection_thread = std::thread([&]()
			{
				while (m_server_running)
				{
					try
					{ 
						asio::ip::tcp::socket socket(io_context_);
						acceptor_.accept(socket);
						handle_client(socket);
					}
					catch (...)
					{
						std::cerr << "Error accepting connection." << std::endl;
					}
				}
				std::cout << "Server stopped accepting connections." << std::endl;
			});
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error starting server: " << e.what() << std::endl;
	}
}

void SyncHTTPServer::stop()
{
	if (!m_server_running)
	{
		return;
	}
	try
	{
		m_server_running = false;
		acceptor_.close();
		io_context_.stop();
		if (m_WaitConnection_thread.joinable())
		{
			m_WaitConnection_thread.join();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error stopping server: " << e.what() << std::endl;
	}
}

void SyncHTTPServer::handle_client(asio::ip::tcp::socket& socket)
{
	try {
		asio::streambuf buffer;
		asio::read_until(socket, buffer, "\r\n\r\n"); // Read HTTP header

		std::istream request_stream(&buffer);
		std::string request_line;
		std::getline(request_stream, request_line);
		std::cout << "Received request from (" << socket.remote_endpoint().address().to_string() << ") : " << request_line << std::endl;

		std::string response =
			"HTTP/1.1 200 OK\r\n"
			"Content-Length: 13\r\n"
			"Connection: close\r\n"
			"\r\n"
			"Hello, world!";

		asio::write(socket, asio::buffer(response));
	}
	catch (std::exception& e) {
		std::cerr << "Error handling client: " << e.what() << std::endl;
	}
}
