#include "tokenizer.hpp"
#include <cctype>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

const std::string Tokenizer::spm_space = "▁";

Tokenizer::Tokenizer(const std::string &vocab_path) {
    std::ifstream file(vocab_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open vocab file: " + vocab_path);
    }

    json j;
    file >> j;

    for (auto &[token, id] : j.items()) {
        token_to_id[token] = id;
        id_to_token[id] = token;
    }
}

std::string Tokenizer::normalize(const std::string &text) {
    std::string result;

    for (char c : text) {
        result += isspace(static_cast<unsigned char>(c)) ? ' ' : c;
    }
    return result;
}

std::vector<int64_t> Tokenizer::encode(const std::string &input_text) {
    std::vector<int64_t> tokens;
    std::istringstream stream(normalize(input_text));
    std::string word;

    while (stream >> word) {
        std::string current = spm_space + word;
        size_t pos = 0;

        while (pos < current.size()) {
            size_t len = current.size();
            bool found = false;

            while (len > 0) {
                std::string sub = current.substr(pos, len);
                if (token_to_id.count(sub)) {
                    tokens.push_back(token_to_id[sub]);
                    pos += len;
                    found = true;
                    break;
                }
                --len;
            }

            if (!found) {
                tokens.push_back(token_to_id.at("<unk>"));
                break;
            }
        }
    }

    return tokens;
}

std::string Tokenizer::decode(const std::vector<int64_t> &token_ids) {
    std::string result;

    for (size_t i = 0; i < token_ids.size(); ++i) {
        int64_t id = token_ids[i];

        if (!id_to_token.count(id))
            continue;

        std::string token = id_to_token[id];
        if (token == "<pad>" || token == "<s>" || token == "</s>")
            continue;

        if (token.rfind(spm_space, 0) == 0) {
            if (!result.empty())
                result += " ";
            result += token.substr(3);
        } else {
            result += token;
        }
    }

    return result;
}
