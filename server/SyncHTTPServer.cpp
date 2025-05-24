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
				asio::streambuf asio_buffer;
				std::ostringstream data;
				while (m_server_running)
				{
					try
					{ 
						asio::ip::tcp::socket socket(io_context_); 
						asio::ip::tcp::no_delay option(true);
						//socket.set_option(option);
						acceptor_.accept(socket);
						if (m_server_running)
						{
                            m_logger->info("[Conn Thread] Accepted connection from {}", socket.remote_endpoint().address().to_string());
							handle_client(socket, asio_buffer, data);
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
							m_logger->info("[Conn Thread] Accept operation cancelled");
						}
						else
						{
							m_logger->error("[Conn Thread] Error accepting connection: {}", e.what());
						}
					}
					catch (const std::exception& e)
					{
						m_logger->error("[Conn Thread] Error accepting connection : {}", e.what());
					}
				}
				m_logger->info("[Conn Thread] Server stopped accepting connections.");
			});
	}
	catch (const std::exception& e)
	{
		m_logger->error("[" __FUNCTION__ "] Error starting server: {}", e.what());
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
			m_logger->error("[" __FUNCTION__ "] Error closing acceptor: {}", ec.message());
		}
		io_context_.stop();
		if (m_WaitConnection_thread.joinable())
		{
			m_WaitConnection_thread.join();
		}
	}
	catch (const std::exception& e)
	{
		m_logger->error("[" __FUNCTION__ "] Error stopping server: {}", e.what());
	}
}

static void read_full_request(asio::ip::tcp::socket& socket, asio::streambuf &buffer, std::string& request_data)
{
	buffer.consume(buffer.size());
	request_data.clear();

	// Read headers
	asio::read_until(socket, buffer, "\r\n\r\n");

	std::istream request_stream(&buffer);
	std::string line;

	// Append headers directly to request_data
	std::ostringstream header_stream;
	while (std::getline(request_stream, line) && line != "\r") {
		header_stream << line << "\n";
	}
	header_stream << "\r\n";
	request_data += header_stream.str();

	// Step 2: Parse headers to find Content-Length
	size_t content_length = 0;

	auto pos = request_data.find("Content-Length:");
	if (pos != std::string::npos) {
		auto end_of_line = request_data.find('\n', pos);
		if (end_of_line != std::string::npos) {
			std::string value = request_data.substr(pos + 15, end_of_line - pos - 15);
			// Trim whitespace
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t\r\n") + 1);
			content_length = std::stoul(value);
		}
	}

	// Step 3: Check how much of the body is already in the buffer
	size_t already_buffered = buffer.size();
	if (content_length > 0) {
		std::vector<char> temp(already_buffered);
		request_stream.read(temp.data(), already_buffered);
		request_data.append(temp.data(), request_stream.gcount());

		size_t remaining = content_length > (request_data.size() - header_stream.str().size()) ?
			content_length - (request_data.size() - header_stream.str().size()) : 0;

		if (remaining > 0) {
			std::vector<char> body_buf(remaining);
			asio::read(socket, asio::buffer(body_buf));
			request_data.append(body_buf.data(), remaining);
		}
	}
}


void SyncHTTPServer::handle_client(asio::ip::tcp::socket& socket, asio::streambuf& asio_buffer, std::ostringstream& request_data)
{
	try {
        auto start = std::chrono::high_resolution_clock::now();
		auto begin = start;

		std::string request_data;
		read_full_request(socket, asio_buffer, request_data);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        m_logger->info("[" __FUNCTION__ "] Read request in {} ms", duration.count());

		start = std::chrono::high_resolution_clock::now();

        auto request = RequestFactory::parse(request_data);
		
		end = std::chrono::high_resolution_clock::now(); 		
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_logger->info("[" __FUNCTION__ "] Request parsed in {} ms", duration.count());

		start = std::chrono::high_resolution_clock::now();

        auto res = m_req_handler->handleRequest(*request);

		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_logger->info("[" __FUNCTION__ "] Request handled in {} ms", duration.count());

		if (res)
		{
			asio::write(socket, asio::buffer(res->str()));
		}
		else
		{
			m_logger->error("[" __FUNCTION__ "] Error handling request ");
		}
        end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin); 
		m_logger->info("[" __FUNCTION__ "] Total time: {} ms", duration.count());

	}
	catch (std::exception& e) 
	{
		m_logger->error("[" __FUNCTION__ "] Error handling client : {}", e.what());
	}
}
