//
// Created by 13454 on 2025/7/22.
//

#ifndef NETCPP_SRC_HTTP_HTTPSERVER_H_
#define NETCPP_SRC_HTTP_HTTPSERVER_H_
#include "asio.hpp"
#include "TcpServer.h"
namespace NETCPP {
class HttpServer {
 public:

 private:
  TcpServer tcp_server_;
};
}
#endif //NETCPP_SRC_HTTP_HTTPSERVER_H_
