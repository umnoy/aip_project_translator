// Отключение аннотаций SAL
#define _SAL_VERSION 20
#define _Out_
#define _In_
#define _In_opt_
#define _Frees_ptr_opt_
#define _Check_return_
#undef _Success_  // Отменяем предыдущее определение
#define _Success_(expr)  // Переопределяем макрос
#define _Ret_maybenull_
#define _Null_terminated_
#define _Outptr_opt_result_maybenull_


#include "../tokenizer/tokenizer.hpp"
#include <onnxruntime_cxx_api.h>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief Структура для представления луча в алгоритме поиска по лучу (beam search).
 */
struct Beam {
    std::vector<int64_t> tokens; ///< Последовательность токенов в луче.
    float score;                 ///< Оценка луча (логарифм вероятности).

    /**
     * @brief Оператор сравнения для использования в priority_queue.
     * @param other Другой луч для сравнения.
     * @return true, если текущий луч имеет меньший score.
     */
    bool operator<(const Beam &other) const { return score < other.score; }
};

/**
 * @brief Класс для перевода текста с использованием моделей ONNX и токенизатора.
 *
 * Этот класс реализует перевод текста с помощью энкодера и декодера ONNX,
 * используя алгоритм поиска по лучу (beam search) для генерации перевода.
 */
class Translator {
public:
    /**
     * @brief Конструктор, инициализирующий переводчик с токенизатором и моделями.
     * @param tokenizer Токенизатор для обработки текста.
     * @param encoder_path Путь к ONNX-модели энкодера.
     * @param decoder_path Путь к ONNX-модели декодера.
     * @param pad_token_id Идентификатор токена заполнения (<pad>).
     * @param eos_token_id Идентификатор токена конца последовательности (<eos>).
     * @param max_length Максимальная длина генерируемой последовательности.
     * @param beam_width Количество лучей в алгоритме beam search.
     * @throws Ort::Exception Если не удалось загрузить модели ONNX.
     *
     * Пример:
     *   Tokenizer tokenizer("vocab.json");
     *   Translator translator(tokenizer, "encoder.onnx", "decoder.onnx", 0, 2, 50, 3);
     */
    Translator(const Tokenizer tokenizer, const std::string &encoder_path,
               const std::string &decoder_path, int pad_token_id, int eos_token_id,
               int max_length = 50, int beam_width = 3);

    /**
     * @brief Переводит входной текст.
     * @param input Входной текст для перевода.
     * @return Переведённый текст.
     *
     * Пример:
     *   std::string input = "Hello World";
     *   translator.run(input) // возвращает, например, "Hola Mundo"
     */
    std::string run(const std::string &input);

private:
    Ort::Env env;                               ///< Окружение ONNX Runtime.
    Ort::Session encoder_session;               ///< Сессия для энкодера ONNX.
    Ort::Session decoder_session;               ///< Сессия для декодера ONNX.
    Ort::SessionOptions session_options;        ///< Опции сессии ONNX.
    Ort::AllocatorWithDefaultOptions allocator; ///< Аллокатор ONNX.

    int pad_token_id; ///< Идентификатор токена заполнения.
    int eos_token_id; ///< Идентификатор токена конца последовательности.
    int max_length;   ///< Максимальная длина генерируемой последовательности.
    int beam_width;   ///< Количество лучей в beam search.
    Tokenizer tokenizer; ///< Токенизатор для обработки текста.

    /**
     * @brief Кодирует входной текст в скрытое состояние энкодера.
     * @param input_ids Вектор идентификаторов токенов.
     * @return Скрытое состояние энкодера.
     *
     * Пример:
     *   std::vector<int64_t> input_ids = {101, 102};
     *   auto hidden = encode_input(input_ids); // возвращает вектор скрытых состояний
     */
    std::vector<float> encode_input(const std::vector<int64_t> &input_ids);

    /**
     * @brief Выполняет один шаг декодирования.
     * @param input_ids Текущая последовательность токенов декодера.
     * @param encoder_input_ids Входные токены энкодера (маска внимания).
     * @param encoder_hidden_state Скрытое состояние энкодера.
     * @return Логиты для следующего токена.
     *
     * Пример:
     *   std::vector<int64_t> input_ids = {0};
     *   std::vector<int64_t> enc_ids = {101, 102};
     *   std::vector<float> enc_hidden = {...};
     *   auto logits = decode_step(input_ids, enc_ids, enc_hidden); // возвращает логиты
     */
    std::vector<float> decode_step(const std::vector<int64_t> &input_ids,
                                  const std::vector<int64_t> &encoder_input_ids,
                                  const std::vector<float> &encoder_hidden_state);

    /**
     * @brief Декодирует идентификаторы токенов в текст.
     * @param ids Вектор идентификаторов токенов.
     * @return Декодированный текст.
     *
     * Пример:
     *   std::vector<int64_t> ids = {101, 102};
     *   decode_ids(ids) // возвращает, например, "Hello World"
     */
    std::string decode_ids(const std::vector<int64_t> &ids);

    /**
     * @brief Применяет softmax к логитам для получения вероятностей.
     * @param logits Вектор логитов.
     * @return Вектор вероятностей.
     *
     * Пример:
     *   std::vector<float> logits = {0.1, 0.2, 0.7};
     *   auto probs = softmax(logits); // возвращает нормализованные вероятности
     */
    std::vector<float> softmax(const std::vector<float> &logits);

    /**
     * @brief Выбирает k токенов с наибольшими вероятностями.
     * @param probs Вектор вероятностей.
     * @param k Количество выбираемых токенов.
     * @return Вектор пар {токен, вероятность}, отсортированный по убыванию.
     *
     * Пример:
     *   std::vector<float> probs = {0.1, 0.7, 0.2};
     *   auto top = top_k(probs, 2); // возвращает, например, {{1, 0.7}, {2, 0.2}}
     */
    std::vector<std::pair<int64_t, float>> top_k(const std::vector<float> &probs, int k);
};
