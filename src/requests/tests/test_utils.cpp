#include "../include/utils.hpp"
#include "doctest.h"
#include <fstream>
#include <filesystem>

TEST_SUITE("Utils Tests") {
    TEST_CASE("Load all keys") {
        // Создаем временный файл конфигурации
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

        auto keys = load_api_keys_from_file("test_config.json");
        
        REQUIRE_FALSE(keys.yandex_key.empty());
        REQUIRE_FALSE(keys.yandex_folder_id.empty());
        REQUIRE_FALSE(keys.libretranslate_key.empty());
        REQUIRE_FALSE(keys.deepl_key.empty());

        CHECK_EQ(keys.yandex_key, "test_yandex_key");
        CHECK_EQ(keys.yandex_folder_id, "test_folder_id");
        CHECK_EQ(keys.libretranslate_key, "test_libretranslate_key");
        CHECK_EQ(keys.deepl_key, "test_deepl_key");

        WARN(keys.yandex_key.length() > 10);
        WARN(keys.libretranslate_key.length() > 10);
        WARN(keys.deepl_key.length() > 10);

        std::filesystem::remove("test_config.json");
    }

    TEST_CASE("File not found") {
        REQUIRE_THROWS_AS(
            load_api_keys_from_file("nonexistent.json"),
            std::runtime_error
        );
    }

    TEST_CASE("Invalid JSON") {
        std::ofstream config_file("invalid_config.json");
        config_file << "invalid json content";
        config_file.close();

        REQUIRE_THROWS_AS(
            load_api_keys_from_file("invalid_config.json"),
            std::runtime_error
        );

        std::filesystem::remove("invalid_config.json");
    }

    TEST_CASE("Missing sections") {
        std::ofstream config_file("partial_config.json");
        config_file << R"({
            "yandex_translate": {
                "api_key": "test_yandex_key"
            }
        })";
        config_file.close();

        auto keys = load_api_keys_from_file("partial_config.json");
        
        REQUIRE_FALSE(keys.yandex_key.empty());
        CHECK_EQ(keys.yandex_key, "test_yandex_key");
        
        CHECK(keys.yandex_folder_id.empty());
        CHECK(keys.libretranslate_key.empty());
        CHECK(keys.deepl_key.empty());

        WARN(keys.yandex_key.length() > 10);

        std::filesystem::remove("partial_config.json");
    }

    TEST_CASE("Empty values") {
        std::ofstream config_file("empty_config.json");
        config_file << R"({
            "yandex_translate": {
                "api_key": "",
                "folder_id": ""
            },
            "libretranslate": {
                "api_key": ""
            },
            "deepl": {
                "api_key": ""
            }
        })";
        config_file.close();

        auto keys = load_api_keys_from_file("empty_config.json");
        
        REQUIRE(keys.yandex_key.empty());
        REQUIRE(keys.yandex_folder_id.empty());
        REQUIRE(keys.libretranslate_key.empty());
        REQUIRE(keys.deepl_key.empty());

        WARN(keys.yandex_key.length() == 0);
        WARN(keys.libretranslate_key.length() == 0);
        WARN(keys.deepl_key.length() == 0);

        std::filesystem::remove("empty_config.json");
    }
}

int main(int argc, char** argv) {
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
} 