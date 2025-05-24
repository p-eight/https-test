#pragma once

#include "Core/IResponse.hpp"

class HttpStatus
{
public:
    enum Code {
        // 1xx Informational
        Continue = 100,
        SwitchingProtocols = 101,

        // 2xx Success
        OK = 200,
        Created = 201,
        Accepted = 202,
        NoContent = 204,

        // 3xx Redirection
        MovedPermanently = 301,
        Found = 302,
        NotModified = 304,

        // 4xx Client Error
        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,

        // 5xx Server Error
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503
    };

    static std::string to_string(Code code)
    {
        switch (code)
        {
            case Continue: return "Continue";
            case SwitchingProtocols: return "Switching Protocols";

            case OK: return "OK";
            case Created: return "Created";
            case Accepted: return "Accepted";
            case NoContent: return "No Content";

            case MovedPermanently: return "Moved Permanently";
            case Found: return "Found";
            case NotModified: return "Not Modified";

            case BadRequest: return "Bad Request";
            case Unauthorized: return "Unauthorized";
            case Forbidden: return "Forbidden";
            case NotFound: return "Not Found";
            case MethodNotAllowed: return "Method Not Allowed";

            case InternalServerError: return "Internal Server Error";
            case NotImplemented: return "Not Implemented";
            case BadGateway: return "Bad Gateway";
            case ServiceUnavailable: return "Service Unavailable";

            default: return "Unknown Status";
        }
    }
};

class HttpResponse : public IResponse
{
public:

    HttpResponse()
        : m_statusCode(HttpStatus::Code::OK), m_reason(HttpStatus::to_string(HttpStatus::Code::OK)) {
    }

    ResponseType type() const override { return ResponseType::Http; }

    void setStatusCode(int code) override { m_statusCode = static_cast<HttpStatus::Code>(code); setReasonPhrase(HttpStatus::to_string(m_statusCode)); }
    void setStatusCode(HttpStatus::Code code) { m_statusCode = code; setReasonPhrase(HttpStatus::to_string(m_statusCode)); }
    void setReasonPhrase(const std::string& reason) override { m_reason = reason; }
    void sethttpVersion(std::string version) { m_httpVersion = version; }
    void setHeader(const std::string& name, const std::string& value) override {
        m_headers[name] = value;
    }

    void setBody(const std::string& body) override {
        m_body = body;
        m_headers["Content-Length"] = std::to_string(body.size());
    }

    std::string_view getBody()
    {
        return m_body;
    }

    HttpStatus::Code getCode()
    {
        return m_statusCode;
    }

    std::string str() const override {
        std::ostringstream oss;
        oss << m_httpVersion << " " << m_statusCode << " " << m_reason << "\r\n";
        for (const auto& [key, value] : m_headers) {
            oss << key << ": " << value << "\r\n";
        }
        oss << "\r\n";
        oss << m_body;
        return oss.str();
    }
private:
    HttpStatus::Code m_statusCode;
    std::string m_reason;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;
    std::string m_httpVersion;
};