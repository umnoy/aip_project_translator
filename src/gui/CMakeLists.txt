cmake_minimum_required(VERSION 3.16)

project(c_project VERSION 0.1 LANGUAGES CXX)

enable_testing()

option(BUILD_GUI_ONLY "Build only GUI without ONNX and other dependencies" OFF)

if(BUILD_GUI_ONLY)
    add_definitions(-DBUILD_GUI_ONLY) 
endif()

set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt@5")
set(Qt5_DIR "/opt/homebrew/opt/qt@5/lib/cmake/Qt5")

set(CMAKE_AUTOUIC ON) 
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOMOC_MOC_OPTIONS "-nn")
set(CMAKE_AUTOMOC_SEARCH_PATHS "")

set(ENV{QTDIR} "/opt/homebrew/opt/qt@5")
set(ENV{PATH} "/opt/homebrew/opt/qt@5/bin:$ENV{PATH}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0167 OLD)

set(Qt5Core_DIR "/opt/homebrew/opt/qt@5/lib/cmake/Qt5Core")
set(Qt5Gui_DIR "/opt/homebrew/opt/qt@5/lib/cmake/Qt5Gui")
set(Qt5Widgets_DIR "/opt/homebrew/opt/qt@5/lib/cmake/Qt5Widgets")
set(Qt5Network_DIR "/opt/homebrew/opt/qt@5/lib/cmake/Qt5Network")

add_definitions(-DQT_NO_VERSION_TAGGING)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network Core Test)

if(NOT BUILD_GUI_ONLY)
    find_package(Boost REQUIRED COMPONENTS system json url)
    find_package(OpenSSL REQUIRED)
    find_package(nlohmann_json 3.11.2 REQUIRED)
    
    if(APPLE)
        set(ONNXRUNTIME_LIB_DIR "${CMAKE_SOURCE_DIR}/../core/lib/macos")
        set(ONNXRUNTIME_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../core/include")
    else()
        set(ONNXRUNTIME_LIB_DIR "${CMAKE_SOURCE_DIR}/../core/lib")
        set(ONNXRUNTIME_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../core/include")
    endif()

    if(NOT EXISTS "${ONNXRUNTIME_INCLUDE_DIR}")
        message(FATAL_ERROR "ONNX Runtime include directory not found: ${ONNXRUNTIME_INCLUDE_DIR}")
    endif()
    if(NOT EXISTS "${ONNXRUNTIME_LIB_DIR}")
        message(FATAL_ERROR "ONNX Runtime lib directory not found: ${ONNXRUNTIME_LIB_DIR}")
    endif()
endif()

if(BUILD_GUI_ONLY)
    set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
    )
else()
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
endif()

add_executable(c_project ${PROJECT_SOURCES})

target_include_directories(c_project PRIVATE
    ${CMAKE_SOURCE_DIR}/../core/src
    ${CMAKE_SOURCE_DIR}/../requests/src
)

if(NOT BUILD_GUI_ONLY)
    target_include_directories(c_project PRIVATE
        ${ONNXRUNTIME_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}
    )
endif()

target_link_libraries(c_project PRIVATE
    Qt5::Widgets
    Qt5::Network
    Qt5::Core
)

if(NOT BUILD_GUI_ONLY)
    target_link_libraries(c_project PRIVATE
        nlohmann_json::nlohmann_json
        Boost::system
        Boost::json
        Boost::url
        OpenSSL::SSL
        OpenSSL::Crypto
    )

    if(APPLE)
        find_library(ONNXRUNTIME_LIBRARY
            NAMES onnxruntime libonnxruntime
            PATHS ${ONNXRUNTIME_LIB_DIR}
            NO_DEFAULT_PATH
        )
    else()
        find_library(ONNXRUNTIME_LIBRARY
            NAMES onnxruntime libonnxruntime
            PATHS ${ONNXRUNTIME_LIB_DIR}
            NO_DEFAULT_PATH
        )
    endif()

    if(NOT ONNXRUNTIME_LIBRARY)
        message(FATAL_ERROR "ONNX Runtime library not found in ${ONNXRUNTIME_LIB_DIR}")
    endif()

    target_link_libraries(c_project PRIVATE ${ONNXRUNTIME_LIBRARY})
endif()

if(WIN32)
    set_target_properties(c_project PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
elseif(APPLE)
    set_target_properties(c_project PROPERTIES 
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.c_project"
        MACOSX_BUNDLE_BUNDLE_NAME "c_project"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    )
endif()

include(GNUInstallDirs)
install(TARGETS c_project 
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} 
)

set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/doctest/share/cmake/doctest" ${CMAKE_PREFIX_PATH})

add_executable(gui_tests
    tests/mainwindow_test.cpp
    mainwindow.cpp
)

target_include_directories(gui_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}
    /opt/homebrew/opt/doctest/include/doctest
)

target_link_libraries(gui_tests PRIVATE 
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Test
)

if(BUILD_GUI_ONLY)
    target_compile_definitions(gui_tests PRIVATE BUILD_GUI_ONLY)
endif()

set_target_properties(c_project gui_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

add_test(NAME GuiTests COMMAND gui_tests) 