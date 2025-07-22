//
// Created by 13454 on 2025/7/22.
//

#include "iostream"
#include "TcpServer.h"
int main() {

  asio::io_context io_context;
  NETCPP::TcpServer server(io_context, asio::ip::tcp::endpoint(asio::ip::make_address_v4("0.0.0.0"), 18888));
  server.SetReadCallback([](NETCPP::ConnectionPtr ptr) {
    auto buffer = ptr->ReadBuffer().data();
    std::string_view str_view(static_cast<const char *>(buffer.data()), buffer.size());
    std::cout << str_view << std::endl;

    std::string output = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, world!";
    ptr->Write(output);
    ptr->ShutDown();
    
  });
  server.SetThreadNum(4);
  server.start();
  io_context.run();
}