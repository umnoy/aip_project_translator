cmake_minimum_required(VERSION 3.10)

project(TranslatorProject CXX) 

set(CMAKE_CXX_STANDARD 17)
add_definitions(-D_WIN32_WINNT=0x0601)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_EXTENSIONS OFF) 

find_package(Boost REQUIRED CONFIG COMPONENTS system asio beast json)
find_package(OpenSSL REQUIRED)

include_directories(include)

add_executable(TranslatorApp
    main.cpp
    src/http_client.cpp
    src/online_translators.cpp
    src/utils.cpp 
)

target_link_libraries(TranslatorApp
    PRIVATE
    Boost::system
    Boost::asio
    Boost::beast
    Boost::json
    OpenSSL::SSL 
    OpenSSL::Crypto 
)
