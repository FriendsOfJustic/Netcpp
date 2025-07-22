cmake_minimum_required(VERSION 3.10)
project(Netcpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
find_package(asio CONFIG REQUIRED)
link_libraries(asio::asio)
aux_source_directory(src SRC_LIST)
#message(1111111111111111 ${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
add_executable(${PROJECT_NAME} ${SRC_LIST}
        src/CallbackInterface.h
        src/Connection.cc
        src/Connection.h
        src/TcpServer.cc
        src/TcpServer.h
        src/Acceptor.h
        src/Acceptor.cc)

add_executable(test1 tests/test1.cc ${SRC_LIST})
