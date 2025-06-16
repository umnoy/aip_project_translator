cmake_minimum_required(VERSION 3.16)

project(c_project VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0167 OLD)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network Core)
find_package(Boost REQUIRED COMPONENTS system json url)
find_package(OpenSSL REQUIRED)

set(ONNXRUNTIME_LIB_DIR "/home/kali/Desktop/project_translator/src/core/lib")
set(ONNXRUNTIME_INCLUDE_DIR "/home/kali/Desktop/project_translator/src/core/include")

if(NOT EXISTS "${ONNXRUNTIME_INCLUDE_DIR}")
    message(FATAL_ERROR "ONNX Runtime include directory not found: ${ONNXRUNTIME_INCLUDE_DIR}")
endif()
if(NOT EXISTS "${ONNXRUNTIME_LIB_DIR}")
    message(FATAL_ERROR "ONNX Runtime lib directory not found: ${ONNXRUNTIME_LIB_DIR}")
endif()

include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ../core/src/tokenizer/tokenizer.cpp
    ../core/src/translator/translator.cpp
    ../requests/src/http_client.cpp
    ../requests/src/online_translators.cpp
    ../requests/src/utils.cpp
)

add_executable(c_project ${PROJECT_SOURCES})

target_include_directories(c_project PRIVATE
    ${CMAKE_SOURCE_DIR}/../core/src
    ${CMAKE_SOURCE_DIR}/../requests/src
    ${ONNXRUNTIME_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(c_project PRIVATE
    Qt5::Widgets
    Qt5::Network
    Qt5::Core
    nlohmann_json::nlohmann_json
    Boost::system
    Boost::json
    Boost::url
    OpenSSL::SSL
    OpenSSL::Crypto
)

find_library(ONNXRUNTIME_LIBRARY
    NAMES onnxruntime libonnxruntime
    PATHS ${ONNXRUNTIME_LIB_DIR}
    NO_DEFAULT_PATH
)
if(NOT ONNXRUNTIME_LIBRARY)
    message(FATAL_ERROR "ONNX Runtime library not found in ${ONNXRUNTIME_LIB_DIR}")
endif()

target_link_libraries(c_project PRIVATE ${ONNXRUNTIME_LIBRARY})

set_target_properties(c_project PROPERTIES
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS c_project
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
