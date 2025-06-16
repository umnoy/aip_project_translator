/**
 * @file utils.cpp
 * @brief Реализация утилит для работы с конфигурацией
 * 
 * @details Файл содержит реализацию функций для работы с конфигурационными
 * файлами и API-ключами сервисов перевода.
 */

#include "../include/utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream> 
#include <stdexcept> 

#include <boost/json.hpp>
#include <boost/system/error_code.hpp> 
#include <boost/system/system_error.hpp> 

namespace json = boost::json;
namespace sys = boost::system;

/**
 * @brief Загрузка API-ключей из JSON-файла конфигурации
 * 
 * @details Функция читает и парсит JSON-файл конфигурации, извлекая API-ключи
 * для различных сервисов перевода.
 * 
 * @param filepath Путь к файлу конфигурации
 * 
 * @return Структура ApiKeys с загруженными ключами
 * 
 * @throws std::runtime_error в случаях:
 *   - Файл не может быть открыт
 *   - Ошибка парсинга JSON
 *   - Ошибка извлечения данных из JSON
 * 
 * @note При отсутствии некоторых ключей в конфигурации
 * выводится предупреждение, но функция продолжает работу
 */
ApiKeys load_api_keys_from_file(const std::string& filepath) {
    ApiKeys keys;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::stringstream ss;
        ss << "CONFIGURATION ERROR: Failed to open configuration file '" << filepath << "'.";
        throw std::runtime_error(ss.str());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::string json_content = buffer.str();

    sys::error_code ec;
    json::value jv = json::parse(json_content, ec);
    if (ec) {
        std::stringstream ss;
        ss << "CONFIGURATION ERROR: JSON parsing error in file '" << filepath << "': " << ec.message();
        throw std::runtime_error(ss.str());
    }

    try {
        json::object obj = jv.as_object();

        if (obj.count("yandex_translate") && obj.at("yandex_translate").is_object()) {
            json::object yandex_obj = obj.at("yandex_translate").as_object();
            if (yandex_obj.count("api_key") && yandex_obj.at("api_key").is_string()) {
                keys.yandex_key = std::string(yandex_obj.at("api_key").as_string().c_str());
            } else {
                std::cerr << "Warning: 'yandex_translate.api_key' not found or invalid in '" << filepath << "'." << std::endl;
            }
            if (yandex_obj.count("folder_id") && yandex_obj.at("folder_id").is_string()) {
                keys.yandex_folder_id = std::string(yandex_obj.at("folder_id").as_string().c_str());
            } else {
                std::cerr << "Warning: 'yandex_translate.folder_id' not found or invalid in '" << filepath << "'." << std::endl;
            }
        } else {
            std::cerr << "Warning: 'yandex_translate' section not found in '" << filepath << "'." << std::endl;
        }

        if (obj.count("libretranslate") && obj.at("libretranslate").is_object()) {
            json::object libretranslate_obj = obj.at("libretranslate").as_object();
            if (libretranslate_obj.count("api_key") && libretranslate_obj.at("api_key").is_string()) {
                keys.libretranslate_key = std::string(libretranslate_obj.at("api_key").as_string().c_str());
            } else {
                std::cerr << "Warning: 'libretranslate.api_key' not found or invalid in '" << filepath << "'." << std::endl;
            }
        } else {
            std::cerr << "Warning: 'libretranslate' section not found in '" << filepath << "'." << std::endl;
        }

        if (obj.count("deepl") && obj.at("deepl").is_object()) {
            json::object deepl_obj = obj.at("deepl").as_object();
            if (deepl_obj.count("api_key") && deepl_obj.at("api_key").is_string()) {
                keys.deepl_key = std::string(deepl_obj.at("api_key").as_string().c_str());
            } else {
                std::cerr << "Warning: 'deepl.api_key' not found or invalid in '" << filepath << "'." << std::endl;
            }
        } else {
            std::cerr << "Warning: 'deepl' section not found in '" << filepath << "'." << std::endl;
        }
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "CONFIGURATION ERROR: Error extracting data from JSON file '" << filepath << "': " << e.what();
        throw std::runtime_error(ss.str());
    }

    return keys;
}
