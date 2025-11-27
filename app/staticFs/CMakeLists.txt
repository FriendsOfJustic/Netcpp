

cmake_minimum_required(VERSION 3.20)

project(staticFs)


add_executable(staticFs main.cc)
target_link_libraries(staticFs netcpp_static)
