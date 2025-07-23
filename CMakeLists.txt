cmake_minimum_required(VERSION 3.10)
project(Netcpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(third_party/asio-1.34.2/include)
include_directories(third_party/json/include)
aux_source_directory(src SRC_LIST)
aux_source_directory(src/http SRC_LIST)


include_directories(${CMAKE_SOURCE_DIR}/src)


message(STATUS "src_list: ${SRC_LIST}")
add_executable(test1 tests/test1.cc ${SRC_LIST})
