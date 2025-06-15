#include "../Core/IConfig.hpp"
#include "../Core/ILogger.hpp"
#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"


class JSONConfig : public IConfig
{
public:
    using json = nlohmann::json;

    static JSONConfig from_file(std::string_view file_path, std::shared_ptr<ILogger> logger = nullptr) 
    {
        if (logger) logger->info("[" __FUNCTION__ "] Loading JSON config from file: {}", file_path);

        std::ifstream file(file_path.data());
        if (!file) throw std::runtime_error("[" __FUNCTION__ "] Failed to open JSON config file");
        if (!file.is_open()) throw std::runtime_error("[" __FUNCTION__ "] File is not open");

        json config = json::parse(file);
        file.close();
        return JSONConfig(config, logger);
    }

    static JSONConfig from_string(const std::string& json_str, std::shared_ptr<ILogger> logger = nullptr) 
    {
        if (logger) logger->info("[" __FUNCTION__ "] Loading JSON config from string");

        json config = json::parse(json_str);
        return JSONConfig(config, logger);
    }   


    std::optional<ConfigValue> get_value(std::string_view value) override 
    {
        auto it = m_config.find(value.data());
        if (it != m_config.end())
        {
            return it->second;
        }
        if (m_logger) m_logger->error("Key {} not found in JSON config", value.data());
        return std::nullopt;
    }
private:
    std::unordered_map<std::string, ConfigValue> m_config;
    std::shared_ptr<ILogger> m_logger;

    JSONConfig(const json& config, std::shared_ptr<ILogger> logger) : m_logger(logger)
    {
        PopulateConfig(config);
    }

    void PopulateConfig(const json& config)
    {
        if (m_logger) m_logger->info("[" __FUNCTION__ "] JSONConfig initialized with path:");
        try
        {
            for (const auto& [key, value] : config.items()) {
                if (value.is_string()) m_config[key] = value.get<std::string>();
                else if (value.is_boolean()) m_config[key] = value.get<bool>();
                else if (value.is_number_integer()) m_config[key] = value.get<int>();
                else if (value.is_number_float()) m_config[key] = value.get<double>();
                else m_config[key] = value.dump(); // fallback: store as JSON string
            }
        }
        catch (const json::parse_error& e)
        {
            if (m_logger) m_logger->error("[" __FUNCTION__ "] Failed to parse JSON config : {}", e.what());
        }
    }
};