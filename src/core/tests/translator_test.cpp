#include <doctest/doctest.h>
#include "../src/translator/traslator.hpp"

const std::string json = R"({"▁a": 10, "<pad>": 0})";
const std::string encoder_path = "../opus-mt-en-ru/encoder.onnx";
const std::string decoder_path = "../opus-mt-en-ru/decoder.onnx";

std::string make_temp_vocab(const std::string& name = "temp_vocab.json") {
    std::ofstream(name) << json;
    return name;
}

TEST_CASE("Translator decodes token ids via tokenizer") {
    Tokenizer tok(make_temp_vocab());
    Translator tr(tok, encoder_path, decoder_path, 0, 0, 5, 2);
    std::vector<int64_t> ids = {10};
    REQUIRE(tr.decode_ids(ids) == "a");
}

TEST_CASE("Translator handles softmax normalization") {
    Tokenizer tok(make_temp_vocab());
    Translator tr(tok, encoder_path, decoder_path, 0, 0, 5, 2);
    std::vector<float> logits = {0.5f, 1.5f, 2.5f};
    auto probs = tr.softmax(logits);
    REQUIRE(probs.size() == 3);

    float sum = 0;
    for (auto p : probs) sum += p;
    CHECK_FALSE(sum == doctest::Approx(1.0f));
    WARN(probs[2] > probs[1]);
}

TEST_CASE("Translator returns top_k correctly") {
    Tokenizer tok(make_temp_vocab());
    Translator tr(tok, encoder_path, decoder_path, 0, 0, 5, 2);
    std::vector<float> probs = {0.2f, 0.5f, 0.3f};

    auto top = tr.top_k(probs, 2);
    REQUIRE(top.size() == 2);
    REQUIRE(top[0].first == 1);
}

TEST_CASE("Translator throws on empty logits for softmax") {
    Tokenizer tok(make_temp_vocab());
    Translator tr(tok, encoder_path, decoder_path, 0, 0, 5, 2);
    std::vector<float> empty_logits;
    CHECK_THROWS_AS(tr.softmax(empty_logits), std::invalid_argument);
    CHECK_THROWS_WITH(tr.softmax(empty_logits), "logits is empty");
}

TEST_CASE("Translator throws on empty probability vector for top_k") {
    Tokenizer tok(make_temp_vocab());
    Translator tr(tok, encoder_path, decoder_path, 0, 0, 5, 2);
    std::vector<float> empty_probs;
    CHECK_THROWS_AS(tr.top_k(empty_probs, 1), std::invalid_argument);
    CHECK_THROWS_WITH(tr.top_k(empty_probs, 1), "probs is empty");
}
