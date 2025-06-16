/**
 * @file online_translators.cpp
 * @brief Реализация менеджера онлайн-переводчиков
 * 
 * @details Файл содержит реализацию методов класса OnlineTranslatorsManager
 * для работы с различными сервисами перевода (Yandex, LibreTranslate, DeepL).
 */

#include "../include/online_translators.hpp"
#include "../include/utils.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <boost/json.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/beast/http/verb.hpp>

namespace json = boost::json;
namespace sys = boost::system;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ip = net::ip;

/**
 * @brief Конструктор менеджера переводчиков
 * 
 * @details Загружает API-ключи из файла конфигурации при создании объекта
 * 
 * @param config_filepath Путь к файлу конфигурации
 */
OnlineTranslatorsManager::OnlineTranslatorsManager(const std::string& config_filepath) {
    std::cout << "DEBUG: Inside OnlineTranslatorsManager constructor, before load_api_keys_from_file" << std::endl;
    api_keys_ = load_api_keys_from_file(config_filepath);
    std::cout << "The API keys have been successfully loaded from the file: " << config_filepath << std::endl;
    std::cout << "DEBUG: Inside OnlineTranslatorsManager constructor, after load_api_keys_from_file and message" << std::endl;
}

/**
 * @brief Получение переводов от всех доступных сервисов
 * 
 * @details Последовательно отправляет запросы к каждому сервису перевода
 * (Yandex.Cloud, LibreTranslate, DeepL) и собирает результаты
 * 
 * @param text Текст для перевода
 * @param source_lang Исходный язык
 * @param target_lang Целевой язык
 * 
 * @return Вектор результатов перевода от каждого сервиса
 */
std::vector<TranslationResult> OnlineTranslatorsManager::GetTranslations(
    const std::string& text,
    const std::string& source_lang,
    const std::string& target_lang)
{
    std::vector<TranslationResult> results;
    std::cout << "DEBUG: Entering GetTranslations" << std::endl;

    // Яндекс
    try {
        std::pair<std::string, std::vector<std::pair<std::string, std::string>>> yandex_request_data =
            PrepareYandexRequest(text, source_lang, target_lang);
        std::string yandex_api_url = "translate.api.cloud.yandex.net:443";
        std::string yandex_target = "/translate/v2/translate";

        std::cout << "DEBUG: Before GetTranslationFromOne (Yandex)" << std::endl;
        TranslationResult yandex_result = GetTranslationFromOne(
            "Yandex.Cloud",
            yandex_api_url,
            yandex_target,
            http::verb::post,
            yandex_request_data.first,
            yandex_request_data.second
        );
        std::cout << "DEBUG: After GetTranslationFromOne (Yandex)" << std::endl;
        results.push_back(yandex_result);
    } catch (const std::exception& e) {
        std::cerr << "Error when making a request to Yandex.Cloud: " << e.what() << std::endl;
        results.push_back(TranslationResult("Yandex.Cloud", "", false, e.what()));
    }

  // LibreTranslate
    try {
        std::cout << "DEBUG: Before PrepareLibreTranslateRequest" << std::endl;
        std::pair<std::string, std::vector<std::pair<std::string, std::string>>> libretranslate_request_data =
            PrepareLibreTranslateRequest(text, source_lang, target_lang);
        std::cout << "DEBUG: After PrepareLibreTranslateRequest" << std::endl;

        std::string libretranslate_api_url = "172.20.10.2:5000"; // Актуальный IP для LibreTranslate
        std::string libretranslate_target = "/translate";

        std::cout << "DEBUG: Before GetTranslationFromOne (LibreTranslate)" << std::endl;
        TranslationResult libretranslate_result = GetTranslationFromOne(
            "LibreTranslate",
            libretranslate_api_url,
            libretranslate_target,
            http::verb::post,
            libretranslate_request_data.first,
            libretranslate_request_data.second
        );
        std::cout << "DEBUG: After GetTranslationFromOne (LibreTranslate)" << std::endl;
        results.push_back(libretranslate_result);
    } catch (const std::exception& e) {
        std::cerr << "Error when requesting LibreTranslate: " << e.what() << std::endl;
        results.push_back(TranslationResult("LibreTranslate", "", false, e.what()));
    }

    // DeepL
    try {
        std::cout << "DEBUG: Before PrepareDeepLRequest" << std::endl;
        std::pair<std::string, std::vector<std::pair<std::string, std::string>>> deepl_request_data =
            PrepareDeepLRequest(text, source_lang, target_lang);
        std::cout << "DEBUG: After PrepareDeepLRequest, before GetTranslationFromOne (DeepL)" << std::endl;

        std::string deepl_api_url = "api-free.deepl.com:443";
        std::string deepl_target = "/v2/translate";

        std::cout << "DEBUG: Before GetTranslationFromOne (DeepL)" << std::endl;
        TranslationResult deepl_result = GetTranslationFromOne(
            "DeepL",
            deepl_api_url,
            deepl_target,
            http::verb::post,
            deepl_request_data.first,
            deepl_request_data.second
        );
        std::cout << "DEBUG: After GetTranslationFromOne (DeepL)" << std::endl;
        results.push_back(deepl_result);
    } catch (const std::exception& e) {
        std::cerr << "Error when requesting the DeepL: " << e.what() << std::endl;
        results.push_back(TranslationResult("DeepL", "", false, e.what()));
    }

    std::cout << "DEBUG: Exiting GetTranslations" << std::endl;
    return results;
}

