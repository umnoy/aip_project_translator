#include "../include/online_translators.hpp"
#include "doctest.h"
#include <fstream>
#include <filesystem>

TEST_SUITE("Online Translators Tests") {
    TEST_CASE("Constructor with missing config file") {
        REQUIRE_THROWS_AS(
            OnlineTranslatorsManager("nonexistent.json"),
            std::runtime_error
        );
    }

    TEST_CASE("Empty text translation") {
        std::ofstream config_file("test_config.json");
        config_file << R"({
            "yandex_translate": {
                "api_key": "test_yandex_key",
                "folder_id": "test_folder_id"
            },
            "libretranslate": {
                "api_key": "test_libretranslate_key"
            },
            "deepl": {
                "api_key": "test_deepl_key"
            }
        })";
        config_file.close();

        OnlineTranslatorsManager manager("test_config.json");
        auto results = manager.GetTranslations("", "en", "ru");
        
        REQUIRE_FALSE(results.empty());
        for (const auto& result : results) {
            REQUIRE_FALSE(result.success);
            CHECK(result.translated_text.empty());
            CHECK_FALSE(result.error_message.empty());
            WARN(result.error_message.length() > 10);
        }

        std::filesystem::remove("test_config.json");
    }

    TEST_CASE("Invalid languages") {
        std::ofstream config_file("test_config.json");
        config_file << R"({
            "yandex_translate": {
                "api_key": "test_yandex_key",
                "folder_id": "test_folder_id"
            },
            "libretranslate": {
                "api_key": "test_libretranslate_key"
            },
            "deepl": {
                "api_key": "test_deepl_key"
            }
        })";
        config_file.close();

        OnlineTranslatorsManager manager("test_config.json");
        auto results = manager.GetTranslations("Hello", "invalid", "invalid");
        
        REQUIRE_FALSE(results.empty());
        for (const auto& result : results) {
            REQUIRE_FALSE(result.success);
            CHECK(result.translated_text.empty());
            CHECK_FALSE(result.error_message.empty());
            WARN(result.error_message.length() > 10);
        }

        std::filesystem::remove("test_config.json");
    }

    TEST_CASE("Long text translation") {
        std::ofstream config_file("test_config.json");
        config_file << R"({
            "yandex_translate": {
                "api_key": "test_yandex_key",
                "folder_id": "test_folder_id"
            },
            "libretranslate": {
                "api_key": "test_libretranslate_key"
            },
            "deepl": {
                "api_key": "test_deepl_key"
            }
        })";
        config_file.close();

        OnlineTranslatorsManager manager("test_config.json");
        std::string long_text(1000, 'a');
        auto results = manager.GetTranslations(long_text, "en", "ru");
        
        REQUIRE_FALSE(results.empty());
        for (const auto& result : results) {
            if (result.success) {
                REQUIRE_FALSE(result.translated_text.empty());
                CHECK(result.error_message.empty());
                WARN(result.translated_text.length() > 100);
            } else {
                CHECK(result.translated_text.empty());
                CHECK_FALSE(result.error_message.empty());
                WARN(result.error_message.length() > 10);
            }
        }

        std::filesystem::remove("test_config.json");
    }

    TEST_CASE("Special characters translation") {
        std::ofstream config_file("test_config.json");
        config_file << R"({
            "yandex_translate": {
                "api_key": "test_yandex_key",
                "folder_id": "test_folder_id"
            },
            "libretranslate": {
                "api_key": "test_libretranslate_key"
            },
            "deepl": {
                "api_key": "test_deepl_key"
            }
        })";
        config_file.close();

        OnlineTranslatorsManager manager("test_config.json");
        std::string text = "Hello! @#$%^&*()_+{}|:\"<>?[]\\;',./~`";
        auto results = manager.GetTranslations(text, "en", "ru");
        
        REQUIRE_FALSE(results.empty());
        for (const auto& result : results) {
            if (result.success) {
                REQUIRE_FALSE(result.translated_text.empty());
                CHECK(result.error_message.empty());
                WARN(result.translated_text.length() > 10);
            } else {
                CHECK(result.translated_text.empty());
                CHECK_FALSE(result.error_message.empty());
                WARN(result.error_message.length() > 10);
            }
        }

        std::filesystem::remove("test_config.json");
    }

    TEST_CASE("Unicode characters translation") {
        std::ofstream config_file("test_config.json");
        config_file << R"({
            "yandex_translate": {
                "api_key": "test_yandex_key",
                "folder_id": "test_folder_id"
            },
            "libretranslate": {
                "api_key": "test_libretranslate_key"
            },
            "deepl": {
                "api_key": "test_deepl_key"
            }
        })";
        config_file.close();

        OnlineTranslatorsManager manager("test_config.json");
        std::string text = "Hello! Привет! 你好!";
        auto results = manager.GetTranslations(text, "en", "ru");
        
        REQUIRE_FALSE(results.empty());
        for (const auto& result : results) {
            if (result.success) {
                REQUIRE_FALSE(result.translated_text.empty());
                CHECK(result.error_message.empty());
                WARN(result.translated_text.length() > 10);
            } else {
                CHECK(result.translated_text.empty());
                CHECK_FALSE(result.error_message.empty());
                WARN(result.error_message.length() > 10);
            }
        }

        std::filesystem::remove("test_config.json");
    }
}

int main(int argc, char** argv) {
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
} 