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
        //m_body = std::string(raw.substr(pos));
        parseBody(raw.substr(pos));
        std::cout << "Body: " << m_jsonBody << std::endl;
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

void HttpRequest::parseBody(const std::string_view body_data)
{
    m_body = std::string(body_data);

    if (m_headers.count("Content-Length"))
    {
        size_t content_length = std::stoul(m_headers.at("Content-Length"));
        if (m_body.size() > content_length) 
        {
            m_body.resize(content_length);
        }
    }

    if (m_headers.count("Content-Type"))
    {
        std::string content_type = m_headers.at("Content-Type");
        if (content_type == "application/x-www-form-urlencoded")
        {
            parseUrlEncodedForm(m_body);
        }
        else if (content_type.find("multipart/form-data") != std::string::npos)
        {
            size_t boundary_start = content_type.find("boundary=");
            if (boundary_start != std::string::npos)
            {
                std::string boundary = content_type.substr(boundary_start + 9);
                parseMultipartForm(m_body, boundary);
            }
        }
    }
}
void HttpRequest::parseUrlEncodedForm(const std::string& form_data)
{
}
void HttpRequest::parseMultipartForm(const std::string& body, const std::string& boundary)
{
    m_jsonBody = json::array();
    std::string delimiter = "--" + boundary;
    std::string end_delimiter = delimiter + "--";
    size_t pos = 0;

    // Find first boundary
    pos = body.find(delimiter, pos);
    if (pos == std::string::npos) return;
    pos += delimiter.length();

    while (pos < body.size())
    {
        // Check for end boundary
        if (body.compare(pos, end_delimiter.length(), end_delimiter) == 0) break;
        // Skip CRLF after boundary
        if (body[pos] == '\r') pos++;
        if (body[pos] == '\n') pos++;

        nlohmann::json part = {
                {"headers", nlohmann::json::object()},
                {"content", ""},
                {"name", ""},
                {"filename", ""},
                {"json", nullptr} // Will be populated if content is JSON
        };

        // Parse headers
        size_t header_end = body.find("\r\n\r\n", pos);
        if (header_end == std::string::npos) break;

        std::string headers_str = body.substr(pos, header_end - pos);
        std::istringstream headers_stream(headers_str);
        std::string header_line;

        while (std::getline(headers_stream, header_line) && header_line != "\r") {
            size_t colon = header_line.find(':');
            if (colon != std::string::npos) {
                std::string key = header_line.substr(0, colon);
                std::string value = header_line.substr(colon + 1);

                // Trim whitespace
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                part["headers"][key] = value;

                // Parse Content-Disposition
                if (key == "Content-Disposition") {
                    size_t name_pos = value.find("name=\"");
                    if (name_pos != std::string::npos) {
                        name_pos += 6;
                        size_t name_end = value.find("\"", name_pos);
                        part["name"] = value.substr(name_pos, name_end - name_pos);
                    }

                    size_t filename_pos = value.find("filename=\"");
                    if (filename_pos != std::string::npos) {
                        filename_pos += 10;
                        size_t filename_end = value.find("\"", filename_pos);
                        part["filename"] = value.substr(filename_pos, filename_end - filename_pos);
                    }
                }
            }
        }

        // Parse content
        pos = header_end + 4; // Skip "\r\n\r\n"
        size_t part_end = body.find("\r\n" + delimiter, pos);
        if (part_end == std::string::npos) break;

        part["content"] = body.substr(pos, part_end - pos);

        // Auto-parse JSON content
        if (part["headers"].contains("Content-Type") &&
            part["headers"]["Content-Type"].get<std::string>().find("application/json") != std::string::npos) {
            try {
                part["json"] = nlohmann::json::parse(part["content"].get<std::string>());
            }
            catch (...) {
                // Invalid JSON remains nullptr
            }
        }

        m_jsonBody.push_back(part);
        pos = part_end;
    }
}
std::string HttpRequest::urlDecode(const std::string& str)
{
    return std::string();
}
;