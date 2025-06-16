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

TEST_CASE("Tokenizer basic operations") {
    const std::string vocab_path = "test_vocab.json";
    create_test_vocab(vocab_path);
    Tokenizer tokenizer(vocab_path);

    std::string input = "Hello World! Test";
    std::vector<int64_t> tokens = tokenizer.encode(input);
    
    REQUIRE(tokens.size() > 0); // REQUIRE: Должны быть токены после кодирования
    REQUIRE(tokenizer.decode(tokens).find("Hello") != std::string::npos); // REQUIRE: Декодирование сохраняет слова
    
    CHECK(tokens.front() == 101); // CHECK: Первый токен соответствует "Hello"
    CHECK(tokenizer.encode("!").size() == 1); // CHECK: Знак препинания кодируется в один токен
    
    WARN(tokenizer.encode("Unknown").size() == 1); // WARN: Неизвестное слово кодируется в один токен
    WARN(tokenizer.decode({0}) == ""); // WARN: Неизвестный токен декодируется в пустую строку
} 