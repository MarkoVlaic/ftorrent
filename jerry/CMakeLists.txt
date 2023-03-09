cmake_minimum_required (VERSION 3.5)
project(Jerry)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(${PROJECT_NAME} src/jerry.cpp src/lexer.cpp src/parser.cpp src/is/string_input_source.cpp src/is/file_input_source.cpp)
add_library(lib::jerry ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)