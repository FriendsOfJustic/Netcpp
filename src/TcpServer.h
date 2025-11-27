//
// Created by 13454 on 2025/7/22.
//

#ifndef NETCPP_SRC_TCPSERVER_H_
#define NETCPP_SRC_TCPSERVER_H_
#include "EventLoopThread.h"
#include "asio.hpp"
#include "Connection.h"

namespace NETCPP {
  class Acceptor;

  class TcpServer {
  public:
    TcpServer(asio::io_context &io_context, asio::ip::tcp::endpoint endpoint);

    void start();

    void SetThreadNum(int thread_num) { thread_pool_.SetThreadNum(thread_num); }


    void SetConnectionCallback(const std::function<void(ConnectionPtr ptr)> &connection_callback) {
      connection_callback_ = connection_callback;
    }

    void SetReadCallback(const std::function<void(ConnectionPtr ptr)> &read_callback) {
      read_callback_ = read_callback;
    }

    void SetWriteCompleteCallback(const std::function<void(ConnectionPtr ptr)> &write_complete_callback) {
      write_complete_callback_ = write_complete_callback;
    }


    asio::io_context &GetNextLoop() {
      if (thread_pool_.threadNum() == 0) {
        return base_loop_;
      } else {
        return thread_pool_.GetNextLoop();
      }
    }

  private:
    void NewConnection(asio::ip::tcp::socket &sock, asio::io_context &io_context);

    std::function<void(ConnectionPtr ptr)> connection_callback_;
    std::function<void(ConnectionPtr ptr)> read_callback_;
    std::function<void(ConnectionPtr ptr)> write_complete_callback_;
    asio::io_context &base_loop_;
    EventLoopThreadPool thread_pool_;
    NETCPP::Acceptor *acceptor_;
    int next_conn_id_;
  };
}

#endif //NETCPP_SRC_TCPSERVER_H_
