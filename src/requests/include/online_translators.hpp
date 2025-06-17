/**
 * @file online_translators.hpp
 * @brief Менеджер онлайн-переводчиков
 * 
 * @details Заголовочный файл содержит класс для работы с различными
 * онлайн-сервисами перевода (Yandex, LibreTranslate, DeepL).
 */

#ifndef ONLINE_TRANSLATORS_HPP
#define ONLINE_TRANSLATORS_HPP

#include "http_client.hpp"

#include <string>
#include <vector>
#include <stdexcept>

#include <boost/json.hpp>
#include <boost/system/error_code.hpp>

namespace json = boost::json;

/**
 * @struct ApiKeys
 * @brief Структура для хранения API-ключей сервисов перевода
 */
struct ApiKeys {
    std::string yandex_key;
    std::string yandex_folder_id;
    std::string libretranslate_key;
    std::string deepl_key;
};

/**
 * @struct TranslationResult
 * @brief Результат перевода от одного сервиса
 */
struct TranslationResult {
    std::string translator_name; 
    std::string translated_text; 
    bool success;
    std::string error_message;

    /**
     * @brief Конструктор результата перевода
     * 
     * @param name Название сервиса
     * @param text Переведенный текст
     * @param s Успешность перевода
     * @param error Сообщение об ошибке
     */
    TranslationResult(const std::string& name, const std::string& text, bool s, const std::string& error = "")
        : translator_name(name), translated_text(text), success(s), error_message(error) {}
};

/**
 * @class OnlineTranslatorsManager
 * @brief Менеджер для работы с различными сервисами перевода
 * 
 * @details Класс предоставляет единый интерфейс для работы с несколькими
 * сервисами перевода: Яндекс.Переводчик, LibreTranslate и DeepL.
 */
class OnlineTranslatorsManager {  
private: 
    HttpClient http_client_;
    ApiKeys api_keys_;

public: 
    /**
     * @brief Конструктор менеджера переводчиков
     * 
     * @param config_filepath Путь к файлу конфигурации с API-ключами
     */
    OnlineTranslatorsManager(const std::string &config_filepath);

    /**
     * @brief Получение переводов от всех доступных сервисов
     * 
     * @param text Текст для перевода
     * @param source_lang Исходный язык
     * @param target_lang Целевой язык
     * 
     * @return Вектор результатов перевода от каждого сервиса
     */
    std::vector<TranslationResult> GetTranslations( 
        const std::string& text,
        const std::string& source_lang,
        const std::string& target_lang
    );

private: 
    /**
     * @brief Получение перевода от одного сервиса
     * 
     * @param translator_name Название сервиса
     * @param url URL для запроса
     * @param target Путь запроса
     * @param method HTTP-метод
     * @param request_body Тело запроса
     * @param headers HTTP-заголовки
     * 
     * @return Результат перевода
     */
    TranslationResult GetTranslationFromOne( 
        const std::string& translator_name,
        const std::string& url,
        const std::string& target, 
        http::verb method,
        const std::string& request_body,
        const std::vector<std::pair<std::string, std::string>>& headers
    );

    /**
     * @brief Подготовка запроса к Яндекс.Переводчику
     * 
     * @param text Текст для перевода
     * @param source_lang Исходный язык
     * @param target_lang Целевой язык
     * 
     * @return Пара из тела запроса и заголовков
     */
    std::pair<std::string, std::vector<std::pair<std::string, std::string>>> 
        PrepareYandexRequest(const std::string& text, const std::string& source_lang, const std::string& target_lang);

    /**
     * @brief Разбор ответа от Яндекс.Переводчика
     * 
     * @param response_body Тело ответа
     * 
     * @return Переведенный текст
     */
    std::string ParseYandexResponse(const std::string& response_body);

    /**
     * @brief Подготовка запроса к LibreTranslate
     * 
     * @param text Текст для перевода
     * @param source_lang Исходный язык
     * @param target_lang Целевой язык
     * 
     * @return Пара из тела запроса и заголовков
     */
    std::pair<std::string, std::vector<std::pair<std::string, std::string>>>
        PrepareLibreTranslateRequest(const std::string& text, const std::string& source_lang, const std::string& target_lang);

    /**
     * @brief Разбор ответа от LibreTranslate
     * 
     * @param response_body Тело ответа
     * 
     * @return Переведенный текст
     */
    std::string ParseLibreTranslateResponse(const std::string& response_body);

    /**
     * @brief Подготовка запроса к DeepL
     * 
     * @param text Текст для перевода
     * @param source_lang Исходный язык
     * @param target_lang Целевой язык
     * 
     * @return Пара из тела запроса и заголовков
     */
    std::pair<std::string, std::vector<std::pair<std::string, std::string>>>
        PrepareDeepLRequest(const std::string& text, const std::string& source_lang, const std::string& target_lang);
    
    /**
     * @brief Разбор ответа от DeepL
     * 
     * @param response_body Тело ответа
     * 
     * @return Переведенный текст
     */
    std::string ParseDeepLResponse(const std::string& response_body);
}; 

#endif 
