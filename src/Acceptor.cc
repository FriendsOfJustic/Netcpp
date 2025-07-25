//
// Created by 13454 on 2025/7/22.
//
#include "Acceptor.h"

#include "TcpServer.h"
void NETCPP::Acceptor::doAccept() {
//  auto self = shared_from_this();
  auto &io_loop = server_->GetNextLoop();
  acceptor_.async_accept(io_loop,
                         [this, &io_loop](const asio::error_code &error, asio::ip::tcp::socket socket) {
                           if (!error) {
                             // 处理新连接
                             read_callback_(socket, io_loop);
                             doAccept();
                           }
                         });
}
