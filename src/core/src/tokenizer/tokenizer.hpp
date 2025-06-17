#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Класс для токенизации текста на основе словаря.
 *
 * Этот класс загружает словарь из JSON-файла и использует его для кодирования
 * текста в последовательность идентификаторов токенов и декодирования обратно в текст.
 */
class Tokenizer {
public:
    /**
     * @brief Конструктор, загружающий словарь из файла.
     * @param vocab_path Путь к JSON-файлу со словарем.
     * @throws std::runtime_error Если файл не удалось открыть или прочитать.
     *
     */
    Tokenizer(const std::string &vocab_path);

    /**
     * @brief Кодирует текст в последовательность идентификаторов токенов.
     * @param text Входной текст для кодирования.
     * @return Вектор идентификаторов токенов.
     *
     */
    std::vector<int64_t> encode(const std::string &text);

    /**
     * @brief Декодирует последовательность идентификаторов токенов в текст.
     * @param token_ids Вектор идентификаторов токенов.
     * @return Декодированный текст.
     *
     */
    std::string decode(const std::vector<int64_t> &token_ids);

    /**
     * @brief Нормализует текст, заменяя пробельные символы на одиночные пробелы.
     * @param text Входной текст для нормализации.
     * @return Нормализованный текст.
     *
     */
    std::string normalize(const std::string &text);

    std::unordered_map<std::string, int64_t> token_to_id; ///< Маппинг токенов в их идентификаторы.
    std::unordered_map<int64_t, std::string> id_to_token; ///< Маппинг идентификаторов в токены.
    static const std::string spm_space; ///< Специальный префикс для токенов

};
