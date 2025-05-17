#pragma once
#include <string>

class IResponse
{
public:
    enum class ResponseType {
        Http,
        Grpc,
        Command,
        Unknown
    };
    virtual ~IResponse() = default;

    virtual ResponseType type() const = 0;

    // Core components
    virtual void setStatusCode(int code) = 0;
    virtual void setReasonPhrase(const std::string& reason) = 0;

    // Headers
    virtual void setHeader(const std::string& name, const std::string& value) = 0;

    // Body
    virtual void setBody(const std::string& body) = 0;

    // Serialize response for sending
    virtual std::string str() const = 0;
};