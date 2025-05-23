#pragma once
#include <string>
#include "Core/IBodyParser.hpp"
#include <unordered_map>
#include <string_view>
#include <chrono>
#include <iostream>

class MultipartBodyParser : public IBodyParser
{
public:
    bool parse(std::string_view contentType, std::string_view body) override
    {
        auto start = std::chrono::high_resolution_clock::now();
        if (auto it = contentType.find("multipart/form-data"); it == std::string_view::npos || contentType.empty())
        {
            return false;
        }
        else if (boundary = getBoundary(contentType); boundary.empty())
        {
            return false;
        }

        raw_body = std::move(body);
        std::string_view content = raw_body;
        m_parts.clear();
        std::string delimiter = "--" + boundary;
        size_t pos = 0;

        while (true)
        {
            size_t start = content.find(delimiter, pos);
            if (start == std::string_view::npos) break;
            start += delimiter.length();

            // End boundary
            if (content.substr(start, 2) == "--") break;

            // Skip \r\n
            if (content.substr(start, 2) == "\r\n") start += 2;

            size_t end = content.find(delimiter, start);
            if (end == std::string_view::npos) break;

            std::string_view part = content.substr(start, end - start);
            parsePart(part);

            pos = end;
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Parsing took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";
        return !m_parts.empty();
    };

    std::string_view getValue(std::string_view key) const override
    {
        auto it = m_parts.find(std::string(key));
        return (it != m_parts.end()) ? it->second : std::string_view{};
    };

    std::unordered_map<std::string, std::string_view> getAll() const override
    {
        return m_parts;
    };

private:
    std::string raw_body;
    std::string boundary;
    std::unordered_map<std::string, std::string_view> m_parts;
    std::string getBoundary(std::string_view contentType)
    {
        std::string boundary("boundary=");

        if (auto pos = contentType.find(boundary); pos != std::string_view::npos)
        {
            return std::string(contentType.substr(pos + boundary.length()));
        }
        return "";
    }
    void parsePart(std::string_view part)
    {
        size_t header_end = part.find("\r\n\r\n");
        if (header_end == std::string_view::npos) return;

        std::string_view headers = part.substr(0, header_end);
        std::string_view body = part.substr(header_end + 4);

        std::string name;
        std::string filename;
        size_t cd_pos = headers.find("Content-Disposition:");
        if (cd_pos != std::string_view::npos)
        {
            size_t name_pos = headers.find("name=\"", cd_pos);
            if (name_pos != std::string_view::npos)
            {
                name_pos += 6;
                size_t end_quote = headers.find('"', name_pos);
                if (end_quote != std::string_view::npos)
                {
                    name = std::string(headers.substr(name_pos, end_quote - name_pos));
                }
            }

            size_t filename_pos = headers.find("filename=\"", cd_pos);
            if (filename_pos != std::string_view::npos)
            {
                filename_pos += 10;
                size_t end_quote = headers.find('"', filename_pos);
                if (end_quote != std::string_view::npos)
                {
                    filename = std::string(headers.substr(filename_pos, end_quote - filename_pos));
                }
            }
        }

        if (!name.empty()) 
        {
            size_t offset = body.data() - raw_body.data();
            m_parts[name] = std::string_view(raw_body).substr(offset, body.size());
        }
    }
};