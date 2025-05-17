#pragma once

#include "Core/IRequestHandler.hpp"
#include <unordered_map>
#include <functional>

class ResquestRouter : IRequestHandler
{
public:
    using HandlerPtr = std::shared_ptr<IRequestHandler>;
    using Key = std::pair<std::string, std::string>; // (method, path)

    void registerHandler(const std::string& method, const std::string& path, std::shared_ptr<IRequestHandler> handler) {
        m_routes[{toUpper(method), path}] = std::move(handler);
    }

    std::unique_ptr<IResponse> handleRequest(const IRequest& request) override {
        Key key{toUpper(request.method()), request.uri()};
        auto it = m_routes.find(key);
        if (it != m_routes.end()) {
            return it->second->handleRequest(request);
        }
        //return std::make_unique<IResponse>();
        return it->second->handleRequest(request);
    }

private:

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
