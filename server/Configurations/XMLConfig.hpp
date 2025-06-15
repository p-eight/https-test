#include "../Core/IConfig.hpp"
#include "../Core/ILogger.hpp"

class XMLConfig : public IConfig
{
public:
    XMLConfig() = default;
    static XMLConfig from_file(const std::filesystem::path& file_path)
    {
        // Implement XML file loading logic here
        return XMLConfig();
    }
    static XMLConfig from_string(const std::string& xml_str)
    {
        // Implement XML string parsing logic here
        return XMLConfig();
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
};