#include "online_translators.hpp" 
#include <iostream>
#include <vector> 
#include <string> 


int main() { 
    std::string config_filepath = "api_keys.json";
    std::cout << "DEBUG: Entering main" << std::endl;
    try { 
        std::cout << "DEBUG: Before OnlineTranslatorsManager constructor" << std::endl;
        OnlineTranslatorsManager manager(config_filepath);
        std::cout << "DEBUG: After OnlineTranslatorsManager constructor" << std::endl;

        std::string text_to_translate = "Text for translate"; //Должны передаваться из граф интерфейса
        std::string source_lang = "en";
        std::string target_lang = "ru";

        std::cout << "DEBUG: Before GetTranslations call" << std::endl;
        // Запрос перевода
        std::vector<TranslationResult> translations = manager.GetTranslations( 
            text_to_translate, // Переданные параметры из интерфейса
            source_lang,       
            target_lang        
        );
        std::cout << "DEBUG: After GetTranslations call" << std::endl;

        // Полученные переводы
        std::cout << "DEBUG: Displaying results" << std::endl;
        for (const auto& result : translations) {
            std::cout << "From " << result.translator_name << ": ";
            if (result.success) {
                std::cout << "\"" << result.translated_text << "\"";
            } else {
                std::cout << "Error (" << result.error_message << ")";
            }
            std::cout << std::endl;
        }
        std::cout << "DEBUG: Finished displaying results" << std::endl;

    } catch (const std::exception& e) { 
        std::cerr << "Critical program error: " << e.what() << std::endl; 
        return 1; // Возвращаем код ошибки 1
    }
    std::cout << "DEBUG: Exiting main successfully" << std::endl;
    return 0; // Возвращаем код успеха 0
}