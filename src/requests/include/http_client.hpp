/**
 * @file http_client.hpp
 * @brief HTTP-клиент на базе Boost.Beast
 * 
 * @details Заголовочный файл содержит реализацию HTTP-клиента для выполнения
 * HTTP/HTTPS запросов с использованием библиотеки Boost.Beast.
 */

#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <string>
#include <vector>
#include <utility>
#include <stdexcept>

#include <boost/beast/core.hpp> 
#include <boost/beast/http.hpp> 
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/system/error_code.hpp> 

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ip = net::ip;
namespace sys = boost::system;
namespace ssl = net::ssl;

/**
 * @class HttpClient
 * @brief Класс для выполнения HTTP/HTTPS запросов
 * 
 * @details Реализует функциональность для отправки HTTP-запросов.
 * Поддерживает HTTP и HTTPS протоколы.
 * 
 * @note Потокобезопасен
 */
class HttpClient {
public:
    /**
     * @brief Выполняет HTTP-запрос
     * 
     * @param[in] host Хост или IP-адрес
     * @param[in] port Порт (80 для HTTP, 443 для HTTPS)
     * @param[in] target Путь запроса
     * @param[in] method HTTP-метод
     * @param[in] body Тело запроса
     * @param[in] headers HTTP-заголовки
     * 
     * @return Тело ответа
     * 
     * @throws std::runtime_error при ошибке запроса
     * @throws std::invalid_argument при некорректных параметрах
     */
    std::string SendRequest(
        const std::string& host,
        const std::string& port,
        const std::string& target,
        http::verb method,
        const std::string& body = "",
        const std::vector<std::pair<std::string, std::string>>& headers = {}
    );

};

#endif
