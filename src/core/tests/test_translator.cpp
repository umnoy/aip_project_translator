#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "../translator/traslator.hpp"
#include "../tokenizer/tokenizer.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>
#include <cmath>


void create_test_vocab(const std::string& path) {
    nlohmann::json vocab = {
        {"<unk>", 0},
        {"<pad>", 1},
        {"<s>", 2},
        {"</s>", 3},
        {"▁Hello", 101},
        {"▁World", 102},
        {"!", 103},
        {"▁Привет", 201},
        {"▁Мир", 202}
    };
    std::ofstream file(path);
    file << vocab.dump();
    file.close();
}


class MockTranslator : public Translator {
public:
    MockTranslator(const Tokenizer& tokenizer, int pad_token_id, int eos_token_id, int max_length, int beam_width)
        : Translator(tokenizer, "", "", pad_token_id, eos_token_id, max_length, beam_width) {}

    std::vector<float> encode_input(const std::vector<int64_t>& input_ids) override {
        return std::vector<float>(input_ids.size() * 768, 0.1f); // Эмуляция скрытого состояния
    }

    std::vector<float> decode_step(const std::vector<int64_t>& input_ids,
                                  const std::vector<int64_t>& encoder_input_ids,
                                  const std::vector<float>& encoder_hidden_state) override {
        std::vector<float> logits(203, 0.0f);
        if (input_ids.back() == 1) { 
            logits[3] = 1.0f;
        } else if (input_ids.back() == 201) {
            logits[202] = 0.8f; 
        } else {
            logits[201] = 0.7f;
            logits[202] = 0.2f;
        }
        return logits;
    }
};

TEST_CASE("Translator functionality") {
    const std::string vocab_path = "test_vocab.json";
    create_test_vocab(vocab_path);
    Tokenizer tokenizer(vocab_path);

    SUBCASE("Translator initialization and decode_ids") {
        REQUIRE_NOTHROW(MockTranslator(tokenizer, 1, 3, 50, 3)); // REQUIRE 1: Инициализация без ошибок
        MockTranslator translator(tokenizer, 1, 3, 50, 3);
        REQUIRE(translator.decode_ids({101, 102, 103}) == "Hello World!"); // REQUIRE 2: Декодирование через decode_ids
        CHECK_NOTHROW(translator.decode_ids({0})); // CHECK 1: Декодирование неизвестного токена
        CHECK(translator.decode_ids({1, 2, 3}).empty()); // CHECK 2: Игнорирование специальных токенов
        WARN(translator.decode_ids({}).empty()); // WARN 1: Пустой вектор токенов
        WARN_NOTHROW(translator.decode_ids({101})); // WARN 2: Успешное декодирование одного токена
    }

    SUBCASE("Softmax and top_k") {
        MockTranslator translator(tokenizer, 1, 3, 50, 3);
        std::vector<float> logits = {0.1f, 0.7f, 0.2f};
        auto probs = translator.softmax(logits);
        CHECK(probs.size() == 3); // CHECK 1: Корректный размер выходного вектора
        CHECK(std::abs(probs[1] - 0.4562f) < 0.01f); // CHECK 2: Проверка значения softmax
        REQUIRE(std::accumulate(probs.begin(), probs.end(), 0.0f) - 1.0f < 0.01f); // REQUIRE 1: Сумма вероятностей равна 1
        auto topk = translator.top_k(probs, 2);
        REQUIRE(topk[0].first == 1); // REQUIRE 2: Топ-1 токен корректен
        WARN(topk.size() == 2); // WARN 1: Размер топ-k равен запрошенному
        WARN(topk[1].second < topk[0].second); // WARN 2: Второй токен имеет меньшую вероятность
    }

    SUBCASE("Run translation") {
        MockTranslator translator(tokenizer, 1, 3, 10, 2);
        std::string input = "Hello World!";
        std::string output = translator.run(input);
        CHECK(output.find("Привет") != std::string::npos); // CHECK 1: Содержит ожидаемое слово
        CHECK(output.find("Мир") != std::string::npos); // CHECK 2: Содержит второе ожидаемое слово
        REQUIRE(!output.empty()); // REQUIRE 1: Результат не пустой
        REQUIRE(output.size() <= 100); // REQUIRE 2: Длина результата ограничена
        WARN(output != input); // WARN 1: Перевод отличается от входного текста
        WARN(output.find("!") == std::string::npos); // WARN 2: Знаки пунктуации могут отсутствовать
    }
}
