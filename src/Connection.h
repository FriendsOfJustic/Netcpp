//
// Created by 13454 on 2025/7/22.
//

#ifndef NETCPP_SRC_CONNECTION_H_
#define NETCPP_SRC_CONNECTION_H_

#include <string>
#include <utility>

#include "Buffer.h"
#include "asio.hpp"
#include "memory"
#include "spdlog/spdlog.h"

namespace NETCPP {
class Connection;
typedef std::shared_ptr<Connection> ConnectionPtr;

class Connection : public std::enable_shared_from_this<Connection> {
 public:
  struct FileInfo {
    FILE* fd;
    int raw_fd;
    uint64_t offset;
    uint64_t total_size;
    enum class SendType {
      sendfile = 0,
      direct = 1,
    };
    SendType send_type;

#ifdef __linux__
    int64_t blk_size;
#endif
  };

  Connection(std::string name, asio::ip::tcp::socket& socket,
             asio::io_context& io_context);

  void SetReadCallback(
      const std::function<void(ConnectionPtr ptr)>& read_callback) {
    read_callback_ = read_callback;
  }

  void SetWriteCompleteCallback(
      const std::function<void(ConnectionPtr ptr)>& write_complete_callback) {
    write_complete_callback_ = write_complete_callback;
  }

  void SetErrorCallback(
      const std::function<void(ConnectionPtr ptr, std::error_code)>&
          error_callback) {
    error_callback_ = error_callback;
  }

  void SetCloseCallback(
      const std::function<void(ConnectionPtr ptr)>& close_callback) {
    close_callback_ = close_callback;
  }

  void doRead();

  void doWrite();

  void start() { doRead(); }

  std::string& Name() { return name_; }
  Buffer& ReadBuffer() { return read_buffer_; }
  Buffer& WriteBuffer() { return write_buffer_; }

  void ShutDown();

  void ForceShutDown() { socket_.close(); }

  void Write(const std::string& message);

  void Write(const char* data, size_t len);

  void SendFile(const std::string& path);

  std::any& Context() { return context_; }
  void SetContext(std::any context) { context_ = std::move(context); }
  asio::io_context& GetLoop() { return io_context_; }
  std::any& UserData() { return user_data_; }
  void SetUserData(std::any user_data) { user_data_ = std::move(user_data); }

  asio::ip::tcp::socket& GetSocket() { return socket_; }

  ~Connection() {
    if (socket_.is_open()) {
      socket_.close();
    }
    spdlog::info("connection: {} destroyed", name_);
  }

 private:
  std::function<void(ConnectionPtr ptr)> close_callback_;
  std::any user_data_;
  std::any context_;
  bool is_shutdown_ = false;

  void doWriteFile();

  void onFinishRead(const asio::error_code& error, size_t bytes_transferred);

  std::string name_;
  std::unique_ptr<FileInfo> file_info_ = nullptr;
  asio::io_context& io_context_;
  std::function<void(ConnectionPtr ptr)> read_callback_;
  std::function<void(ConnectionPtr ptr)> write_complete_callback_;
  std::function<void(ConnectionPtr ptr, std::error_code)> error_callback_;
  Buffer read_buffer_;
  Buffer write_buffer_;
  asio::ip::tcp::socket socket_;
};
}  // namespace NETCPP

#endif  // NETCPP_SRC_CONNECTION_H_
