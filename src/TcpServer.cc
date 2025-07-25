//
// Created by 13454 on 2025/7/22.
//

#include "TcpServer.h"
#include "Acceptor.h"
#include "Connection.h"
NETCPP::TcpServer::TcpServer(asio::io_context &io_context, asio::ip::tcp::endpoint endpoint)
    : base_loop_(io_context), acceptor_(new NETCPP::Acceptor(this, io_context,
                                                             endpoint)) {

  acceptor_->SetReadCallback([this](asio::ip::tcp::socket &sock, asio::io_context &io_context) {
    NewConnection(sock, io_context);
  });
}
void NETCPP::TcpServer::start() {

  thread_pool_.start();
  acceptor_->start();
}
void NETCPP::TcpServer::NewConnection(asio::ip::tcp::socket &sock, asio::io_context &io_context) {
  auto name = "conn-" + std::to_string(next_conn_id_++);
  auto conn = std::make_shared<Connection>(name, sock, io_context);
  conn->SetReadCallback(read_callback_);
  conn->start();
}
