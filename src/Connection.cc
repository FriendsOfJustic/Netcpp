//
// Created by 13454 on 2025/7/22.
//

#include "Connection.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#ifdef __linux__
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <filesystem>
#include <thread>

#include "spdlog/spdlog.h"

namespace NETCPP {
#ifdef SEND_FILE
  thread_local int *static_blk_size = nullptr;
  thread_local char *static_direct_buffer = nullptr;
  size_t DirectRead(int fd, char **buffer, size_t count, size_t blk_size) {
    if (static_blk_size == nullptr) {
      static_blk_size = new int(blk_size);
    } else if (*static_blk_size != blk_size) {
      *static_blk_size = blk_size;
      free(static_direct_buffer);
    }
    size_t buf_size = 512 * (*static_blk_size); // 16 块，保证长度对齐

    if (posix_memalign((void **) &static_direct_buffer, *static_blk_size,
                       buf_size) != 0) {
      throw std::runtime_error("posix_memalign error");
    }
    auto actual_sz = read(fd, static_direct_buffer, buf_size);
    if (actual_sz == -1) {
      throw std::runtime_error("read file error");
    }
    *buffer = static_direct_buffer;
    return actual_sz;
  }
  void Connection::doWriteFile() {
    if (file_info_->offset == file_info_->total_size) {
      spdlog::debug("connection: {} sendfile end", name_);
      close(file_info_->raw_fd);
      file_info_.release();
      if (write_complete_callback_) {
        write_complete_callback_(shared_from_this());
      }
      if (is_shutdown_) {
        asio::error_code ec;
        spdlog::info("server shutting down");
        socket_.shutdown(asio::socket_base::shutdown_send, ec);
      }
      return;
    }
    socket_.async_wait(
      asio::socket_base::wait_write, [this](const asio::error_code &error) {
        if (error) {
          spdlog::error("connection: {} write error: {} {}:{}", name_,
                        error.message(), __FILE__, __LINE__);
          return;
        }
        if (file_info_->send_type == FileInfo::SendType::sendfile) {
          auto send_sz = file_info_->total_size - file_info_->offset;

          if (!socket_.is_open()) spdlog::info("已经关闭");
          loff_t offset = file_info_->offset;
          auto actual_sz = sendfile64(socket_.native_handle(),
                                      file_info_->raw_fd, &offset, send_sz);

          spdlog::debug("sendfile64: {}", actual_sz);
          if (actual_sz == -1 && errno != EAGAIN) {
            spdlog::info("sendfile64 error {} {}", actual_sz, strerror(errno));
            throw std::runtime_error("sendfile64 error");
          }
          file_info_->offset += std::max(actual_sz, static_cast<ssize_t>(0));
        } else {
          char *buffer = nullptr;
          auto actual_sz = DirectRead(file_info_->raw_fd, &buffer,
                                      sizeof(buffer), file_info_->blk_size);

          if (actual_sz == -1) {
            spdlog::error("read file error{}", strerror(errno));
            throw std::runtime_error("read file error");
          }
          actual_sz = socket_.write_some(asio::buffer(buffer, actual_sz));
          spdlog::debug("DirectRead send: {}", actual_sz);
          if (actual_sz == -1) {
            throw std::runtime_error("write file error");
          }
          file_info_->offset += actual_sz;
        }
        doWriteFile();
      });
  }
#else


#ifdef ENABLE_PROTOBUF_EXTENSIONS
  void Connection::SendMsg(const BaseMessagePtr &msg) {
    if (!msg) {
      spdlog::error("connection: {} send msg error: msg is null", name_);
      return;
    }
    std::string b;
    codec_.serialize(msg, b);
    Write(b);
  }
#endif


  void Connection::doWriteFile() {
    if (file_info_->offset == file_info_->total_size) {
      fclose(file_info_->fd);
      file_info_ = nullptr;
      if (write_complete_callback_) {
        write_complete_callback_(shared_from_this());
      }
      if (is_shutdown_) {
        asio::error_code ec;
        spdlog::info("server shutting down");
        socket_.shutdown(asio::socket_base::shutdown_send, ec);
      }
      return;
    }
    socket_.async_wait(
      asio::socket_base::wait_write, [this](const asio::error_code &error) {
        if (error) {
          spdlog::error("connection: {} write error: {} {}:{}", name_,
                        error.message(), __FILE__, __LINE__);
          return;
        }
        char buffer[65535];
        auto send_sz = std::min(static_cast<size_t>(65535),
                                file_info_->total_size - file_info_->offset);
        auto ret = fread(buffer, 1, send_sz, file_info_->fd);
        if (ret < 0) {
          throw std::runtime_error("read file error");
        }
        auto actual_sz = socket_.write_some(asio::buffer(buffer, send_sz));
        // std::cout << "connection: {} write {} bytes" << name_ << actual_sz <<
        // std::endl;
        spdlog::info("connection: {} write {} bytes", name_, actual_sz);
        file_info_->offset += actual_sz;
        doWriteFile();
      });
  }
#endif

