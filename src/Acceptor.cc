//
// Created by 13454 on 2025/7/22.
//
#include "Acceptor.h"

#include "TcpServer.h"
void NETCPP::Acceptor::doAccept() {
//  auto self = shared_from_this();
  acceptor_.async_accept(server_->GetNextLoop(),
                         [this](const asio::error_code &error, asio::ip::tcp::socket socket) {
                           if (!error) {
                             // 处理新连接
                             read_callback_(socket);
                             doAccept();
                           }
                         });
}
