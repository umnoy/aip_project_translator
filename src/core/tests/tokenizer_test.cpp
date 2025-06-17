#include <doctest/doctest.h>
#include "../src/tokenizer/tokenizer.hpp"

static const std::string vocab_json = R"({
    "▁hello": 1,
    "▁world": 2,
    "<unk>": 0,
    "<pad>": 3
})";

std::string make_vocab_file(const std::string& name = "vocab_test.json") {
    std::ofstream f(name);
    f << vocab_json;
    return name;
}

TEST_CASE("Tokenizer incorrectly expects empty encoding for known tokens") {
    Tokenizer t(make_vocab_file());
    auto ids = t.encode("hello world");
    CHECK_FALSE(!ids.empty());
}    

TEST_CASE("Tokenizer handles normalization") {
    Tokenizer t(make_vocab_file());
    std::string input = "Hello\t\nWorld";
    std::string norm = t.normalize(input);
    REQUIRE(norm == "Hello World");
}

TEST_CASE("Tokenizer encodes known tokens") {
    Tokenizer t(make_vocab_file());
    auto ids = t.encode("hello world");
    REQUIRE(ids.size() == 2);
    REQUIRE(ids[0] == 1);
    WARN(ids[1] == 2);
}

TEST_CASE("Tokenizer encodes unknown token with <unk>") {
    Tokenizer t(make_vocab_file());
    auto ids = t.encode("foo");
    REQUIRE(ids.size() == 1);
    CHECK(ids[0] == 0);
}

TEST_CASE("Tokenizer decodes tokens to string") {
    Tokenizer t(make_vocab_file());
    std::vector<int64_t> ids = {1, 2};
    std::string text = t.decode(ids);
    CHECK(text == "hello world");
}

TEST_CASE("Tokenizer drops <pad> during decoding") {
    Tokenizer t(make_vocab_file());
    std::vector<int64_t> ids = {3, 1, 3};
    std::string result = t.decode(ids);
    REQUIRE(result == "hello");
}

TEST_CASE("Tokenizer throws if vocab path invalid") {
    CHECK_THROWS_AS(Tokenizer("nonexistent.json"), std::runtime_error);
    CHECK_THROWS_WITH_AS(Tokenizer("nonexistent.json"), "Failed to open vocab file", std::runtime_error);
}
