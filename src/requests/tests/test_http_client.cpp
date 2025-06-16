#include "../include/http_client.hpp"
#include "doctest.h"
#include <string>
#include <vector>
#include <iostream>

TEST_SUITE("HTTP Client Tests") {
    HttpClient client;

    TEST_CASE("Successful HTTP GET") {
        std::string response = client.SendRequest("httpbin.org", "80", "/get", http::verb::get);
        CHECK(response.find("\"url\": \"http://httpbin.org/get\"") != std::string::npos);
        WARN(response.length() > 100);
        std::cout << response << std::endl;
    }

    TEST_CASE("Successful HTTPS GET") {
        std::string response = client.SendRequest("httpbin.org", "443", "/get", http::verb::get);
        CHECK(response.find("\"url\": \"https://httpbin.org/get\"") != std::string::npos);
        WARN(response.length() > 100);
        std::cout << response << std::endl;
    }

    TEST_CASE("Custom headers") {
        std::vector<std::pair<std::string, std::string>> headers = {
            {"X-Custom-Header", "test"},
            {"Authorization", "Bearer token"}
        };
        std::string response = client.SendRequest("httpbin.org", "443", "/headers", http::verb::get, "", headers);
        CHECK(response.find("\"X-Custom-Header\": \"test\"") != std::string::npos);
        CHECK(response.find("\"Authorization\": \"Bearer token\"") != std::string::npos);
        WARN(response.length() > 100);
        std::cout << response << std::endl;
    }

    TEST_CASE("DNS error") {
        CHECK_THROWS_AS(client.SendRequest("nonexistent.domain", "80", "/", http::verb::get), std::runtime_error);
    }

    TEST_CASE("Connection error") {
        CHECK_THROWS_AS(client.SendRequest("localhost", "9999", "/", http::verb::get), std::runtime_error);
    }

}

int main(int argc, char** argv) {
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
} 