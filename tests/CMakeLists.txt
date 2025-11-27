cmake_minimum_required(VERSION 3.20)


project(test)


file(GLOB TEST_SOURCE ${CMAKE_CURRENT_LIST_DIR}/*.cc ${CMAKE_CURRENT_LIST_DIR}/*.cpp)


set(PROTO_DIR ${CMAKE_CURRENT_LIST_DIR}/proto)
file(GLOB TEST_PROTO_SOURCE ${PROTO_DIR}/*.proto)

set(PROTO_GEN_SRC "")
set(PROTO_GEN_INC "")
foreach (PROTO_FILE ${TEST_PROTO_SOURCE})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    list(APPEND PROTO_GEN_SRC ${CMAKE_BINARY_DIR}/pb/${PROTO_NAME}.pb.cc)
    list(APPEND PROTO_GEN_INC ${CMAKE_BINARY_DIR}/pb/${PROTO_NAME}.pb.h)
    add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/pb/${PROTO_NAME}.pb.cc ${CMAKE_BINARY_DIR}/pb/${PROTO_NAME}.pb.h
            COMMAND protobuf::protoc
            ARGS --cpp_out=${CMAKE_BINARY_DIR}/pb --proto_path=${PROTO_DIR} ${PROTO_FILE}
            DEPENDS ${PROTO_FILE}
            VERBATIM
    )
endforeach ()

include_directories(${CMAKE_BINARY_DIR}/pb)
add_custom_target(generate_protobuf
        DEPENDS ${PROTO_GEN_SRC} ${PROTO_GEN_INC}
)
add_library(PROTOBUF_LIB STATIC ${PROTO_GEN_SRC})
add_dependencies(PROTOBUF_LIB generate_protobuf)

add_executable(test_proto_server test_proto_server.cc)
target_link_libraries(test_proto_server netcpp_static PROTOBUF_LIB)


add_executable(test_proto_client test_proto_client.cc)
target_link_libraries(test_proto_client netcpp_static PROTOBUF_LIB)


message(STATUS "PROTO_GEN_SRC: ${PROTO_GEN_SRC}")
message(STATUS "PROTO_GEN_INC: ${PROTO_GEN_INC}")


#foreach (TEST_FILE ${TEST_SOURCE})
#    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
#    add_executable(${TEST_NAME} ${TEST_FILE} ${TEST_PROTO_SOURCE})
#endforeach ()

