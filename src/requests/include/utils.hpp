/**
 * @file utils.hpp
 * @brief Утилиты для работы с API-ключами
 * 
 * @details Заголовочный файл содержит вспомогательные функции
 * для работы с конфигурацией и API-ключами сервисов перевода.
 */

#ifndef UTILS_HPP 
#define UTILS_HPP 

#include "online_translators.hpp"
#include <string> 
#include <stdexcept> 

/**
 * @brief Загрузка API-ключей из файла конфигурации
 * 
 * @param filepath Путь к файлу конфигурации
 * 
 * @return Структура с API-ключами
 * 
 * @throws std::runtime_error если файл не найден или имеет неверный формат
 */
ApiKeys load_api_keys_from_file(const std::string& filepath); 

#endif 
