cmake_minimum_required(VERSION 3.15)
project(TranslatorProject LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)


add_library(tokenizer
    tokenizer/tokenizer.cpp
    tokenizer/tokenizer.hpp
)

add_library(translator
    translator/translator.cpp
    translator/translator.hpp
)


file(GLOB TEST_SOURCES tests/*.cpp)

add_executable(run_tests ${TEST_SOURCES})
target_link_libraries(run_tests tokenizer translator doctest::doctest)
target_include_directories(run_tests PRIVATE tokenizer translator)
