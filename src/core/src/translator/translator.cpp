#include "../tokenizer/tokenizer.hpp"
#include "traslator.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <onnxruntime_c_api.h>
#include <onnxruntime_cxx_api.h>
#include <queue>
#include <string>
#include <utility>
#include <vector>

Translator::Translator(const Tokenizer tokenizer, const std::string &encoder_path,
                      const std::string &decoder_path, int pad_token_id,
                      int eos_token_id, int max_length, int beam_width)
    : env(ORT_LOGGING_LEVEL_WARNING, "Translator"), session_options(),
      encoder_session(nullptr), decoder_session(nullptr),
      pad_token_id(pad_token_id), eos_token_id(eos_token_id),
      max_length(max_length), beam_width(beam_width), tokenizer(tokenizer) {
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    encoder_session = Ort::Session(env, encoder_path.c_str(), session_options);
    decoder_session = Ort::Session(env, decoder_path.c_str(), session_options);
}

std::string Translator::run(const std::string &input) {
    std::vector<int64_t> input_ids = tokenizer.encode(input);
    std::vector<int64_t> attention_mask(input_ids.size(), 1);
    std::vector<float> encoder_hidden = encode_input(input_ids);

    std::priority_queue<Beam> beams;
    beams.push({{pad_token_id}, 0.0f});
    std::vector<Beam> completed_beams;

    for (int step = 0; step < max_length; ++step) {
        std::priority_queue<Beam> new_beams;

        while (!beams.empty()) {
            Beam beam = beams.top();
            beams.pop();

            if (!beam.tokens.empty() && beam.tokens.back() == eos_token_id) {
                completed_beams.push_back(beam);
                continue;
            }

            std::vector<float> logits = decode_step(beam.tokens, attention_mask, encoder_hidden);
            std::vector<float> probs = softmax(logits);
            auto topk = top_k(probs, beam_width);

            for (auto &[token_id, prob] : topk) {
                Beam new_beam = beam;
                new_beam.tokens.push_back(token_id);
                new_beam.score += std::log(prob + 1e-8f);
                new_beams.push(new_beam);
                if ((int)new_beams.size() > beam_width) {
                    std::priority_queue<Beam> tmp;
                    while ((int)tmp.size() < beam_width && !new_beams.empty()) {
                        tmp.push(new_beams.top());
                        new_beams.pop();
                    }
                    new_beams = std::move(tmp);
                }
            }   
            }
        }

        if (new_beams.empty())
            break;

        beams = new_beams;
    }

    Beam best;
    if (!completed_beams.empty()) {
        best = *std::max_element(completed_beams.begin(), completed_beams.end(),
                            [](const Beam &a, const Beam &b) { return a.score < b.score; });
    } else if (!beams.empty()) {
        best = beams.top();
    } else {
        return "";
    }

    return tokenizer.decode(best.tokens);
}

std::vector<float> Translator::encode_input(const std::vector<int64_t> &input_ids) {
    std::vector<int64_t> attention_mask(input_ids.size(), 1);
    std::array<int64_t, 2> input_shape{1, (int64_t)input_ids.size()};

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

    Ort::Value input_tensor = Ort::Value::CreateTensor<int64_t>(
        memory_info, const_cast<int64_t *>(input_ids.data()), input_ids.size(),
        input_shape.data(), 2);

    Ort::Value mask_tensor = Ort::Value::CreateTensor<int64_t>(
        memory_info, attention_mask.data(), attention_mask.size(),
        input_shape.data(), 2);

    const char *input_names[] = {"input_ids", "attention_mask"};
    const char *output_names[] = {"last_hidden_state"};

    std::array<Ort::Value, 2> inputs = {std::move(input_tensor), std::move(mask_tensor)};

    auto output_tensors = encoder_session.Run(Ort::RunOptions{nullptr}, input_names, inputs.data(),
                                             inputs.size(), output_names, 1);

    float *output_data = output_tensors.front().GetTensorMutableData<float>();
    size_t size = output_tensors.front().GetTensorTypeAndShapeInfo().GetElementCount();

    return std::vector<float>(output_data, output_data + size);
}

std::vector<float> Translator::decode_step(const std::vector<int64_t> &input_ids,
                                     const std::vector<int64_t> &encoder_input_ids,
                                     const std::vector<float> &encoder_hidden_state) {
    std::array<int64_t, 2> dec_shape{1, static_cast<int64_t>(input_ids.size())};
    std::array<int64_t, 2> enc_mask_shape{1, static_cast<int64_t>(encoder_input_ids.size())};
    std::array<int64_t, 3> enc_hidden_shape{1, static_cast<int64_t>(encoder_input_ids.size()),
                                       static_cast<int64_t>(encoder_hidden_state.size() /
                                                            encoder_input_ids.size())};

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);

    Ort::Value dec_input = Ort::Value::CreateTensor<int64_t>(
        memory_info, const_cast<int64_t *>(input_ids.data()), input_ids.size(),
        dec_shape.data(), 2);

    Ort::Value enc_mask = Ort::Value::CreateTensor<int64_t>(
        memory_info, const_cast<int64_t *>(encoder_input_ids.data()),
        encoder_input_ids.size(), enc_mask_shape.data(), 2);

    Ort::Value enc_hidden = Ort::Value::CreateTensor<float>(
        memory_info, const_cast<float *>(encoder_hidden_state.data()),
        encoder_hidden_state.size(), enc_hidden_shape.data(),
        enc_hidden_shape.size());

    const char *input_names[] = {"encoder_attention_mask", "input_ids", "encoder_hidden_states"};
    const char *output_names[] = {"logits"};

    std::array<Ort::Value, 3> inputs = {std::move(enc_mask), std::move(dec_input), std::move(enc_hidden)};

    auto output_tensors = decoder_session.Run(Ort::RunOptions{nullptr}, input_names, inputs.data(),
                                             inputs.size(), output_names, 1);

    float *logits_data = output_tensors.front().GetTensorMutableData<float>();
    size_t vocab_size = output_tensors.front().GetTensorTypeAndShapeInfo().GetShape().back();

    return std::vector<float>(logits_data + (input_ids.size() - 1) * vocab_size,
                         logits_data + input_ids.size() * vocab_size);
}

std::vector<float> Translator::softmax(const std::vector<float> &logits) {
    float max_logit = *std::max_element(logits.begin(), logits.end());
    std::vector<float> exps(logits.size());
    for (size_t i = 0; i < logits.size(); ++i)
        exps[i] = std::exp(logits[i] - max_logit);
    float sum = std::accumulate(exps.begin(), exps.end(), 0.0f);
    for (float &x : exps)
        x /= sum;
    return exps;
}

std::vector<std::pair<int64_t, float>> Translator::top_k(const std::vector<float> &probs, int k) {
    std::vector<std::pair<int64_t, float>> topk;
    for (int64_t i = 0; i < (int64_t)probs.size(); ++i)
        topk.emplace_back(i, probs[i]);
    std::partial_sort(topk.begin(), topk.begin() + k, topk.end(),
                 [](const auto &a, const auto &b) { return a.second > b.second; });
    topk.resize(k);
    return topk;
}

std::string Translator::decode_ids(const std::vector<int64_t> &ids) {
    return tokenizer.decode(ids);
}