  void Connection::onFinishRead(const asio::error_code &error,
                                size_t bytes_transferred) {
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
          spdlog::error("connection: {} read error: {} {}:{}", name_,
                        error.message(), __FILE__, __LINE__);
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

  Connection::Connection(std::string name, asio::ip::tcp::socket &socket,
                         asio::io_context &io_context)
    : socket_(std::move(socket)),
      name_(std::move(name)),
      io_context_(io_context) {
    socket_.non_blocking(true);
  }

  void Connection::doRead() {
    auto self = shared_from_this();
    std::shared_ptr<std::vector<char> > array =
        std::make_shared<std::vector<char> >(1024 * 32);

    socket_.async_read_some(
      asio::buffer(*array), [this, self, array](const asio::error_code &error,
                                                size_t bytes_transferred) {
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
              spdlog::error("connection: {} read error: {} {}:{}", name_,
                            error.message(), __FILE__, __LINE__);
            }
          }
          return;
        }
        read_buffer_.write(array->data(), bytes_transferred);
        if (read_callback_) {
          try {
            read_callback_(shared_from_this());
          } catch (const std::exception &e) {
            spdlog::error("read callback error: {} ", e.what());
          }
        }

        doRead();
      });
  }

  void Connection::doWrite() {
    if (!socket_.is_open()) {
      return;
    }
    if (write_buffer_.size() == 0) {
      if (write_complete_callback_) write_complete_callback_(shared_from_this());
      if (is_shutdown_) {
        asio::error_code ec;
        spdlog::info("server shutting down");
        socket_.shutdown(asio::socket_base::shutdown_send, ec);
      }
      return;
    }
    socket_.async_wait(
      asio::socket_base::wait_write, [this](const asio::error_code &error) {
        asio::error_code ec;
        size_t sz = std::min(static_cast<size_t>(65535), write_buffer_.size());
        const auto len = socket_.write_some(
          asio::buffer(write_buffer_.getReadPos(), sz), ec);
        if (ec) {
          spdlog::info("error while writing: {}", ec.message());
          return;
        }
        write_buffer_.discard(len);
        doWrite();
      });
  }

  void Connection::ShutDown() {
    if (write_buffer_.size() == 0 && !file_info_) {
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

#ifdef SEND_FILE

  void Connection::SendFile(const std::string &p) {
    using namespace std::filesystem;
    spdlog::info("use send file");
    path file_path = p;
    if (!exists(p)) {
      throw std::invalid_argument("path does not exist");
    }
    // 获取文件大小
    uint64_t sz = file_size(file_path);
    spdlog::debug("file size: {}", sz);
    file_info_ = std::make_unique<FileInfo>();
    file_info_->total_size = sz;
    file_info_->offset = 0;
    if (sz < 1024 * 1024 * 100) {
      auto fd = open(p.c_str(), O_RDONLY);
      if (fd == -1) {
        throw std::runtime_error("open file error");
      }
      file_info_->raw_fd = fd;
      file_info_->send_type = FileInfo::SendType::sendfile;

      loff_t offset = file_info_->offset;
      auto actuall_sz =
          sendfile64(socket_.native_handle(), file_info_->raw_fd, &offset, sz);

      spdlog::debug("sendfile64: {}", actuall_sz);
      if (actuall_sz == -1) {
        spdlog::error("sendfile error{}", strerror(errno));
        throw std::runtime_error("sendfile64 error");
      }
      file_info_->offset += actuall_sz;
    } else {
      auto fd = open(p.c_str(), O_RDONLY | O_DIRECT | O_LARGEFILE);
      if (fd == -1) {
        throw std::runtime_error("open file error");
      }
      file_info_->raw_fd = fd;
      file_info_->send_type = FileInfo::SendType::direct;
      struct stat64 st;
      fstat64(file_info_->raw_fd, &st);
      spdlog::debug("blksize: {}", st.st_blksize);
      file_info_->blk_size = st.st_blksize;
      char *buffer = nullptr;
      auto actual_sz = DirectRead(file_info_->raw_fd, &buffer, sizeof(buffer),
                                  file_info_->blk_size);
      if (actual_sz == -1) {
        spdlog::error("read file error{}", strerror(errno));
        throw std::runtime_error("read file error");
      }
      actual_sz = socket_.write_some(asio::buffer(buffer, actual_sz));

      if (actual_sz == -1) {
        throw std::runtime_error("write file error");
      }
      file_info_->offset += actual_sz;
    }
    doWriteFile();
  }

#else
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
    auto ret = fread(buffer, 1, send_sz, file);
    if (ret < 0) {
      throw std::runtime_error("read file error");
    }
    auto actual_sz = socket_.write_some(asio::buffer(buffer, send_sz));
    file_info_ = std::make_unique<FileInfo>();
    file_info_->fd = file;
    file_info_->offset = actual_sz;
    file_info_->total_size = sz;
    doWriteFile();
  }
#endif
} // namespace NETCPP
