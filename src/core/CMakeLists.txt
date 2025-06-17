cmake_minimum_required(VERSION 3.15)
project(TranslatorProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(ONNXRUNTIME_LIB onnxruntime HINTS /usr/lib /usr/local/lib)
find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_cxx_api.h HINTS /usr/include/onnxruntime /usr/local/include/onnxruntime)

if (NOT ONNXRUNTIME_LIB OR NOT ONNXRUNTIME_INCLUDE_DIR)
    message(FATAL_ERROR "ONNX Runtime not found. Please install it or specify paths manually.")
endif()


add_library(tokenizer
    ./src/tokenizer/tokenizer.cpp
    ./src/tokenizer/tokenizer.hpp
)

add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE libs/doctest)


add_library(translator
    ./src/translator/translator.cpp
    ./src/translator/traslator.hpp
)

add_executable(run_tests
    ./tests/test_main.cpp
    ./tests/tokenizer_test.cpp
    ./tests/translator_test.cpp
)

target_link_libraries(run_tests tokenizer translator doctest)
target_include_directories(run_tests PRIVATE tokenizer translator)
