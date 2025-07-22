//
// Created by 13454 on 2025/7/22.
//

#ifndef NETCPP_SRC_ACCEPTOR_H_
#define NETCPP_SRC_ACCEPTOR_H_

#include "asio.hpp"
#include "TcpServer.h"
namespace NETCPP {

class TcpServer;
class Acceptor : public std::enable_shared_from_this<Acceptor> {
 public:
  Acceptor(TcpServer *server, asio::io_context &io_context, const asio::ip::tcp::endpoint &endpoint)
      : acceptor_(io_context, endpoint), next_conn_id_(0), server_(server) {
  }

  void SetReadCallback(const std::function<void(asio::ip::tcp::socket &socket)> &read_callback) {
    read_callback_ = read_callback;
  }
  void start() {
    doAccept();
  }
 private:
  void doAccept();
  TcpServer *server_;
  std::function<void(asio::ip::tcp::socket &socket)> read_callback_;
  asio::ip::tcp::acceptor acceptor_;
  int next_conn_id_;
};
}
#endif //NETCPP_SRC_ACCEPTOR_H_
