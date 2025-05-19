#include "HttpRequest.hpp"
#include <iostream>

void HttpRequest::parseHttp(const std::string_view raw)
{
    auto trim = [](std::string& s)
        {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return !std::isspace(ch);
                }));
            s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                return !std::isspace(ch);
                }).base(), s.end());
        };

    // Split into lines
    size_t pos = 0;
    size_t line_end = raw.find("\r\n");
    if (line_end == std::string::npos)
        return;

    std::string_view request_line = raw.substr(0, line_end);

    // Parse request line
    size_t first_space = request_line.find(' ');
    if (first_space == std::string_view::npos) {
        m_method = std::string(request_line);
    }
    else {
        m_method = std::string(request_line.substr(0, first_space));

        size_t second_space = request_line.find(' ', first_space + 1);
        if (second_space == std::string_view::npos) {
            m_path = std::string(request_line.substr(first_space + 1));
        }
        else {
            m_path = std::string(request_line.substr(first_space + 1, second_space - first_space - 1));
            m_httpVersion = std::string(request_line.substr(second_space + 1));
        }
    }

    // Extract query string
    size_t query_start = m_path.find('?');
    if (query_start != std::string::npos) {
        std::string query_str = m_path.substr(query_start + 1);
        m_path = m_path.substr(0, query_start);

        size_t start = 0;
        while (start < query_str.size()) {
            size_t end = query_str.find('&', start);
            if (end == std::string::npos)
                end = query_str.size();

            std::string pair = query_str.substr(start, end - start);
            size_t eq = pair.find('=');
            if (eq != std::string::npos) {
                m_queryParams[pair.substr(0, eq)] = pair.substr(eq + 1);
            }
            else {
                m_queryParams[pair] = "";
            }

            start = end + 1;
        }
    }

    // Headers
    pos = line_end + 2;
    while (true) {
        line_end = request.find("\r\n", pos);
        if (line_end == std::string_view::npos)
            return;

        if (line_end == pos) {
            pos = line_end + 2; // End of headers
            break;
        }

        std::string_view line = raw.substr(pos, line_end - pos);
        size_t colon = line.find(':');
        if (colon != std::string_view::npos) {
            std::string key = std::string(line.substr(0, colon));
            std::string value = std::string(line.substr(colon + 1));

            trim(key);
            trim(value);

            m_headers[key] = value;
        }

        pos = line_end + 2;
    }

    // Body
    if (pos < raw.size()) {
        m_body = std::string(raw.substr(pos));
    }
};

std::string HttpRequest::getHeader(const std::string &name) const
{
    auto it = m_headers.find(name);
    return it != m_headers.end() ? it->second : "";
};

std::string HttpRequest::getQueryParam(const std::string &name) const
{
    auto it = m_queryParams.find(name);
    return it != m_queryParams.end() ? it->second : "";
}

std::string HttpRequest::getPathParam(const std::string& name) const
{
    return "";
};