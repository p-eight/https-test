#include "MultiThreadHTTPServer.hpp"
#include "AsyncClientHandler.hpp"


std::shared_ptr<MultiThreadHTTPServer> MultiThreadHTTPServer::Builder::build()
{
    return std::make_shared<MultiThreadHTTPServer>(
        m_logger.value_or(std::make_shared<VoidLogger>()),
        m_client_repo.value_or(std::make_shared<VoidClientRepository>()),
        m_client_conn_repo.value_or(std::make_shared<VoidClientConnectionRepository>()),
        m_event_repo.value_or(std::make_shared<VoidEventRepository>()),
        m_req_handler.value_or(std::make_shared<VoidRequestHandler>()),
        m_config.value_or(std::make_shared<VoidConfig>())
    );
}

void MultiThreadHTTPServer::stop()
{
    if (!is_running())
    {
        m_logger->error("[" __FUNCTION__ "] Server is already stopped.");
        return;
    }
    m_server_running = false;
    acceptor_.close();
    if (m_work_guard)
    {
        auto ptr = m_work_guard.get();
        ptr->reset();
        m_work_guard.reset();
    }
    io_context_.stop();
    ioThreadPool.join_all();
}

void MultiThreadHTTPServer::start()  
{  
    if (m_server_running)  
    {  
        m_logger->error("[" __FUNCTION__ "] Server is already running.");  
        return;  
    }  

    validateDependencies();  

    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), m_serverconfig.port);  
    acceptor_.open(endpoint.protocol());  
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));  
    acceptor_.bind(endpoint);  
    acceptor_.listen(m_serverconfig.max_connections);  

    m_server_running = true;  
    m_logger->info("[" __FUNCTION__ "] Server starting on port {}", m_serverconfig.port);  

    // Fix: Use std::optional to manage m_work_guard initialization  
    m_work_guard = std::make_shared< asio::executor_work_guard<asio::io_context::executor_type>>(asio::make_work_guard(io_context_));
    startAccept();  

    const size_t thread_count = std::max(2u, std::thread::hardware_concurrency() * 4);  

    for (size_t i = 0; i < thread_count; ++i)  
    {  
        ioThreadPool.emplace_back([this, i]()  
        {  
            try  
            {  
                m_logger->trace("[" __FUNCTION__ "] IO thread {} started", i);  
                io_context_.run();  
                m_logger->trace("[" __FUNCTION__ "] IO thread {} finished", i);  
            }  
            catch (const std::exception& e)  
            {  
                m_logger->error("[" __FUNCTION__ "] IO thread {} exception: {}", i, e.what());  
            }  
        });  
    }  

    m_logger->info("[" __FUNCTION__ "] Server started with {} IO threads", thread_count);  
}  


void MultiThreadHTTPServer::validateDependencies()
{
    if (!m_logger)
    {
        throw std::runtime_error("Logger is not set.");
    }
    if (!m_client_repo)
    {
        throw std::runtime_error("Client repository is not set.");
    }
    if (!m_client_conn_repo)
    {
        throw std::runtime_error("Client connection repository is not set.");
    }
    if (!m_event_repo)
    {
        throw std::runtime_error("Event repository is not set.");
    }
    if (!m_req_handler)
    {
        throw std::runtime_error("Request handler is not set.");
    }
    if (!m_config)
    {
        throw std::runtime_error("Configuration is not set.");
    }
}


void MultiThreadHTTPServer::startAccept()
{
    auto socket = std::make_shared<asio::ip::tcp::socket>(io_context_);
    acceptor_.async_accept(*socket, \
        [this, socket](const auto& error)
        {
            handleAccept(socket, error);
        });
}

void MultiThreadHTTPServer::handleAccept(std::shared_ptr<asio::ip::tcp::socket> socket, const std::error_code& error)
{
    if (!m_server_running.load())
    {
        m_logger->error("[" __FUNCTION__ "] Server is not running.");
        return;
    }

    if (!error)
    {
        m_logger->info("[" __FUNCTION__ "] New connection accepted from: {}", socket->remote_endpoint().address().to_string());
        
        asio::post(io_context_, \
            [this, socket]() 
            { 
                handleClient(socket); 
            });
        startAccept();
    }
    else if (error)
    {
        m_logger->error("[" __FUNCTION__ "] Error occurred during accept: " + error.message());
        if (asio::error::operation_aborted != error)
        {
            startAccept();
        }
        else
        {
            m_logger->error("[" __FUNCTION__ "] Unexpected error occurred");
        }
    }
}

void MultiThreadHTTPServer::handleClient(std::shared_ptr<asio::ip::tcp::socket> socket)
{

    try
    {
        std::thread::id this_id = std::this_thread::get_id();
        std::size_t thread_num = std::hash<std::thread::id>{}(this_id);
        m_logger->info("[" __FUNCTION__ "] Processing client", thread_num);

        auto client = std::make_shared<AsyncClientHandler>(m_logger, m_req_handler, socket);
        client->start();
    }
    catch (const std::exception& e)
    {
        m_logger->error("Exception in handleClient: {}", e.what());
    }
}