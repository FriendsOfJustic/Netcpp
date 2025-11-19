//
// Created by 13454 on 2025/7/22.
//

#include "Connection.h"
#include <filesystem>
#include "spdlog/spdlog.h"

namespace NETCPP {
  void Connection::doWriteFile() {
    if (file_info_->offset == file_info_->total_size) {
      fclose(file_info_->fd);
      file_info_ = nullptr;
      if (write_complete_callback_) {
        write_complete_callback_(shared_from_this());
      }
      return;
    }
    asio::post(io_context_, [this]() {
      char buffer[65535];
      auto send_sz = std::min(static_cast<size_t>(65535), file_info_->total_size - file_info_->offset);
      fread(buffer, 1, send_sz, file_info_->fd);
      auto actual_sz = socket_.write_some(asio::buffer(buffer, send_sz));
      file_info_->offset += actual_sz;
    });
  }

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
                                    spdlog::error("connection: {} read error: {} {}:{}", name_, error.message(),
                                                  __FILE__, __LINE__);
                                  }
                                }
                                return;
                              }
                              read_buffer_.write(array->data(), bytes_transferred);
                              if (read_callback_) {
                                read_callback_(shared_from_this());
                              }
                              doRead();
                            });
  }

  void Connection::doWrite() {
    if (!socket_.is_open()) {
      return;
    }
    if (write_buffer_.size() == 0) {
      if (write_complete_callback_)
        write_complete_callback_(shared_from_this());
      if (is_shutdown_) {
        asio::error_code ec;
        spdlog::info("server shutting down");
        socket_.shutdown(asio::socket_base::shutdown_send, ec);
      }
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

  void Connection::ShutDown() {
    if (write_buffer_.size() == 0) {
      asio::error_code ec;
      socket_.shutdown(asio::socket_base::shutdown_send, ec);
    } else {
      is_shutdown_ = true;
    }
  }

  void Connection::Write(const std::string &message) {
    Write(message.data(), message.size());
  }

  void Connection::Write(const char *data, size_t len) {
    if (!socket_.is_open()) {
      return;
    }
    if (file_info_) {
      throw std::logic_error("can not write msg while file is sending");
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


  void Connection::SendFile(const std::string &p) {
    using namespace std::filesystem;
    path file_path = p;
    if (!exists(p)) {
      throw std::invalid_argument("path does not exist");
    }
    // 获取文件大小
    uint64_t sz = file_size(file_path);
    uint64_t send_sz = std::min(static_cast<uint64_t>(65535), sz);
    auto file = fopen(p.c_str(), "rb");
    char buffer[65535];
    fread(buffer, 1, send_sz, file);
    auto actual_sz = socket_.write_some(asio::buffer(buffer, send_sz));
    file_info_ = std::make_unique<FileInfo>();
    file_info_->fd = file;
    file_info_->offset = actual_sz;
    file_info_->total_size = sz;
    doWriteFile();
  }
} // NETCPP
