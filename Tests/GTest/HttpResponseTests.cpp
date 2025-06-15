#include "gtest/gtest.h"
#include "HttpResponse.hpp"

namespace HtppStatusTests
{
    TEST(HttpStatusTest, ConvertsCodeToString) {
        EXPECT_EQ(HttpStatus::to_string(HttpStatus::OK), "OK");
        EXPECT_EQ(HttpStatus::to_string(HttpStatus::NotFound), "Not Found");
        EXPECT_EQ(HttpStatus::to_string(HttpStatus::InternalServerError), "Internal Server Error");
        EXPECT_EQ(HttpStatus::to_string(HttpStatus::BadRequest), "Bad Request");
        EXPECT_EQ(HttpStatus::to_string(HttpStatus::Unauthorized), "Unauthorized");
    }
    TEST(HttpStatusTest, HandlesUnknownCode) {
        EXPECT_EQ(HttpStatus::to_string(static_cast<HttpStatus::Code>(999)), "Unknown Status");
    }
}