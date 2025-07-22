//
// Created by 13454 on 2025/7/22.
//

#ifndef NETCPP_SRC_CONNECTION_H_
#define NETCPP_SRC_CONNECTION_H_
#include <string>
#include <utility>
#include "memory"
#include "asio.hpp"
namespace NETCPP {

class Connection;
typedef std::shared_ptr<Connection> ConnectionPtr;
class Connection : public std::enable_shared_from_this<Connection> {

 public:

  Connection(std::string name, asio::ip::tcp::socket &socket)
      : socket_(std::move(socket)), name_(std::move(name)) {}

  void SetReadCallback(const std::function<void(ConnectionPtr ptr)> &read_callback) {
    read_callback_ = read_callback;
  }
  void doRead();
  void doWrite();
  void start() { doRead(); }

  asio::streambuf &ReadBuffer() { return read_buffer_; }
  asio::streambuf &WriteBuffer() { return write_buffer_; }
  void ShutDown() { is_shutdown_ = true; }
  void ForceShutDown() { socket_.close(); }

  void Write(const std::string &message);

 private:

  bool is_shutdown_ = false;
  void onFinishRead(const asio::error_code &error, size_t bytes_transferred);
  void onFinishWrite(const asio::error_code &error, size_t bytes_transferred);
  std::string name_;
  std::function<void(ConnectionPtr ptr)> read_callback_;
  asio::streambuf read_buffer_;
  asio::streambuf write_buffer_;
  asio::ip::tcp::socket socket_;
};

} // NETCPP

#endif //NETCPP_SRC_CONNECTION_H_
