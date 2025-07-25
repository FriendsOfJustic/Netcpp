//
// Created by 13454 on 2025/7/22.
//

#include "Connection.h"
#include "iostream"
#include "spdlog/spdlog.h"
namespace NETCPP {

void Connection::onFinishRead(const asio::error_code &error, size_t bytes_transferred) {
  if (error) {
    switch (error.value()) {
      case asio::error::eof: {
        if (close_callback_) {
          close_callback_(shared_from_this());
        }
        socket_.close();
        break;
      }
      default: {
        if (error_callback_) {
          error_callback_(shared_from_this(), error);
        }
        spdlog::error("connection: {} read error: {} {}:{}", name_, error.message(), __FILE__, __LINE__);
      }
    }
    return;
  }

  // TODO 处理数据
  if (read_callback_) {
    read_callback_(shared_from_this());
  }
  doRead();
}
void Connection::doRead() {
  auto self = shared_from_this();
  std::shared_ptr<std::vector<char>> array = std::make_shared<std::vector<char>>(1024 * 32);
  socket_.async_read_some(asio::buffer(*array),
                          [this, self, array](const asio::error_code &error, size_t bytes_transferred) {
                            read_buffer_.write(array->data(), bytes_transferred);
                            onFinishRead(error, bytes_transferred);
                          });
}
void Connection::doWrite() {

  if (write_buffer_.size() == 0) {
    if (write_complete_callback_) {
      write_complete_callback_(shared_from_this());
    }
    if (is_shutdown_ && socket_.is_open()) {
      std::error_code ec;
      socket_.shutdown(asio::socket_base::shutdown_send, ec);
      if (ec) {
        socket_.close();
        return;
      }
    }
    return;
  }
  auto self = shared_from_this();
  size_t len = 65535;
  auto begin = write_buffer_.getBuffer(len);
  auto send_buf = std::make_shared<std::vector<char>>(begin, begin + len);
  write_buffer_.discard(len);
  socket_.async_write_some(asio::buffer(*send_buf),
                           [this, self, send_buf](const asio::error_code &error,
                                                  size_t bytes_transferred) {
                             onFinishWrite(error, bytes_transferred);  // 调用回调函数
                           });
}
void Connection::onFinishWrite(const asio::error_code &error, size_t bytes_transferred) {
  if (error) {
    spdlog::error("connection: {} write error: {} {}:{}", name_, error.message(), __FILE__, __LINE__);
    std::error_code ec;
    socket_.shutdown(asio::socket_base::shutdown_send, ec);
    if (ec) {
      spdlog::error("connection: {} shutdown error: {} {}:{}", name_, ec.message(), __FILE__, __LINE__);
    }
    return;
  }
  doWrite();
}
void Connection::Write(const std::string &message) {
  if (!socket_.is_open()) {
    return;
  }
  write_buffer_.write(message.data(), message.size());
  doWrite();
}
void Connection::Write(const char *data, size_t len) {
  if (!socket_.is_open()) {
    return;
  }
  write_buffer_.write(data, len);
  doWrite();
}
} // NETCPP