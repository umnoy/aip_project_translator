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
        return std::vector<float>(input_ids.size() * 768, 0.1f);
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

TEST_CASE("Translator basic translation") {
    const std::string vocab_path = "test_vocab.json";
    create_test_vocab(vocab_path);
    Tokenizer tokenizer(vocab_path);
    MockTranslator translator(tokenizer, 1, 3, 50, 3);

    std::string input = "Hello World";
    std::string output = translator.run(input);
    
    REQUIRE(!output.empty()); // REQUIRE: Перевод не должен быть пустым
    REQUIRE(output.find("Привет") != std::string::npos); // REQUIRE: Должен содержать перевод первого слова
    
    CHECK(output.find("Мир") != std::string::npos); // CHECK: Должен содержать перевод второго слова
    CHECK(output != input); // CHECK: Перевод должен отличаться от входного текста
    
    WARN(output.find("Hello") == std::string::npos); // WARN: Исходный текст не должен остаться в переводе
    WARN(output.length() <= 20); // WARN: Длина перевода должна быть разумной
} 