/**
 * @brief Получение перевода от одного сервиса
 * 
 * @details Выполняет HTTP-запрос к указанному сервису и обрабатывает ответ
 * 
 * @param translator_name Название сервиса перевода
 * @param url URL сервиса
 * @param target Путь запроса
 * @param method HTTP-метод
 * @param request_body Тело запроса
 * @param headers HTTP-заголовки
 * 
 * @return Результат перевода
 */
TranslationResult OnlineTranslatorsManager::GetTranslationFromOne(
    const std::string& translator_name,
    const std::string& url,
    const std::string& target,
    http::verb method,
    const std::string& request_body,
    const std::vector<std::pair<std::string, std::string>>& headers)
{
    std::cout << "DEBUG: Entering GetTranslationFromOne for " << translator_name << std::endl;
    std::string translated_text = "Error: The transfer could not be received";
    bool success = false;
    std::string error_message = "";

    try {
        std::string host;
        std::string port;
        size_t colon_pos = url.find(":");
        if (colon_pos == std::string::npos) {
            host = url;
            port = "443";
        } else {
            host = url.substr(0, colon_pos);
            port = url.substr(colon_pos + 1);
        }

        std::cout << "DEBUG: Before SendRequest call for " << translator_name << std::endl;
        std::string response_body = http_client_.SendRequest(
            host, port, target, method, request_body, headers
        );
        std::cout << "DEBUG: After SendRequest call for " << translator_name << ", response: " << response_body << std::endl;

        if (translator_name == "Yandex.Cloud") {
            std::cout << "DEBUG: Parsing Yandex response" << std::endl;
            translated_text = ParseYandexResponse(response_body);
            if (translated_text.find("Error") == std::string::npos) {
                success = true;
                error_message = "";
            } else {
                success = false;
                error_message = translated_text;
                translated_text = "";
            }
        } else if (translator_name == "LibreTranslate") {
            std::cout << "DEBUG: Parsing LibreTranslate response" << std::endl;
            translated_text = ParseLibreTranslateResponse(response_body);
            if (translated_text.rfind("Error", 0) != 0) {
                success = true;
                error_message = "";
            } else {
                success = false;
                error_message = translated_text;
                translated_text = "";
            }
        } else if (translator_name == "DeepL") {
            std::cout << "DEBUG: Parsing DeepL response" << std::endl;
            translated_text = ParseDeepLResponse(response_body);
            if (translated_text.rfind("Error", 0) != 0) {
                success = true;
                error_message = "";
            } else {
                success = false;
                error_message = translated_text;
                translated_text = "";
            }
        } else {
            error_message = "Unknown translator: " + translator_name;
            success = false;
            translated_text = "";
        }
    } catch (const boost::system::system_error& e) {
        error_message = "Boost system error for " + translator_name + ": " + e.what();
        std::cerr << "DEBUG: " << error_message << std::endl;
        success = false;
        translated_text = "";
    } catch (const std::exception& e) {
        error_message = "Standard exception for " + translator_name + ": " + e.what();
        std::cerr << "DEBUG: " << error_message << std::endl;
        success = false;
        translated_text = "";
    } catch (...) {
        error_message = "Unknown error for " + translator_name;
        std::cerr << "DEBUG: " << error_message << std::endl;
        success = false;
        translated_text = "";
    }
    std::cout << "DEBUG: Exiting GetTranslationFromOne for " << translator_name << ", success: " << success << ", error: " << error_message << std::endl;
    return TranslationResult(translator_name, translated_text, success, error_message);
}

