#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <locale>
#include <clocale>
#include "tokenizer/tokenizer.hpp"
#include "translator/traslator.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    try {
        std::locale::global(std::locale(""));
        cout << "Локаль успешно установлена." << endl;
    } catch (const std::runtime_error& e) {
        cerr << "Ошибка установки локали: " << e.what() << endl;
        try {
             std::locale::global(std::locale("en_US.UTF-8"));
             cout << "Локаль установлена на en_US.UTF-8." << endl;
        } catch (const std::runtime_error& e2) {
             cerr << "Не удалось установить локаль en_US.UTF-8: " << e2.what() << endl;
             cerr << "Возможны проблемы с кодировкой." << endl;
        }
    }
    std::ios_base::sync_with_stdio(false);

    int choice;
    cout << "Выберите опцию перевода:" << endl;
    cout << "1 - С английского на русский (en-ru)" << endl;
    cout << "2 - С русского на английский (ru-en)" << endl;
    cout << "Ваш выбор: ";
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    std::string vocab_path;
    std::string encoder_path;
    std::string decoder_path;

    if (choice == 1) {
        vocab_path = "../models/opus-mt-en-ru/vocab.json";
        encoder_path = "../models/opus-mt-en-ru/encoder_model.onnx";
        decoder_path = "../models/opus-mt-en-ru/decoder_model.onnx";
        cout << "Выбран перевод en-ru." << endl;
    } else if (choice == 2) {
        vocab_path = "../models/opus-mt-ru-en/vocab.json";
        encoder_path = "../models/opus-mt-ru-en/encoder_model.onnx";
        decoder_path = "../models/opus-mt-ru-en/decoder_model.onnx";
        cout << "Выбран перевод ru-en." << endl;
    } else {
        cerr << "Неверный выбор опции перевода. Завершение программы." << endl;
        return 1;
    }

    int pad_token_id = 62517;
    int eos_token_id = 0;
    int max_length = 20;
    int beam_width = 3;

    Tokenizer tokenizer(vocab_path);
    Translator translator(tokenizer, encoder_path, decoder_path, pad_token_id, eos_token_id, max_length, beam_width);

    string input_text;
    cout << "Введите текст для перевода (или 'exit' для выхода):" << endl;
    while (true) {
        cout << "> ";
        getline(cin, input_text);
        if (input_text == "exit") {
            break;
        }
        if (input_text.empty()) {
            cout << "Введите текст." << endl;
            continue;
        }
        string result = translator.run(input_text);
        cout << "Переведено: " << result << endl;
    }
    return 0;
}
