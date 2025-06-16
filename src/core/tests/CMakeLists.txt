# CMakeLists.txt для сборки тестов в src/core/tests/

cmake_minimum_required(VERSION 3.16)
project(TranslationTests CXX)

# Установка директории сборки
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/../build)

# Включение тестов
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # Исходные файлы тестов
    set(TEST_SOURCES
        test_tokenizer.cpp
        test_translator.cpp
    )

    # Исполняемый файл для тестов
    add_executable(translation_tests ${TEST_SOURCES})

    # Исходные файлы проекта
    target_sources(translation_tests PRIVATE
        ../src/tokenizer/tokenizer.cpp
        ../src/translator/translator.cpp
    )

    # Заголовочные файлы
    target_include_directories(translation_tests PRIVATE
        ../src/tokenizer
        ../src/translator
        ${ONNXRUNTIME_INCLUDE_DIR}
    )

    # Линковка библиотек
    target_link_libraries(translation_tests PRIVATE
        doctest::doctest
        nlohmann_json::nlohmann_json
        ${ONNXRUNTIME_LIBRARY}
    )

    # Регистрация тестов
    add_test(NAME TranslationTests COMMAND translation_tests)
endif()