/**
 * @brief Подготовка запроса к Яндекс.Переводчику
 * 
 * @details Формирует JSON-тело запроса и заголовки для API Яндекс.Переводчика
 * 
 * @param text Текст для перевода
 * @param source_lang Исходный язык
 * @param target_lang Целевой язык
 * 
 * @return Пара из тела запроса и заголовков
 */
std::pair<std::string, std::vector<std::pair<std::string, std::string>>>
OnlineTranslatorsManager::PrepareYandexRequest(const std::string& text, const std::string& source_lang, const std::string& target_lang)
{
    json::value json_body = json::object {
        {"folderId", api_keys_.yandex_folder_id},
        {"sourceLanguageCode", source_lang.empty() ? "auto" : source_lang},
        {"targetLanguageCode", target_lang},
        {"texts", json::array{text}}
    };

    std::string request_body = json::serialize(json_body);

    std::vector<std::pair<std::string, std::string>> headers = {
        {"Content-Type", "application/json"},
        {"Authorization", "Api-Key " + api_keys_.yandex_key}
    };

    return {request_body, headers};
}

/**
 * @brief Разбор ответа от Яндекс.Переводчика
 * 
 * @details Парсит JSON-ответ от API и извлекает переведенный текст
 * 
 * @param response_body Тело ответа от API
 * 
 * @return Переведенный текст или сообщение об ошибке
 */
std::string OnlineTranslatorsManager::ParseYandexResponse(const std::string& response_body)
{
    try {
        json::value jv = json::parse(response_body);

        if (!jv.is_object()) {
            return "Error parsing the Yandex response: a non-JSON object was received.";
        }

        json::object response_obj = jv.as_object();

        if (response_obj.count("error")) {
            if (response_obj["error"].is_object() && response_obj["error"].as_object().count("message")) {
                if (response_obj["error"].as_object()["message"].is_string()) {
                    return "Yandex.Cloud API error: " + std::string(response_obj["error"].as_object()["message"].as_string().c_str());
                } else {
                    return "Yandex.Cloud API error: Неизвестный формат сообщения об ошибке.";
                }
            } else {
                return "Yandex.Cloud API error: Unknown format of the 'error' field.";
            }
        }

        if (!response_obj.count("translations") || !response_obj["translations"].is_array()) {
            return "Error parsing the Yandex response: missing or incorrect format of the 'translations' field.";
        }

        json::array translations_array = response_obj["translations"].as_array();

        if (translations_array.empty()) {
            return "Error parsing the Yandex response: the transfer array is empty.";
        }

        if (translations_array[0].is_object()) {
            json::object translation_obj = translations_array[0].as_object();
            if (translation_obj.count("text") && translation_obj.at("text").is_string()) {
                return std::string(translation_obj.at("text").as_string().c_str());
            } else {
                return "Error parsing the Yandex response: incorrect format of the 'text' field in the first translation.";
            }
        } else {
            return "Error parsing the Yandex response: the first element of the translation array is not an object.";
        }

    } catch (const boost::system::system_error& e) {
        std::cerr << "Error parsing the Yandex JSON response: " << e.what() << std::endl;
        return "Error parsing the Yandex response.";
    } catch (const std::exception& e) {
        std::cerr << "Error processing the Yandex JSON response (structure): " << e.what() << std::endl;
        return "Error processing the Yandex response (structure).";
    }
}

/**
 * @brief Подготовка запроса к LibreTranslate
 * 
 * @details Формирует JSON-тело запроса и заголовки для API LibreTranslate
 * 
 * @param text Текст для перевода
 * @param source_lang Исходный язык
 * @param target_lang Целевой язык
 * 
 * @return Пара из тела запроса и заголовков
 */
std::pair<std::string, std::vector<std::pair<std::string, std::string>>>
OnlineTranslatorsManager::PrepareLibreTranslateRequest(const std::string& text, const std::string& source_lang, const std::string& target_lang)
{
    json::value json_body = json::object {
        {"q", text},
        {"source", source_lang.empty() ? "auto" : source_lang},
        {"target", target_lang},
        {"format", "text"},
        {"alternatives", 3},
        {"api_key", api_keys_.libretranslate_key}
    };

    std::string request_body = json::serialize(json_body);

    std::vector<std::pair<std::string, std::string>> headers = {
        {"Content-Type", "application/json"}
    };

    return {request_body, headers};
}

/**
 * @brief Разбор ответа от LibreTranslate
 * 
 * @details Парсит JSON-ответ от API и извлекает переведенный текст
 * 
 * @param response_body Тело ответа от API
 * 
 * @return Переведенный текст или сообщение об ошибке
 */
