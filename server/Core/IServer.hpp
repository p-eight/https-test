#pragma once
#include <functional>
#include <string>

class IServer {
public:

    struct ServerConfig {
        int port;
        int max_connections;
        int timeout;
        bool use_ssl;
        std::string cert_file;
        std::string key_file;
        std::string dh_file;
    };

    virtual ~IServer() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool is_running() const = 0;
	virtual void configure(const ServerConfig& config) = 0;
};
