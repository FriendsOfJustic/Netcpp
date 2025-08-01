//
// Created by 13454 on 2025/7/22.
//

#include "iostream"
#include "TcpServer.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "nlohmann/json.hpp"
#include "http/HttpServer.h"
int main(int argc, char *argv[]) {
  int port = std::stoi(argv[1]);
  asio::io_context io_context;
  NETCPP::HttpServer server(io_context, asio::ip::tcp::endpoint(asio::ip::make_address_v4("0.0.0.0"), port));
//  server.SetReadCallback([](NETCPP::ConnectionPtr ptr) {
////    auto buffer = ptr->ReadBuffer().data();
////    std::string_view str_view(static_cast<const char *>(buffer.data()), buffer.size());
////    std::cout << str_view << std::endl;
////
////    std::string output = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, world!";
////    ptr->Write(output);
////    ptr->ShutDown();
//
//    NETCPP::HttpRequest request;
//    if (request.Parse(ptr->ReadBuffer())) {
//
////      std::cout << "Method: " << request.GetMethod() << std::endl;
////      std::cout << "Path: " << request.GetPath() << std::endl;
////      std::cout << "Version: " << request.GetVersion() << std::endl;
////      std::cout << "Body: " << request.GetBody() << std::endl;
//////      std::cout << "Content-Length: " << request.GetHeader("Content-Length") << std::endl;
////      std::cout << "Host: " << request.GetHeader("Host") << std::endl;
//
//      NETCPP::HttpResponse response;
//      response.SetStatusCode(200);
//      response.SetStatusMessage("OK");
//      response.SetHeader("Content-Type", "text/plain;charset=utf-8");
//
//      std::stringstream ss;
//      ss << std::this_thread::get_id();
//      response.SetBody("Hello, world! 老谢是一只猪" + ss.str());
//      response.Send(ptr);
//      ptr->ShutDown();
//    }
//
//  });
//  server.SetReadCallback([](const NETCPP::HttpRequest &req, NETCPP::HttpResponse &resp) {
//
//
//  });

  server.GET("/", [](const NETCPP::HttpRequest &req, NETCPP::HttpResponse &resp) {
    resp.SetStatusCode(200);
    resp.SetStatusMessage("OK");
    resp.SetHeader("Content-Type", "text/plain;charset=utf-8");
    resp.SetBody("Hello, world! \r\n 老谢是一只猪 我喜欢老谢💩💩💩💩💩💩💩💩\r\n love you~ ❤️❤️❤️❤️❤️❤️❤️❤️❤️❤️");
  });
  server.SetThreadNum(4);
  server.start();
  io_context.run();
}