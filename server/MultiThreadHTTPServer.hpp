#pragma once
#include <optional>
#include "IServer.hpp"  
#include "ILogger.hpp"  
#include "IClientRepository.hpp"  
#include "IClientConnectionRepository.hpp"  
#include "IEventRepository.hpp"  
#include "IRequestHandler.hpp"  
#include "asio/asio.hpp"  
#include "Configurations/ConfigFactory.hpp"  
#include "VoidConcreteClasses.hpp"  
#include "ThreadPool.hpp"
#include "SimpleThreadPool.hpp"

class MultiThreadHTTPServer : public IServer
{
public:
    class Builder
    {
    public:
        Builder() = default;

        std::shared_ptr<MultiThreadHTTPServer> build();

        Builder& setLogger(std::shared_ptr<ILogger> _logger)
        {
            m_logger = _logger;
            return *this;
        }

        Builder& setRequestHandler(std::shared_ptr<IRequestHandler> _req_handler)
        {
            m_req_handler = _req_handler;
            return *this;
        }
    private:
        std::optional<std::shared_ptr<ILogger>> m_logger;
        std::optional<std::shared_ptr<IClientRepository>> m_client_repo;
        std::optional<std::shared_ptr<IClientConnectionRepository>> m_client_conn_repo;
        std::optional<std::shared_ptr<IEventRepository>> m_event_repo;
        std::optional<std::shared_ptr<IRequestHandler>> m_req_handler;
        std::optional<std::shared_ptr<IConfig>> m_config;
    };


    MultiThreadHTTPServer(
        std::shared_ptr<ILogger> _logger = nullptr,
        std::shared_ptr<IClientRepository> _client_repo = nullptr,
        std::shared_ptr<IClientConnectionRepository> _conn_repo = nullptr,
        std::shared_ptr<IEventRepository> _evt_repo = nullptr,
        std::shared_ptr<IRequestHandler> _req_hdlr = nullptr,
        std::shared_ptr<IConfig> _config = nullptr) :
        acceptor_(io_context_),
        m_logger(_logger),
        m_event_repo(_evt_repo),
        m_client_repo(_client_repo),
        m_client_conn_repo(_conn_repo),
        m_req_handler(_req_hdlr),
        m_config(_config),
        thread_pool(std::thread::hardware_concurrency() * 4)
    {
    };

    ~MultiThreadHTTPServer()
    {
        stop();
    }

    void start() override;
    void stop() override;

    bool is_running() const override {
        return m_server_running;
    }

    void configure(const ServerConfig& config) override {
        if (m_server_running)
        {
            m_logger->info("[" __FUNCTION__ "] - Server is running, stopping before change configurations.");
            stop();
        }
        m_serverconfig = config;
        m_logger->info("[" __FUNCTION__ "] - Restarting server after update configurations.");
    }
private:
    ServerConfig m_serverconfig{ 8081, 4, 30, false, "", "", "" };
    std::atomic<bool> m_server_running{ false };
    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<asio::executor_work_guard<asio::io_context::executor_type>> m_work_guard;
    SimpleThreadPool ioThreadPool;

    ThreadPool thread_pool;

    std::shared_ptr<ILogger> m_logger;
    std::shared_ptr<IClientRepository> m_client_repo;
    std::shared_ptr<IEventRepository> m_event_repo;
    std::shared_ptr<IClientConnectionRepository> m_client_conn_repo;
    std::shared_ptr<IRequestHandler> m_req_handler;
    std::shared_ptr<IConfig> m_config;
    
    void validateDependencies();
    void startAccept();
    void handleAccept(std::shared_ptr<asio::ip::tcp::socket> socket, const std::error_code& error);
    void handleClient(std::shared_ptr<asio::ip::tcp::socket> socket);
};