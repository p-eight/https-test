#include "gtest/gtest.h"
#include "HttpRequest.hpp"

namespace HttpRequestTests
{
    TEST(HttpRequestTest, ParsesRequestLineCorrectly) {
        HttpRequest req("GET /users?id=42 HTTP/1.1\r\n\r\n");

        EXPECT_EQ(req.method(), "GET");
        EXPECT_EQ(req.uri(), "/users");
        EXPECT_EQ(req.httpVersion(), "HTTP/1.1");
    }

    TEST(HttpRequestTest, ParsesHeadersCorrectly) {
        std::string raw =
            "POST /submit HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 19\r\n"
            "\r\n"
            "{\"a\":1,\"b\":true}";

        HttpRequest req(raw);

        EXPECT_EQ(req.getHeader("Host"), "localhost");
        EXPECT_EQ(req.getHeader("Content-Type"), "application/json");
        EXPECT_EQ(req.getHeader("Content-Length"), "19");
    }

    TEST(HttpRequestTest, ParsesBodyCorrectly) {
        std::string raw =
            "POST /data HTTP/1.1\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello=World!";

        HttpRequest req(raw);
        EXPECT_EQ(req.body(), "Hello=World!");
    }

    TEST(HttpRequestTest, ParsesQueryParamsCorrectly) {
        HttpRequest req("GET /search?q=test&lang=en HTTP/1.1\r\n\r\n");

        EXPECT_EQ(req.getQueryParam("q"), "test");
        EXPECT_EQ(req.getQueryParam("lang"), "en");
        EXPECT_EQ(req.getQueryParam("missing"), "");
    }

    TEST(HttpRequestTest, GetReturnsRawRequest) {
        std::string raw = "GET /something HTTP/1.1\r\n\r\n";
        HttpRequest req(raw);

        EXPECT_EQ(req.get(), raw);
    }

    TEST(HttpRequestTest, HandlesEmptyRequest) {
        HttpRequest req("");

        EXPECT_EQ(req.method(), "");
        EXPECT_EQ(req.uri(), "");
        EXPECT_EQ(req.httpVersion(), "");
        EXPECT_TRUE(req.body().empty());
    }

    TEST(HttpRequestTest, HandlesRequestLineWithoutVersion) {
        HttpRequest req("GET /pathOnly\r\n\r\n");

        EXPECT_EQ(req.method(), "GET");
        EXPECT_EQ(req.uri(), "/pathOnly");
        EXPECT_EQ(req.httpVersion(), "");
    }

    TEST(HttpRequestTest, HandlesRequestLineWithoutUri) {
        HttpRequest req("GET\r\n\r\n");

        EXPECT_EQ(req.method(), "GET");
        EXPECT_EQ(req.uri(), "");
        EXPECT_EQ(req.httpVersion(), "");
    }

    TEST(HttpRequestTest, HandlesHeadersWithoutColon) {
        std::string raw =
            "GET / HTTP/1.1\r\n"
            "Host localhost\r\n"   // invalid header line (missing colon)
            "Content-Length 5\r\n" // invalid header line (missing colon)
            "\r\n"
            "body";

        HttpRequest req(raw);

        EXPECT_EQ(req.getHeader("Host"), "");
        EXPECT_EQ(req.getHeader("Content-Length"), "");
        EXPECT_EQ(req.body(), "body");
    }

    TEST(HttpRequestTest, HandlesHeadersWithExtraSpaces) {
        std::string raw =
            "GET / HTTP/1.1\r\n"
            "Host:    example.com\r\n"
            "X-Custom:  custom value \r\n"
            "\r\n";

        HttpRequest req(raw);

        EXPECT_EQ(req.getHeader("Host"), "example.com");
        EXPECT_EQ(req.getHeader("X-Custom"), "custom value");
    }

    TEST(HttpRequestTest, HandlesBodyWithoutContentLength) {
        std::string raw =
            "POST / HTTP/1.1\r\n"
            "\r\n"
            "Hello world!";

        HttpRequest req(raw);
        EXPECT_EQ(req.body(), "Hello world!");
    }
}