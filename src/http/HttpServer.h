//
// Created by 13454 on 2025/7/22.
//

#ifndef NETCPP_SRC_HTTP_HTTPSERVER_H_
#define NETCPP_SRC_HTTP_HTTPSERVER_H_
#include <utility>

#include "asio.hpp"
#include "TcpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Type.h"
namespace NETCPP {

class HttpContext {
 public:
  HttpContext(ConnectionPtr ptr) : response_(std::move(ptr)) {}
  ~HttpContext() = default;
  void SetRequest(const HttpRequest &req) { request_ = req; }
  void SetResponse(const HttpResponse &resp) { response_ = resp; }
  HttpRequest &GetRequest() { return request_; }
  HttpResponse &GetResponse() { return response_; }
 private:
  HttpRequest request_;
  HttpResponse response_;
};

class Router {
 public:
  void addRoute(const HTTP_METHOD &method, const std::string &path, const HttpHandler &handler);
  HttpHandler getHandler(const std::string &method, const std::string &path) const;
 private:
  std::unordered_map<HTTP_METHOD, std::unordered_map<std::string, HttpHandler >> routes_;
};
class HttpServer {
 public:

  HttpServer(asio::io_context &io_context, asio::ip::tcp::endpoint endpoint)
      : tcp_server_(io_context, std::move(endpoint)) {
  }
  void SetReadCallback(const HttpHandler &read_callback) {
    handler_ = read_callback;
  }
  void SetThreadNum(int num) {
    tcp_server_.SetThreadNum(num);
  }

  void GET(const std::string &path, const HttpHandler &handler);
  void POST(const std::string &path, const HttpHandler &handler);
  void PUT(const std::string &path, const HttpHandler &handler);

  void AddRoute(const HTTP_METHOD &method, const std::string &path, const HttpHandler &handler);

  void start() {
    tcp_server_.SetReadCallback([this](ConnectionPtr ptr) {
      TcpReadCallback(std::move(ptr));
    });
    tcp_server_.start();
  }

  void SetRouter(const Router &router) {
    router_ = router;
  }
 private:
  Router router_;
  void TcpReadCallback(ConnectionPtr ptr);

  void Dispatch(const HttpRequest &req, HttpResponse &resp);

  HttpHandler handler_;
  TcpServer tcp_server_;
};
}
#endif //NETCPP_SRC_HTTP_HTTPSERVER_H_
