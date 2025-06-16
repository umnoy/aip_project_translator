#include "../include/http_client.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <stdexcept>

std::string HttpClient::SendRequest(
    const std::string& host,
    const std::string& port,
    const std::string& target,
    http::verb method,
    const std::string& body,
    const std::vector<std::pair<std::string, std::string>>& headers)
{
    std::cout << "DEBUG: Entering SendRequest" << std::endl;
    net::io_context ioc;
  

    bool is_https = (port == "443");

    ip::tcp::resolver resolver(ioc); 
    sys::error_code ec;
    std::cout << "DEBUG: Before resolve" << std::endl;
    auto const results = resolver.resolve(host, port, ec);
    std::cout << "DEBUG: After resolve" << std::endl;
    if (ec) {
        throw std::runtime_error("DNS error: " + ec.message());
    }
    http::response<http::string_body> res;
    std::string response_body;

    if (is_https) {
        std::cout << "DEBUG: Using HTTPS" << std::endl;
        ssl::context ctx(ssl::context::method::tlsv12_client);
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

        std::cout << "DEBUG: Before stream connect" << std::endl;
        beast::get_lowest_layer(stream).connect(results, ec);
        std::cout << "DEBUG: After stream connect" << std::endl;

        if (ec) {
            throw std::runtime_error("TCP connection error: " + ec.message());
        }

        std::cout << "DEBUG: Before SSL handshake" << std::endl;
        stream.handshake(ssl::stream_base::client, ec);
        std::cout << "DEBUG: After SSL handshake" << std::endl;

        if (ec) {
            throw std::runtime_error("SSL Handshake Error: " + ec.message());
        }

        http::request<http::string_body> req{method, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::connection, "close");

        for (const auto& h : headers) { 
            req.set(h.first, h.second);
        }

        if (!body.empty()) { 
            req.body() = body; 
            req.prepare_payload(); 
        }

        std::cout << "DEBUG: Before http write" << std::endl; 
        http::write(stream, req, ec);
        std::cout << "DEBUG: After http write" << std::endl;

        if (ec) { 
            if(ec == net::error::eof) 
                throw std::runtime_error("Sending error: The server closed the connection prematurely"); 
            else if (ec == ssl::error::stream_truncated)
                throw std::runtime_error("Sending error (SSL): The stream is truncated during recording");
            else
                throw std::runtime_error("Sending error: " + ec.message());
        }

        beast::flat_buffer buffer; 
        std::cout << "DEBUG: Before http read" << std::endl; 
        http::read(stream, buffer, res, ec);
        std::cout << "DEBUG: After http read" << std::endl;

        if (ec && ec != http::error::end_of_stream) {
            if (ec == ssl::error::stream_truncated)
                throw std::runtime_error("Response Read error (SSL): The stream is truncated during reading");
            else
                throw std::runtime_error("Error reading the response: " + ec.message());
        }

        std::cout << "DEBUG: Before stream shutdown" << std::endl;
        stream.shutdown(ec);

        if (ec && ec != beast::errc::not_connected && ec != ssl::error::stream_truncated) {
            std::cerr << "Ошибка при SSL shutdown: " << ec.message() << std::endl;
        }

        response_body = res.body();

    } else {
        std::cout << "DEBUG: Using HTTP" << std::endl;
        beast::tcp_stream stream(ioc);
        std::cout << "DEBUG: Before stream connect (HTTP)" << std::endl;
        stream.connect(results, ec);
        std::cout << "DEBUG: After stream connect (HTTP)" << std::endl;
            
        if (ec) {
            throw std::runtime_error("TCP connection error (HTTP): " + ec.message());
        }
            
        http::request<http::string_body> req{method, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::connection, "close");
        for (const auto& h : headers) { req.set(h.first, h.second); }
        if (!body.empty()) { req.body() = body; req.prepare_payload(); }

        std::cout << "DEBUG: Before http write (HTTP)" << std::endl;
        http::write(stream, req, ec);
        std::cout << "DEBUG: After http write (HTTP)" << std::endl;

        if (ec) {
            if(ec == net::error::eof)
                throw std::runtime_error("Sending error (HTTP): The server closed the connection prematurely");
            else
                throw std::runtime_error("Sending error (HTTP): " + ec.message());
        }

        beast::flat_buffer buffer;
        std::cout << "DEBUG: Before http read (HTTP)" << std::endl;
        http::read(stream, buffer, res, ec);
        std::cout << "DEBUG: After http read (HTTP)" << std::endl;

        if (ec && ec != http::error::end_of_stream) {
             throw std::runtime_error("Error reading the response (HTTP): " + ec.message());
        }

        std::cout << "DEBUG: Before socket shutdown (HTTP)" << std::endl;
        stream.socket().shutdown(ip::tcp::socket::shutdown_both, ec);
        if (ec && ec != beast::errc::not_connected) {
            std::cerr << "TCP shutdown error: " << ec.message() << std::endl;
        }
        response_body = res.body();
    }
    if (res.result() != http::status::ok) {
        std::string error_msg = "The server returned the error status: " + std::to_string(res.result_int());
        if (!response_body.empty()) { 
             error_msg += ". Response body: " + response_body;
        }
        throw std::runtime_error(error_msg);
    }

    std::cout << "DEBUG: Exiting SendRequest" << std::endl;
    return response_body; 
}
