cmake_minimum_required(VERSION 3.10)
project(Netcpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(third_party/asio-1.34.2/include)
include_directories(third_party/json/include)
include_directories(third_party/spdlog/include)
aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/http SRC_LIST)
# 检查是否为MSVC编译器
if (MSVC)
    # 设置源文件编码为UTF-8
    # /utf-8 选项同时设置源文件编码和执行环境编码为UTF-8
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

    # 可选：禁用特定警告
    add_compile_options(/wd4819)  # 禁用"该文件包含不能在当前代码页(936)中表示的字符"警告
endif ()


add_subdirectory(third_party/spdlog)

include_directories(${CMAKE_SOURCE_DIR}/src)


message(STATUS "src_list: ${SRC_LIST}")
add_executable(test1 tests/test1.cc ${SRC_LIST}
        src/TcpClient.h
        src/TcpClient.cc
        app/proxy/http_proxy.cc
        src/Buffer.cc
        src/Buffer.h)
add_executable(test2 tests/test_http_server.cc ${SRC_LIST})


add_subdirectory(app)
