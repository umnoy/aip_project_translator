#ifndef ONLINE_TRANSLATORS_HPP
#define ONLINE_TRANSLATORS_HPP

#include "http_client.hpp"

#include <string>
#include <vector>
#include <stdexcept>

#include <boost/json.hpp>
#include <boost/system/error_code.hpp>

namespace json = boost::json;

struct ApiKeys {
    std::string yandex_key;
    std::string yandex_folder_id;
    std::string libretranslate_key;
    std::string deepl_key;
};

struct TranslationResult {
    std::string translator_name; 
    std::string translated_text; 
    bool success;
    std::string error_message;

    TranslationResult(const std::string& name, const std::string& text, bool s, const std::string& error = "")
        : translator_name(name), translated_text(text), success(s), error_message(error) {}
};

class OnlineTranslatorsManager {  
private: 
    HttpClient http_client_;
    ApiKeys api_keys_;

public: 

    OnlineTranslatorsManager(const std::string &config_filepath);

    std::vector<TranslationResult> GetTranslations( 
        const std::string& text,
        const std::string& source_lang,
        const std::string& target_lang
    );

private: 

    TranslationResult GetTranslationFromOne( 
        const std::string& translator_name,
        const std::string& url,
        const std::string& target, 
        http::verb method,
        const std::string& request_body,
        const std::vector<std::pair<std::string, std::string>>& headers
    );

    std::pair<std::string, std::vector<std::pair<std::string, std::string>>> 
        PrepareYandexRequest(const std::string& text, const std::string& source_lang, const std::string& target_lang);

    std::string ParseYandexResponse(const std::string& response_body);

    std::pair<std::string, std::vector<std::pair<std::string, std::string>>>
        PrepareLibreTranslateRequest(const std::string& text, const std::string& source_lang, const std::string& target_lang);

    std::string ParseLibreTranslateResponse(const std::string& response_body);

    std::pair<std::string, std::vector<std::pair<std::string, std::string>>>
        PrepareDeepLRequest(const std::string& text, const std::string& source_lang, const std::string& target_lang);
    
    std::string ParseDeepLResponse(const std::string& response_body);
}; 

#endif 