std::string OnlineTranslatorsManager::ParseLibreTranslateResponse(const std::string& response_body)
{
    try {
        json::value jv = json::parse(response_body);

        if (!jv.is_object()) {
            return "Error parsing the LibreTranslate response: a non-JSON object was received.";
        }

        json::object response_obj = jv.as_object();

        if (response_obj.count("error")) {
            if (response_obj["error"].is_string()) {
                return "LibreTranslate API error: " + std::string(response_obj["error"].as_string().c_str());
            } else {
                return "LibreTranslate API error: Unknown error format in the 'error' field.";
            }
        }

        if (response_obj.count("translatedText") && response_obj["translatedText"].is_string()) {
            return std::string(response_obj["translatedText"].as_string().c_str());
        }

        return "Error parsing the LibreTranslate response: incorrect or unexpected response format.";

    } catch (const boost::system::system_error& e) {
        std::cerr << "Error parsing the LibreTranslate JSON response: " << e.what() << std::endl;
        return "Error parsing the LibreTranslate response.";
    } catch (const std::exception& e) {
        std::cerr << "Error processing the LibreTranslate JSON response (structure): " << e.what() << std::endl;
        return "Error processing the LibreTranslate response (structure).";
    }
}

/**
 * @brief Подготовка запроса к DeepL
 * 
 * @details Формирует JSON-тело запроса и заголовки для API DeepL
 * 
 * @param text Текст для перевода
 * @param source_lang Исходный язык
 * @param target_lang Целевой язык
 * 
 * @return Пара из тела запроса и заголовков
 */
std::pair<std::string, std::vector<std::pair<std::string, std::string>>>
OnlineTranslatorsManager::PrepareDeepLRequest(const std::string& text, const std::string& source_lang, const std::string& target_lang)
{
    json::value json_body = json::object {
        {"text", json::array{text}},
        {"target_lang", target_lang}
    };

    if (!source_lang.empty()) {
        std::string source_lang_upper = source_lang;
        std::transform(source_lang_upper.begin(), source_lang_upper.end(), source_lang_upper.begin(), ::toupper);
        json_body.as_object().emplace("source_lang", source_lang_upper);
    }

    std::string request_body = json::serialize(json_body);

    std::vector<std::pair<std::string, std::string>> headers = {
        {"Content-Type", "application/json"},
        {"Accept", "application/json"},
        {"Authorization", "DeepL-Auth-Key " + api_keys_.deepl_key}
    };

    return {request_body, headers};
}

/**
 * @brief Разбор ответа от DeepL
 * 
 * @details Парсит JSON-ответ от API и извлекает переведенный текст
 * 
 * @param response_body Тело ответа от API
 * 
 * @return Переведенный текст или сообщение об ошибке
 */
std::string OnlineTranslatorsManager::ParseDeepLResponse(const std::string& response_body)
{
    try {
        json::value jv = json::parse(response_body);

        if (!jv.is_object()) {
            return "DeepL response parsing error: non-JSON object received.";
        }

        json::object response_obj = jv.as_object();

        if (response_obj.contains("message")) {
            if (response_obj.at("message").is_string()) {
                return "DeepL API error: " + std::string(response_obj.at("message").as_string().c_str());
            }
            return "DeepL API error: Unknown error message format.";
        }

        if (!response_obj.contains("translations") || !response_obj.at("translations").is_array()) {
            return "DeepL response parsing error: missing or incorrect format of the 'translations' field.";
        }

        json::array translations_array = response_obj.at("translations").as_array();
        if (translations_array.empty()) {
            return "DeepL response parsing error: the translations array is empty.";
        }

        if (translations_array[0].is_object()) {
            json::object translation_obj = translations_array[0].as_object();
            if (translation_obj.contains("text") && translation_obj.at("text").is_string()) {
                return std::string(translation_obj.at("text").as_string().c_str());
            }
            return "DeepL response parsing error: incorrect format of the 'text' field in the first translation.";
        }

        return "DeepL response parsing error: the first element of the translations array is not an object.";
    } catch (const boost::system::system_error& e) {
        std::cerr << "DeepL JSON response parsing error: " << e.what() << std::endl;
        return "DeepL response parsing error.";
    } catch (const std::exception& e) {
        std::cerr << "Error processing the DeepL JSON response (structure): " << e.what() << std::endl;
        return "Error processing the DeepL response (structure).";
    }
}
