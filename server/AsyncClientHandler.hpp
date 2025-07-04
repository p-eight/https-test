#pragma once
#include <asio/asio.hpp>
#include <vector>
#include <memory>
#include "Core/ILogger.hpp"
#include "Core/IRequestHandler.hpp"
#include "RequestFactory.hpp"

class AsyncClientHandler : public std::enable_shared_from_this<AsyncClientHandler>
{
public:
    AsyncClientHandler(std::shared_ptr<ILogger> logger, std::shared_ptr<IRequestHandler> requestHandler, std::shared_ptr<asio::ip::tcp::socket> socket) : \
        m_logger(std::move(logger)), m_requestHandler(std::move(requestHandler)), m_socket(std::move(socket)), m_keepAlive(false), m_buffer(8192)
    {
    };

    ~AsyncClientHandler()
    {
        stop();
    }

    void start()
    {
        auto self = shared_from_this();
        m_logger->info("[" __FUNCTION__ "] Starting connection handling...");
        readData();
    }
    
private:
    std::shared_ptr<ILogger> m_logger;
    std::shared_ptr<IRequestHandler> m_requestHandler;
    std::shared_ptr<asio::ip::tcp::socket> m_socket;
    bool m_keepAlive;
    std::string m_requestData;
    std::vector<char> m_buffer;

    void stop()
    {
        if (m_socket && m_socket->is_open())
        {
            std::error_code ec;
            m_socket->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            m_socket->close(ec);
            m_logger->info("[" __FUNCTION__ "] Connection closed!");
        }
    }

    void readData()
    {
        auto self = shared_from_this();
        
        m_socket->async_read_some(
            asio::buffer(m_buffer),
            [this, self](const std::error_code& ec, std::size_t bytes_transferred)
            {
                handleRead(ec, bytes_transferred);
            });
    }

    void handleRead(const std::error_code& ec, std::size_t bytes_transferred)
    {
        if (!ec)
        {
            //m_logger->info("Received {} bytes from client", bytes_transferred);
            
            // Append received data to request buffer
            m_requestData.append(m_buffer.data(), bytes_transferred);
            
            // Check if we have a complete HTTP request
            if (isCompleteRequest(m_requestData))
            {
                processRequest();
            }
            else
            {
                // Continue reading if request is incomplete
                readData();
            }
        }
        else if (ec == asio::error::eof)
        {
            m_logger->info("Client disconnected normally");
            stop();
        }
        else
        {
            m_logger->error("Read error: {}", ec.message());
            stop();
        }
    }

    bool isCompleteRequest(const std::string& data)
    {
        // Simple HTTP request completion check
        // Look for double CRLF indicating end of headers
        size_t header_end = data.find("\r\n\r\n");
        if (header_end == std::string::npos)
        {
            return false; // Headers not complete
        }

        // Check if we need to read body based on Content-Length
        size_t content_length = getContentLength(data);
        if (content_length > 0)
        {
            size_t body_start = header_end + 4;
            size_t current_body_size = data.length() - body_start;
            return current_body_size >= content_length;
        }

        return true; // No body expected
    }

    size_t getContentLength(const std::string& request)
    {
        std::string content_length_header = "Content-Length: ";
        size_t pos = request.find(content_length_header);
        if (pos != std::string::npos)
        {
            size_t start = pos + content_length_header.length();
            size_t end = request.find("\r\n", start);
            if (end != std::string::npos)
            {
                std::string length_str = request.substr(start, end - start);
                try
                {
                    return std::stoull(length_str);
                }
                catch (...)
                {
                    return 0;
                }
            }
        }
        return 0;
    }

    void processRequest()
    {
        m_logger->debug("Processing request: {}", m_requestData);
        
        // Parse HTTP request to determine if connection should be kept alive
        m_keepAlive = shouldKeepAlive(m_requestData);
        
        auto request = RequestFactory::parse(m_requestData);
        auto response = m_requestHandler->handleRequest(*request);
        // Process request using your request handler
        //std::string response = generateResponse(m_requestData);
        
        // Send response
        sendResponse(response->str());
        
        // Clear request data for next request
        m_requestData.clear();
    }

    bool shouldKeepAlive(const std::string& request)
    {
        // Check Connection header
        if (request.find("Connection: close") != std::string::npos)
        {
            return false;
        }
        
        // HTTP/1.1 defaults to keep-alive
        if (request.find("HTTP/1.1") != std::string::npos)
        {
            return request.find("Connection: close") == std::string::npos;
        }
        
        // HTTP/1.0 defaults to close unless explicitly keep-alive
        return request.find("Connection: keep-alive") != std::string::npos;
    }

    std::string generateResponse(const std::string& request)
    {
        // Use your existing request handler or create a simple response
        std::string body = "Hello, World!";
        std::string connection_header = m_keepAlive ? "keep-alive" : "close";
        
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Length: " << body.length() << "\r\n"
                 << "Connection: " << connection_header << "\r\n"
                 << "Content-Type: text/plain\r\n"
                 << "\r\n"
                 << body;
        
        return response.str();
    }

    void sendResponse(const std::string& response)
    {
        auto self = shared_from_this();
        auto response_buffer = std::make_shared<std::string>(response);
        
        asio::async_write(*m_socket,
            asio::buffer(*response_buffer),
            [this, self, response_buffer](const std::error_code& ec, std::size_t bytes_transferred)
            {
                handleWrite(ec, bytes_transferred);
            });
    }

    void handleWrite(const std::error_code& ec, std::size_t bytes_transferred)
    {
        if (!ec)
        {
            m_logger->info("Response sent successfully ({} bytes)", bytes_transferred);
            
            if (m_keepAlive)
            {
                // Continue reading for next request
                readData();
            }
            else
            {
                // Close connection
                stop();
            }
        }
        else
        {
            m_logger->error("Write error: {}", ec.message());
            stop();
        }
    }


};