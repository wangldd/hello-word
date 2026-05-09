cmake_minimum_required(VERSION 3.16)
project(protocol_gateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(protocol_gateway
    src/adapters.cpp
    src/engine.cpp
    src/http_server.cpp
    src/json.cpp
    src/registry.cpp
    src/store.cpp
    src/models.cpp
)

target_include_directories(protocol_gateway PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(protocol_gateway_server src/main.cpp)
target_link_libraries(protocol_gateway_server PRIVATE protocol_gateway)

add_executable(protocol_gateway_tests tests/test_conversion.cpp)
target_link_libraries(protocol_gateway_tests PRIVATE protocol_gateway)

enable_testing()
add_test(NAME protocol_gateway_tests COMMAND protocol_gateway_tests)
