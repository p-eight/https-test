#pragma once
#include <memory>
#include "../Core/IConfig.hpp"
#include "JSONConfig.hpp"
#include "XMLConfig.hpp"

class ConfigFactory
{
public:
    static std::shared_ptr<IConfig> get(std::string_view source, bool is_raw = false) {
        if (is_raw) {
            return get_from_string(source);
        }
        else {
            return get_from_file(source);
        }
    }
private:
    static std::shared_ptr<IConfig> get_from_file(std::string_view path)
    {
        if (path.ends_with(".json")) {
            return std::make_shared<JSONConfig>(JSONConfig::from_file(path));
        }
        else if (path.ends_with(".xml")) {
            return std::make_shared<XMLConfig>(XMLConfig::from_file(path));
        }
        throw std::runtime_error("Unsupported config file type: " + std::string(path));
    }

    static std::shared_ptr<IConfig> get_from_string(std::string_view raw)
    {
        if (isJson(raw)) {
            return std::make_shared<JSONConfig>(JSONConfig::from_string(std::string(raw)));
        }
        else if (raw.starts_with("<") && raw.ends_with(">")) {
            return std::make_shared<XMLConfig>(XMLConfig::from_string(std::string(raw)));
        }
        throw std::runtime_error("String type not recocnized : " + std::string(raw));
    }

    static bool isJson(std::string_view str) 
    {
        return !str.empty() && \
            ((str.starts_with('[') && str.ends_with(']')) || \
             (str.starts_with("{") && str.ends_with("}")));
    }
};