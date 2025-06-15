#include "gtest/gtest.h"
#include "Configurations/ConfigFactory.hpp"

namespace ConfigFactoryTests
{
    TEST(ConfigFactory, DummyTest)
    {
        EXPECT_TRUE(true);
    }
    TEST(ConfigFactory, GetFromFile_JSON)
    {
        auto config = ConfigFactory::get("D:\\repos\\ph\\https-test\\Tests\\GTest\\config.json");
        EXPECT_TRUE(config->get_value("key1").has_value());
    }
    TEST(ConfigFactory, GetFromFile_XML)
    {
        EXPECT_TRUE(true);
    }
    TEST(ConfigFactory, GetFromString_JSON)
    {
        std::string json_str = R"({"key1": "value1", "key2": 42})";
        auto config = ConfigFactory::get(json_str, true);
        EXPECT_TRUE(config->get_value("key1").has_value());
    }
    TEST(ConfigFactory, GetFromString_XML)
    {
        EXPECT_TRUE(true);
    }
    TEST(ConfigFactory, UnsupportedFileType)
    {
        EXPECT_THROW(ConfigFactory::get("unsupported.txt"), std::runtime_error);
    }
}