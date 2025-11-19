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
    std::shared_ptr<std::vector<char> > array = std::make_shared<std::vector<char> >(1024 * 32);
    socket_.async_read_some(asio::buffer(*array),
                            [this, self, array](const asio::error_code &error, size_t bytes_transferred) {
                              read_buffer_.write(array->data(), bytes_transferred);
                              onFinishRead(error, bytes_transferred);
                            });
  }

  void Connection::doWrite() {
    if (!socket_.is_open()) {
      return;
    }
    if (write_buffer_.size() == 0) {
      if (write_complete_callback_) write_complete_callback_(shared_from_this());
      return;
    }
    asio::post(io_context_, [this]() {
      asio::error_code ec;
      size_t sz = std::min(static_cast<size_t>(65535), write_buffer_.size());
      const auto len = socket_.write_some(asio::buffer(write_buffer_.getReadPos(), sz), ec);
      if (ec) {
        spdlog::info("error while writing: {}", ec.message());
        return;
      }
      write_buffer_.discard(len);
      doWrite();
    });
  }

  void Connection::Write(const std::string &message) {
    Write(message.data(), message.size());
  }

  void Connection::Write(const char *data, size_t len) {
    if (!socket_.is_open()) {
      return;
    }
    asio::error_code ec;
    if (write_buffer_.size() == 0) {
      auto sz = socket_.write_some(asio::buffer(data, len), ec);
      if (ec) {
        spdlog::info("error while writing: {}", ec.message());
      } else {
        data += sz;
        len -= sz;
      }
    }
    if (len > 0) {
      write_buffer_.write(data, len);
      doWrite();
    }
  }
} // NETCPP
