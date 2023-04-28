cmake_minimum_required (VERSION 3.5)
project(Service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost REQUIRED system thread regex filesystem)

if(Boost_FOUND)
    message("Boost found")
    include_directories(PUBLIC ${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Cannot find boost")
endif()

add_library(${PROJECT_NAME} src/service.cpp include/service/bencode.h src/serialization.cpp src/manager.cpp src/sha1.cpp src/metainfo.cpp src/torrent.cpp include/service/exception.h src/udp_tracker.cpp include/service/tracker/tracker.h src/util.cpp src/events.cpp src/peer_connection.cpp src/messages.cpp)
add_library(lib::service ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/../digestpp/)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads Boost::thread)
target_compile_definitions(${PROJECT_NAME} PUBLIC BOOST_ASIO_HAS_IO_URING=1)
