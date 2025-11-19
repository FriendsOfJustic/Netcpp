cmake_minimum_required(VERSION 3.20)
# 启用 CMP0135 NEW 行为（优先推荐）
if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif ()
project(Netcpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
SET(THIRD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party)
include_directories(third_party/asio-1.34.2/include)
include_directories(third_party/json/include)
include_directories(third_party/spdlog/include)
aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/http SRC_LIST)
if (WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    set(WINDOWS_LIBS ws2_32.lib mswsock.lib advapi32.lib)
endif ()
# 检查是否为MSVC编译器
if (MSVC)
    # 设置源文件编码为UTF-8
    # /utf-8 选项同时设置源文件编码和执行环境编码为UTF-8
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

    # 可选：禁用特定警告
    add_compile_options(/wd4819)  # 禁用"该文件包含不能在当前代码页(936)中表示的字符"警告
endif ()

find_package(asio CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
link_libraries(nlohmann_json::nlohmann_json spdlog::spdlog_header_only asio::asio)
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
