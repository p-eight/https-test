#pragma once

#include "Core/IRequestHandler.hpp"
#include <unordered_map>
#include <functional>
#include "ILogger.hpp"
#include <chrono>

class RequestRouter : public IRequestHandler
{
public:
    using HandlerPtr = std::shared_ptr<IRequestHandler>;
    using Key = std::pair<std::string, std::string>; // (method, path)

    RequestRouter(std::shared_ptr<ILogger> _logger) : m_logger(_logger) {}

    void registerHandler(const std::string& method, const std::string& path, std::shared_ptr<IRequestHandler> handler) 
    {
        m_logger->info("[" __FUNCTION__ "] Registering handler for {} {}", method, path);
        m_routes[{toUpper(method), path}] = std::move(handler);
    }

    std::unique_ptr<IResponse> handleRequest(const IRequest& request) override 
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        Key key{toUpper(request.method()), request.uri()};
        
        if (auto it = m_routes.find(key); it != m_routes.end())
        {
            auto returned = it->second->handleRequest(request);
            end = std::chrono::high_resolution_clock::now();
            m_logger->info("[" __FUNCTION__ "] Handled \'{}\' \'{}\' in {} ms", request.method(), request.uri(), std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
            return returned;
        }
        m_logger->error("[" __FUNCTION__ "] No handler found for \'{}\' \'{}\'", request.method(), request.uri());
        return nullptr;
    }

private:
    std::shared_ptr<ILogger> m_logger;
    struct pair_hash {
        std::size_t operator()(const Key& p) const {
            return std::hash<std::string>()(p.first) ^ std::hash<std::string>()(p.second);
        }
    };
    std::unordered_map<Key, HandlerPtr, pair_hash> m_routes;

    static std::string toUpper(const std::string& str) {
        std::string upper = str;
        for (auto& c : upper) c = toupper(c);
        return upper;
    }
};
