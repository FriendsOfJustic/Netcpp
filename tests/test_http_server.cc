//
// Created by 13454 on 2025/7/24.
//
//
// Created by 13454 on 2025/7/22.
//

#include "iostream"
#include "TcpServer.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "nlohmann/json.hpp"
#include "http/HttpServer.h"

#include "nlohmann/json.hpp"
int main(int argc, char *argv[]) {
  int port = std::stoi(argv[1]);
  asio::io_context io_context;
  NETCPP::HttpServer server(io_context, asio::ip::tcp::endpoint(asio::ip::make_address_v4("0.0.0.0"), port));
  server.POST("/student", [](const NETCPP::HttpRequest &req, NETCPP::HttpResponse &resp) {
    resp.SetStatusCode(200);
    resp.SetStatusMessage("OK");
    resp.SetHeader("Content-Type", "application/json;charset=utf-8");
    nlohmann::json students = nlohmann::json::array();

    // 添加第一个学生信息
    students.push_back({
                           {"studentId", "2023001"},
                           {"name", "张三"},
                           {"gender", "男"},
                           {"age", 18},
                           {"className", "高一(1)班"},
                           {"phone", "13800138001"},
                           {"nativePlace", "北京市海淀区"}
                       });

    // 添加第二个学生信息
    students.push_back({
                           {"studentId", "2023002"},
                           {"name", "李四"},
                           {"gender", "女"},
                           {"age", 17},
                           {"className", "高一(2)班"},
                           {"phone", "13900139002"},
                           {"nativePlace", "上海市浦东新区"}
                       });

    // 添加第三个学生信息
    students.push_back({
                           {"studentId", "2023003"},
                           {"name", "王五"},
                           {"gender", "男"},
                           {"age", 18},
                           {"className", "高一(1)班"},
                           {"phone", "13700137003"},
                           {"nativePlace", "广东省广州市天河区"}
                       });

    // 添加第四个学生信息
    students.push_back({
                           {"studentId", "2023004"},
                           {"name", "赵六"},
                           {"gender", "女"},
                           {"age", 17},
                           {"className", "高一(3)班"},
                           {"phone", "13600136004"},
                           {"nativePlace", "四川省成都市锦江区"}
                       });

    // 添加第五个学生信息
    students.push_back({
                           {"studentId", "2023005"},
                           {"name", "孙七"},
                           {"gender", "男"},
                           {"age", 19},
                           {"className", "高一(2)班"},
                           {"phone", "13500135005"},
                           {"nativePlace", "江苏省南京市玄武区"}
                       });
    resp.JSON(nlohmann::json({
                                 {"code", 200},
                                 {"msg", "success"},
                                 {"data", students}
                             }));
  });

  server.POST("/teacher", [](const NETCPP::HttpRequest &req, NETCPP::HttpResponse &resp) {
    resp.SetStatusCode(200);
    resp.SetStatusMessage("OK");
    resp.SetHeader("Content-Type", "application/json;charset=utf-8");
    nlohmann::json teacher = nlohmann::json::array();
    teacher.push_back({
                          {"teacherId", "2023001"},
                          {"name", "张三"},
                          {"gender", "男"},
                          {"age", 18},
                          {"className", "高一(1)班"},
                          {"phone", "13800138001"},
                          {"nativePlace", "北京市海淀区"}
                      });
    resp.JSON(nlohmann::json({
                                 {"code", 200},
                                 {"msg", "success"},
                                 {"data", teacher}
                             }));
  });
  server.SetThreadNum(4);
  server.start();
  io_context.run();
}