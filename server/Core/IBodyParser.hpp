#pragma once
#include <string>
#include <string_view>
#include <unordered_map>

class IBodyParser
{
public:
    virtual ~IBodyParser() = default;
    virtual bool parse(std::string_view contentType, std::string_view body) = 0;
    virtual std::string_view getValue(std::string_view key) const = 0;
    virtual std::unordered_map<std::string, std::string_view> getAll() const = 0;
};