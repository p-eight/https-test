#include "SyncHTTPServer.hpp"
#include <iostream>
#include "RequestFactory.hpp"
#include "HttpResponse.hpp"
#include <chrono>

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
						if (m_server_running)
						{
							handle_client(socket);
						}
						else
						{
							break;
						}
					}
					catch (const asio::system_error& e)
					{
						if (e.code() == asio::error::operation_aborted)
						{
							// Expected when acceptor is closed
							m_logger->info("Accept operation cancelled");
						}
						else
						{
							m_logger->error("Error accepting connection: {}", e.what());
						}
					}
					catch (const std::exception& e)
					{
						m_logger->error("Error accepting connection : {}", e.what());
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
		asio::error_code ec;
		acceptor_.close(ec);
		if (ec) 
		{
			m_logger->error("Error closing acceptor: {}", ec.message());
		}
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

	// Step 1: Read headers
	asio::read_until(socket, buffer, "\r\n\r\n");

	std::istream request_stream(&buffer);
	std::string line;
	std::ostringstream request_data;

	// Read headers into request_data
	while (std::getline(request_stream, line) && line != "\r") {
		request_data << line << "\n";
	}
	request_data << "\r\n"; // End of headers

	// Step 2: Parse headers to find Content-Length
	std::string headers = request_data.str();
	std::istringstream header_stream(headers);
	size_t content_length = 0;

	while (std::getline(header_stream, line)) {
		auto pos = line.find(':');
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
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

	// Step 3: Check how much of the body is already in the buffer
	size_t already_buffered = buffer.size();

	std::string body;
	if (content_length > 0) {
		// Read already-buffered part
		std::vector<char> temp(already_buffered);
		request_stream.read(temp.data(), already_buffered);
		body.append(temp.data(), request_stream.gcount());

		// Read the remaining body from socket (if any)
		size_t remaining = content_length > body.size() ? content_length - body.size() : 0;
		if (remaining > 0) {
			std::vector<char> body_buf(remaining);
			asio::read(socket, asio::buffer(body_buf));
			body.append(body_buf.data(), remaining);
		}

		request_data << body;
	}

	return request_data.str();
}


void SyncHTTPServer::handle_client(asio::ip::tcp::socket& socket)
{
	try {
		
		std::string raw_request = read_full_request(socket);

        auto request = RequestFactory::parse(raw_request);
        
        auto res = m_req_handler->handleRequest(*request);

		asio::write(socket, asio::buffer(res->str()));
	}
	catch (std::exception& e) 
	{
		m_logger->error("Error handling client : {}", e.what());
	}
}
