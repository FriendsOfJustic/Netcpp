//
// Created by 13454 on 2025/7/24.
//
#include <iostream>
#include "TcpClient.h"
#include "spdlog/spdlog.h"
using namespace NETCPP;

bool TcpClient::Connect(const std::string &ip, uint16_t port) {
  if (connection_) {
    throw std::runtime_error("Connection already in use, please close it first");
  }
  asio::ip::tcp::socket socket(ctx_);
  socket.connect(asio::ip::tcp::endpoint(asio::ip::make_address_v4(ip), port));
  connection_ = std::make_shared<Connection>("TcpClient", socket, ctx_);
  connection_->SetReadCallback(readCallback_);
  connection_->start();
  return true;
}

void TcpClient::AsyncConnect(const std::string &ip,
                             uint16_t port,
                             const std::function<void(TcpClientPtr)> &func) {
  asio::ip::tcp::socket socket(ctx_);
  auto connection = std::make_shared<Connection>("TcpClient", socket, ctx_);
  auto self = shared_from_this();
  connection->GetSocket().async_connect(asio::ip::tcp::endpoint(asio::ip::make_address_v4(ip), port),
                                        [self, func, connection, this](const asio::error_code &error) {
                                          if (error) {
                                            spdlog::error("AsyncConnect error: {}", error.message());
                                          } else {
                                            spdlog::debug("AsyncConnect success");
                                            func(self);
                                            connection_ = connection;
                                            connection_->SetReadCallback(readCallback_);
                                            connection_->start();
                                          }
                                        });
}

void TcpClient::Send(const std::string &data) {
  if (!connection_) {
    throw std::runtime_error("Connection not established, please connect first");
  }
  connection_->Write(data);
}


void TcpClient::Close() {
  connection_->ForceShutDown();
}

void TcpClient::Disconnect() {
  if (connection_) {
    connection_->ShutDown();
  }
}

