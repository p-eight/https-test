#include "SyncHTTPServer.hpp"
#include <iostream>
#include "RequestFactory.hpp"


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
						m_logger->error("Error accepting connection.");
					}
				}
				m_logger->info("Server stopped accepting connections.");
			});
	}
	catch (const std::exception& e)
	{
		m_logger->error("Error starting server: {}", e.what());
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
		m_logger->error("Error stopping server: {}", e.what());
	}
}

static std::string read_full_request(asio::ip::tcp::socket& socket)
{
	asio::streambuf buffer;

	// Read headers
	asio::read_until(socket, buffer, "\r\n\r\n");

	std::istream request_stream(&buffer);
	std::string line;
	std::ostringstream request_data;

	// Read header lines
	while (std::getline(request_stream, line) && line != "\r") {
		request_data << line << "\n";
	}
	request_data << "\r\n"; // final empty line

	// Parse headers to find Content-Length
	std::string headers = request_data.str();
	std::istringstream header_stream(headers);
	size_t content_length = 0;

	while (std::getline(header_stream, line)) {
		auto pos = line.find(':');
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			// Remove possible leading/trailing whitespace
			key.erase(0, key.find_first_not_of(" \t"));
			key.erase(key.find_last_not_of(" \t\r\n") + 1);
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t\r\n") + 1);

			if (key == "Content-Length") {
				content_length = std::stoul(value);
				break;
			}
		}
	}

	// Read body if necessary
	if (content_length > 0) {
		std::vector<char> body_buf(content_length);
		asio::read(socket, asio::buffer(body_buf));
		request_data.write(body_buf.data(), content_length);
	}

	return request_data.str();
}


void SyncHTTPServer::handle_client(asio::ip::tcp::socket& socket)
{
	try {
		std::string raw_request = read_full_request(socket);

        auto request = RequestFactory::parse(raw_request);
        
		std::string response =
			"HTTP/1.1 200 OK\r\n"
			"Content-Length: 13\r\n"
			"Connection: close\r\n"
			"\r\n"
			"Hello, world!";

		asio::write(socket, asio::buffer(response));
	}
	catch (std::exception& e) 
	{
		m_logger->error("Error handling client : {}", e.what());
	}
}
