cmake_minimum_required(VERSION 3.10)
project(Netcpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(third_party/asio-1.34.2/include)
include_directories(third_party/json/include)
aux_source_directory(src SRC_LIST)


include_directories(${CMAKE_SOURCE_DIR}/src)
add_executable(${PROJECT_NAME} ${SRC_LIST}
        src/CallbackInterface.h
        src/Connection.cc
        src/Connection.h
        src/TcpServer.cc
        src/TcpServer.h
        src/Acceptor.h
        src/Acceptor.cc)

add_executable(test1 tests/test1.cc ${SRC_LIST}
        src/http/HttpRequest.cc
        src/http/HttpRequest.h
        src/http/HttpResponse.cc
        src/http/HttpResponse.h
        src/http/HttpServer.cc
        src/http/HttpServer.h)
