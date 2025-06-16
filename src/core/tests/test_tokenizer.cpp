#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "/../tokenizer/tokenizer.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


void create_test_vocab(const std::string& path) {
    nlohmann::json vocab = {
        {"<unk>", 0},
        {"<pad>", 1},
        {"<s>", 2},
        {"</s>", 3},
        {"▁Hello", 101},
        {"▁World", 102},
        {"!", 103},
        {"▁Test", 104}
    };
    std::ofstream file(path);
    file << vocab.dump();
    file.close();
}


void create_invalid_vocab(const std::string& path) {
    std::ofstream file(path);
    file << "{invalid_json}";
    file.close();
}

TEST_CASE("Tokenizer functionality") {
    const std::string vocab_path = "test_vocab.json";
    const std::string invalid_vocab_path = "invalid_vocab.json";
    create_test_vocab(vocab_path);
    create_invalid_vocab(invalid_vocab_path);

    SUBCASE("Tokenizer initialization") {
        REQUIRE_NOTHROW(Tokenizer(vocab_path)); // REQUIRE 1: Корректная загрузка словаря
        REQUIRE_THROWS_AS(Tokenizer("non_existent.json"), std::runtime_error); // REQUIRE 2: Ошибка при отсутствии файла
        CHECK_THROWS_AS(Tokenizer(invalid_vocab_path), nlohmann::json::parse_error); // CHECK 1: Ошибка при некорректном JSON
        CHECK_NOTHROW(Tokenizer(vocab_path)); // CHECK 2: Повторная загрузка корректного словаря
        WARN_THROWS_AS(Tokenizer(""), std::runtime_error); // WARN 1: Предупреждение о пустом пути
        WARN_NOTHROW(Tokenizer(vocab_path)); // WARN 2: Успешная инициализация
    }

    SUBCASE("Normalize text") {
        Tokenizer tokenizer(vocab_path);
        std::string input = "Hello\t\nWorld";
        std::string normalized = tokenizer.normalize(input);
        CHECK(normalized == "Hello World"); // CHECK 1: Нормализация пробельных символов
        CHECK(tokenizer.normalize("  Test  ") == " Test "); // CHECK 2: Удаление лишних пробелов
        REQUIRE(normalized.find("\t") == std::string::npos); // REQUIRE 1: Табуляция удалена
        REQUIRE(tokenizer.normalize("") == ""); // REQUIRE 2: Пустая строка остаётся пустой
        WARN(tokenizer.normalize("NoSpaces") == "NoSpaces"); // WARN 1: Текст без пробелов не меняется
        WARN(tokenizer.normalize("\n\n") == " "); // WARN 2: Множественные переводы строки заменяются пробелом
    }

    SUBCASE("Encode and decode text") {
        Tokenizer tokenizer(vocab_path);
        std::string input = "Hello World!";
        std::vector<int64_t> tokens = tokenizer.encode(input);
        CHECK(tokens == std::vector<int64_t>{101, 102, 103}); // CHECK 1: Корректное кодирование
        CHECK(tokenizer.encode("Unknown") == std::vector<int64_t>{0}); // CHECK 2: Неизвестное слово кодируется как <unk>
        REQUIRE(tokenizer.decode(tokens) == "Hello World!"); // REQUIRE 1: Декодирование восстанавливает текст
        REQUIRE(tokenizer.decode({0}) == ""); // REQUIRE 2: Декодирование <unk> даёт пустую строку
        WARN(tokenizer.encode("").empty()); // WARN 1: Пустой текст не создаёт токенов
        WARN(tokenizer.decode({1, 2, 3}).empty()); // WARN 2: Специальные токены игнорируются при декодировании
    }
}
