#pragma once
#include "Core/IRequest.hpp"
#include <unordered_map>
#include <string_view>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class HttpRequest : public IRequest
{
public:

    HttpRequest(const std::string raw_request) : request(raw_request)
    {
        parseHttp(raw_request);
    };

    RequestType type() const override { return RequestType::Http; };
    // Core components
    std::string method() const override {return m_method; };
    std::string uri() const override {return m_path; };
    std::string httpVersion() const override {return m_httpVersion; };

    // Headers / Body
    std::string getHeader(const std::string& name) const override;
    std::string body() const override {return m_body; };

    // Parameters
    std::string getQueryParam(const std::string& name) const override;
    std::string getPathParam(const std::string& name) const override;

    // Metadata
    std::string remoteAddress() const override { return ""; };

    // Command-line form (for non-HTTP input)
    std::string get() const override { return request; };

private:
    std::string request;;
    std::string m_method;
    std::string m_path;
    std::string m_body;
    json m_jsonBody;
    std::string m_httpVersion; 
    std::unordered_map<std::string, std::string> m_headers;
    std::unordered_map<std::string, std::string> m_queryParams;
    // Helper for multipart parsing
    struct FormPart {
        std::unordered_map<std::string, std::string> headers;
        std::string content;
    };

    void parseHttp(const std::string_view raw);
    void parseBody(const std::string_view body_data);
    void parseUrlEncodedForm(const std::string& form_data);
    void parseMultipartForm(const std::string& body, const std::string& boundary);
    static std::string urlDecode(const std::string& str);
};