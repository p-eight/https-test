#include "gtest/gtest.h"
#include "Configurations/JSONConfig.hpp"
#include <memory>
#include <filesystem>

namespace JSONConfigTests
{
    class JSONConfigTest : public ::testing::Test
    {
    protected:
        std::unique_ptr<JSONConfig> m_config{};

        void SetUp() override
        {
            std::string json_str = R"({
                "key1": "value1",
                "key2": 42,
                "key3": true
            })";

            m_config = std::make_unique <JSONConfig >(JSONConfig::from_string(json_str));
        };
    };

    class JSONConfigFiles : public ::testing::Test
    {
    protected:
        std::filesystem::path valid_config_file{ "valid_config.json" };
        std::filesystem::path invalid_config_file{ "invalid_config.json" };
        std::filesystem::path nonexistent_config_file{ "nonexistent_config.json" };

        void SetUp() override
        {
            if (!std::filesystem::exists(valid_config_file))
            {
                std::ofstream(valid_config_file) << R"({
                    "key1": "value1",
                    "key2": 42,
                    "key3": true
                })";
            }
            if (!std::filesystem::exists(invalid_config_file))
            {
                std::ofstream(invalid_config_file) << R"({
                    "key1": "value1",
                    "key2": 42,
                    "key3": true
                )";
            }
            if (std::filesystem::exists(nonexistent_config_file))
            {
                std::filesystem::remove(nonexistent_config_file);
            }
        };

        void TearDown() override
        {
            if (std::filesystem::exists(valid_config_file))
            {
                std::filesystem::remove(valid_config_file);
            }
            if (std::filesystem::exists(invalid_config_file))
            {
                std::filesystem::remove(invalid_config_file);
            }
            if (std::filesystem::exists(nonexistent_config_file))
            {
                std::filesystem::remove(nonexistent_config_file);
            }
        };
    };

    TEST(JSONConfigStandalone, DummyTest)
    {
        EXPECT_TRUE(true);
    }

    TEST(JSONConfigStandalone, LoadValidJsonString)
    {
        const std::string json_str = R"({
                "key1": "value1",
                "key2": 42,
                "key3": true
            })";
        auto config = JSONConfig::from_string(json_str);
        EXPECT_TRUE(config.get_value("key1").has_value());
        EXPECT_EQ(*config.get_as<std::string>("key1"), "value1");
    }

    TEST(JSONConfigStandalone, LoadInvalidJsonString)
    {
        const std::string json_str = R"({
                "key1": "value1",
                "key2": 42,
                "key3": true
            )";

        EXPECT_ANY_THROW(auto config = JSONConfig::from_string(json_str));
    }

    TEST_F(JSONConfigFiles, LoadValidFile)
    {
        auto config = JSONConfig::from_file(valid_config_file.string());
        std::string key1 = *config.get_as<std::string>("key1");
        EXPECT_EQ(key1, std::string("value1"));
    }

    TEST_F(JSONConfigFiles, LoadInvalidFile)
    {
        EXPECT_THROW(auto config = JSONConfig::from_file(invalid_config_file.string()), nlohmann::json::exception);
    }

    TEST_F(JSONConfigFiles, LoadNonExistentFile)
    {
        EXPECT_THROW(auto config = JSONConfig::from_file(nonexistent_config_file.string()), std::runtime_error);
    }

    TEST_F(JSONConfigTest, GetExistingKey)
    {
        auto key1 = m_config->get_as<std::string>("key1");
        EXPECT_TRUE(std::nullopt != key1);
        EXPECT_EQ(*key1, "value1");
    }

    TEST_F(JSONConfigTest, GetNonExistentKey)
    {
        auto key1 = m_config->get_as<std::string>("key4");
        EXPECT_TRUE(std::nullopt == key1);
    }

    TEST_F(JSONConfigTest, GetString)
    {
        auto key1 = m_config->get_as<std::string>("key1");
        EXPECT_EQ(*key1, "value1");
    }

    TEST_F(JSONConfigTest, GetInt)
    {
        auto key1 = m_config->get_as<int>("key2");
        EXPECT_EQ(*key1, 42);
    }

    TEST_F(JSONConfigTest, GetBool)
    {
        auto key1 = m_config->get_as<bool>("key3");
        EXPECT_EQ(*key1, true);
    }
}