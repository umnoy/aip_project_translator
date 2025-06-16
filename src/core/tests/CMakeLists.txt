cmake_minimum_required(VERSION 3.10)
project(TranslationTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(doctest REQUIRED)
if(NOT doctest_FOUND)
    message(FATAL_ERROR "doctest not found. Please install doctest or provide its path.")
endif()


find_package(nlohmann_json REQUIRED)
if(NOT nlohmann_json_FOUND)
    message(FATAL_ERROR "nlohmann_json not found. Please install nlohmann_json.")
endif()


find_library(ONNXRUNTIME_LIBRARY onnxruntime)
if(NOT ONNXRUNTIME_LIBRARY)
    message(FATAL_ERROR "ONNX Runtime library not found. Please install ONNX Runtime.")
endif()


set(TEST_SOURCES
    tests_tokenizer.cpp
    tests_translator.cpp
)


add_executable(translation_tests ${TEST_SOURCES})


target_sources(translation_tests PRIVATE
    ../tokenizer/tokenizer.cpp
    ../translator/translator.cpp
)


target_include_directories(translation_tests PRIVATE
    ../tokenizer
    ../translator
)


target_link_libraries(translation_tests PRIVATE
    doctest::doctest
    nlohmann_json::nlohmann_json
    ${ONNXRUNTIME_LIBRARY}
)


add_test(NAME TranslationTests COMMAND translation_tests)