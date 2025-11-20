//
// Created by 13454 on 2025/7/22.
//

#include "TcpServer.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/HttpServer.h"
#include "iostream"
#include "nlohmann/json.hpp"
int main(int argc, char* argv[]) {
  int port = std::stoi(argv[1]);
  asio::io_context io_context;

  spdlog::set_level(spdlog::level::debug);
  NETCPP::TcpServer server(
      io_context,
      asio::ip::tcp::endpoint(asio::ip::make_address_v4("0.0.0.0"), port));

  server.SetReadCallback([argv](NETCPP::ConnectionPtr ptr) {
    ptr->SendFile(argv[2]);
    ptr->ShutDown();
  });
  server.SetThreadNum(4);
  server.start();
  io_context.run();
}