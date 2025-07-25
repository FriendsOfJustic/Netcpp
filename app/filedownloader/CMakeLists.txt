

project(filedownloader)

include_directories(${CMAKE_SOURCE_DIR}/src)
add_executable(shit client.cc ${SRC_LIST}
        uploader.cc
        uploader.h
        utils.h)