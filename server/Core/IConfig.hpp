#pragma once
#include <string>
#include <optional>
#include <variant>

class IConfig
{
public:
    using ConfigValue = std::variant<int, double, bool, std::string>;
    virtual ~IConfig() = default;
    virtual std::optional<ConfigValue> get_value(std::string_view key) = 0;
    
    template<typename T>
    static std::optional<T> extract(const std::optional<ConfigValue>& value) {
        if (value && std::holds_alternative<T>(*value))
            return std::get<T>(*value);
        return std::nullopt;
    }

    template<typename T>
    std::optional<T> get_as(std::string_view key) {
        auto value = this->get_value(key);
        return extract<T>(value);

    }
};