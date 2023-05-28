cmake_minimum_required (VERSION 3.5)
project(client)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/../digestpp/)

target_compile_definitions(${PROJECT_NAME} PUBLIC BOOST_ASIO_HAS_IO_URING=1)
target_link_libraries(${PROJECT_NAME} lib::service uring)
