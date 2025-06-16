cmake_minimum_required(VERSION 3.16)
project(TranslationCore CXX)

# Установка директории сборки
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

# Включение FetchContent для загрузки зависимостей
include(FetchContent)

# Загрузка nlohmann_json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# Создание интерфейсной библиотеки для doctest
add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/doctest
)

# Добавление поддиректории с тестами
add_subdirectory(tests)

# Основной проект
set(SOURCES
    src/tokenizer/tokenizer.cpp
    src/translator/translator.cpp
)

# Библиотека
add_library(translation_core STATIC ${SOURCES})

# Заголовочные файлы
target_include_directories(translation_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Линковка библиотек
target_link_libraries(translation_core PUBLIC
    nlohmann_json::nlohmann_json
    ${ONNXRUNTIME_LIBRARY}
)