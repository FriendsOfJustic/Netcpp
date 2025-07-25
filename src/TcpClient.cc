//
// Created by 13454 on 2025/7/24.
//
#include <iostream>
#include "TcpClient.h"
#include "spdlog/spdlog.h"
using namespace NETCPP;
bool TcpClient::Connect(const std::string &ip, uint16_t port) {
  try {
    socket_ = std::make_unique<asio::ip::tcp::socket>(ctx_);
    socket_->connect(asio::ip::tcp::endpoint(asio::ip::make_address_v4(ip), port));
  } catch (const std::exception &e) {
    return false;
  }
  return true;
}
void TcpClient::AsyncConnect(const std::string &ip,
                             uint16_t port,
                             const std::function<void(TcpClientPtr)> &func) {
  socket_ = std::make_unique<asio::ip::tcp::socket>(ctx_);

  auto self = shared_from_this();
  socket_->async_connect(asio::ip::tcp::endpoint(asio::ip::make_address_v4(ip), port),
                         [this, self, func](const asio::error_code &error) {
                           if (error) {
                             spdlog::error("AsyncConnect error: {}", error.message());
                           } else {
                             spdlog::debug("AsyncConnect success");
                             func(shared_from_this());
                           }
                         });
}
void TcpClient::Send(const std::string &data) {
  asio::write(*socket_, asio::buffer(data));
}
void TcpClient::AsyncSend(const std::string &data, const std::function<void(TcpClientPtr)> &func) {
  auto self = shared_from_this();
  asio::async_write(*socket_,
                    asio::buffer(data),
                    [this, self, func](const asio::error_code &error, size_t bytes_transferred) {
                      if (error) {
                        spdlog::error("AsyncSend error: {}", error.message());
                      } else {
                        spdlog::debug("AsyncSend success, bytes_transferred: {}", bytes_transferred);
                        func(shared_from_this());
                      }
                    });
}
void TcpClient::Close() {
  socket_->close();
}
void TcpClient::Disconnect() {
  std::error_code ec;
  socket_->shutdown(asio::socket_base::shutdown_type::shutdown_send, ec);
  if (ec) {
    spdlog::error("Disconnect error: {}", ec.message());
    socket_->close();

  }

}

