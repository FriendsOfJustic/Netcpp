//
// Created by 13454 on 2025/7/22.
//

#include "Connection.h"
#include "iostream"
namespace NETCPP {

void Connection::onFinishRead(const asio::error_code &error, size_t bytes_transferred) {
  if (error) {
    switch (error.value()) {
      case asio::error::eof: {
        socket_.close();
        break;
      }
      default:std::cerr << "Error: " << error.message() << std::endl;
    }
    return;
  }
//  read_buffer_.consume(bytes_transferred);
  read_buffer_.commit(bytes_transferred);
  // TODO 处理数据
  if (read_callback_) {
    read_callback_(shared_from_this());
  }
  doRead();
  doWrite();

}
void Connection::doRead() {
  auto self = shared_from_this();
  socket_.async_read_some(read_buffer_.prepare(1024),
                          [this, self](const asio::error_code &error, size_t bytes_transferred) {
                            onFinishRead(error, bytes_transferred);
                          });
}
void Connection::doWrite() {
  if (write_buffer_.size() == 0) {
    if (is_shutdown_ && socket_.is_open()) {
      socket_.shutdown(asio::socket_base::shutdown_send);
    }
    return;
  }
  auto self = shared_from_this();
  asio::async_write(socket_, write_buffer_.data(),
                    [this, self](const asio::error_code &error, size_t bytes_transferred) {
                      onFinishWrite(error, bytes_transferred);  // 调用回调函数
                    });

}
void Connection::onFinishWrite(const asio::error_code &error, size_t bytes_transferred) {
  if (error) {
    std::cerr << "Error: " << error.message() << std::endl;
    socket_.shutdown(asio::socket_base::shutdown_send);
    return;
  }
  write_buffer_.consume(bytes_transferred);
  doWrite();
}
void Connection::Write(const std::string &message) {
  if (!socket_.is_open()) {
    return;
  }
  write_buffer_.sputn(message.data(), message.size());
  doWrite();
}
} // NETCPP