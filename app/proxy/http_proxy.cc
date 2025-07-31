//
// Created by 13454 on 2025/7/24.
//
#include "Connection.h"
#include "http/HttpServer.h"
#include "spdlog/spdlog.h"

using namespace NETCPP;

HttpResponse FAIL(std::error_code ec) {
  HttpResponse resp;
  resp.SetStatusCode(500);
  resp.SetHeader("Content-Type", "text/html");

  std::string body = fmt::format(
      "<!DOCTYPE html>\n"
      "<html lang=\"en\">\n"
      "<head>\n"
      "    <meta charset=\"UTF-8\">\n"
      "    <title>石海涛的代理服务器</title>\n"
      "</head>\n"
      "<body>\n"
      "<h1>石海涛的代理服务器</h1>\n"
      "<p>这是一个简单的代理服务器，用于将客户端的请求转发到目标服务器。</p>\n"
      "<p>当前版本：1.0.0</p>\n"
      "<p>%s</p>\n"
      "</body>\n"
      "</html>", ec.message());
  resp.SetBody(body);
  return resp;
}

void Copy(ConnectionPtr conn) {
  auto peer = std::any_cast<std::weak_ptr<Connection>>(conn->UserData()).lock();
  if (!peer || !peer->GetSocket().is_open()) {
    conn->ForceShutDown();
    return;
  }
  auto len = conn->ReadBuffer().size();
  auto begin = conn->ReadBuffer().getBuffer(len);
  peer->Write(begin, len);
  conn->ReadBuffer().discard(len);
}

void ClosePeer(ConnectionPtr conn) {
  auto peer = std::any_cast<std::weak_ptr<Connection>>(conn->UserData()).lock();
  if (peer) {
    peer->ForceShutDown();
  }
}
void AfterConnectHTTPS(ConnectionPtr self,
                       std::string host, short port,
                       std::shared_ptr<asio::ip::tcp::socket> socket) {
  auto proxy_conn =
      std::make_shared<Connection>("proxy-" + self->Name(), *socket, self->GetLoop());
  proxy_conn->doRead();
  if (proxy_conn->GetSocket().is_open()) {
    spdlog::info("connect to {}:{} success", host, port);
  }
  proxy_conn->SetReadCallback([proxy_conn](std::shared_ptr<Connection> conn) {
    Copy(conn);
  });
  self->SetReadCallback([](std::shared_ptr<Connection> conn) {
    Copy(conn);
  });
  proxy_conn->SetCloseCallback([](std::shared_ptr<Connection> conn) {
    ClosePeer(conn);
  });
  self->SetCloseCallback([](std::shared_ptr<Connection> conn) {
    ClosePeer(conn);
  });
  self->SetErrorCallback([](std::shared_ptr<Connection> conn, std::error_code ec) {
    ClosePeer(conn);
  });
  proxy_conn->SetErrorCallback([](std::shared_ptr<Connection> conn, std::error_code ec) {
    ClosePeer(conn);
  });
  self->SetUserData(proxy_conn->weak_from_this());
  proxy_conn->SetUserData(self->weak_from_this());

  self->Write("HTTP/1.1 200 Connection Established\r\n\r\n");
}
void AfterResolveHTTPS(std::error_code ec, const asio::ip::tcp::resolver::results_type &endpoints, ConnectionPtr self,
                       std::string host, short port) {
  if (ec || endpoints.empty()) {
    spdlog::error("resolve {}:{} failed: {}", host, port, ec.message());
    return;
  }
  auto socket = std::make_shared<asio::ip::tcp::socket>(self->GetLoop());
  socket->async_connect(*endpoints.begin(),
                        [socket, host, port, self](std::error_code ec)mutable {
                          if (ec) {
                            spdlog::error("connect {}:{} failed: {}", host, port, ec.message());
                            return;
                          }
                          AfterConnectHTTPS(self, host, port, socket);
                        });
}

void AfterConnectHTTP(ConnectionPtr self,
                      std::string host, std::string port,
                      std::shared_ptr<asio::ip::tcp::socket> socket, const HttpRequest &req) {

  auto proxy_conn =
      std::make_shared<Connection>("proxy-" + self->Name(), *socket, self->GetLoop());
  proxy_conn->doRead();

  proxy_conn->SetReadCallback([](std::shared_ptr<Connection> conn) {
    Copy(conn);
  });
  proxy_conn->SetCloseCallback([self](std::shared_ptr<Connection> conn) {
    ClosePeer(conn);
  });
  self->SetCloseCallback([proxy_conn](std::shared_ptr<Connection> conn) {
    ClosePeer(conn);
  });
  self->SetErrorCallback([](std::shared_ptr<Connection> conn, std::error_code ec) {
    ClosePeer(conn);
  });
  proxy_conn->SetErrorCallback([](std::shared_ptr<Connection> conn, std::error_code ec) {
    ClosePeer(conn);
  });
  self->SetUserData(proxy_conn->weak_from_this());
  proxy_conn->SetUserData(self->weak_from_this());
  proxy_conn->Write(req.ToString());

}

void AfterResolveHTTP(std::error_code ec, const asio::ip::tcp::resolver::results_type &endpoints, ConnectionPtr self,
                      std::string host, std::string port, HttpRequest req) {
  if (ec || endpoints.empty()) {
    spdlog::error("resolve {}:{} failed: {}", host, port, ec.message());
    self->Write(FAIL(ec).ToString());
    self->ShutDown();
    return;
  }
  auto socket = std::make_shared<asio::ip::tcp::socket>(self->GetLoop());
  socket->async_connect(*endpoints.begin(),
                        [self, req, socket, host, port](const asio::error_code &ec)mutable {
                          if (ec) {
                            spdlog::error("connect error {}", ec.message());
                            self->Write(FAIL(ec).ToString());
                            self->ShutDown();
                            return;
                          }
                          AfterConnectHTTP(self, host, port, socket, req);
                        });
}

bool ParseHttPProxyPath(const std::string &base_path,
                        std::string &path,
                        std::string &host,
                        std::string &port,
                        HttpRequest &request) {

  auto https_pos = base_path.find("https://");
  auto http_pos = base_path.find("http://");

  // host 的下标位置
  auto pos1 = http_pos;
  if (https_pos != std::string::npos) {
    port = "443";
    pos1 = https_pos + 8;
  } else if (http_pos != std::string::npos) {
    port = "80";
    pos1 = http_pos + 7;
  } else {
    return false;
  }
  // path 的起始位置
  auto pos2 = base_path.find_first_of("/", pos1);
  if (pos2 == std::string::npos) {
    path = "/";
  } else {
    path = base_path.substr(pos2);
  }


  // 在pos1-pos2 之间寻找 :
  auto pos3 = base_path.find(":", pos1, pos2 - pos1);
  if (pos3 == std::string::npos) {
    host = base_path.substr(pos1, pos2 - pos1);
  } else {
    host = base_path.substr(pos1, pos3 - pos1);
    port = base_path.substr(pos3 + 1, pos2 - pos3 - 1);
  }
  request.SetPath(path);
  return true;
}

void FirstConnection(HttpContext &ctx) {

  HttpRequest req = ctx.GetRequest();
  auto &resp = ctx.GetResponse();
  auto self = ctx.GetConnection();
  if (!self) {
    return;
  }
  if (req.GetMethod() == "CONNECT") {
    std::string full_host = req.GetPath(); // e.g., "www.baidu.com:443"
    auto pos = full_host.find(':');
    std::string host = full_host;
    short port = 80;

    if (pos != std::string::npos) {
      host = full_host.substr(0, pos);
      port = static_cast<short>(std::stoi(full_host.substr(pos + 1)));
    }
    auto resolver = std::make_shared<asio::ip::tcp::resolver>(self->GetLoop());

    resolver->async_resolve(host,
                            std::to_string(port),
                            [self, host, port, resolver](std::error_code ec,
                                                         asio::ip::tcp::resolver::results_type endpoints) {
                              AfterResolveHTTPS(ec, endpoints, self, host, port);
                            });
  } else {

    // 处理普通的HTTP请求
    auto &url = req.GetPath();

    std::string ip;
    std::string port;
    std::string path;

    if (!ParseHttPProxyPath(url, path, ip, port, req)) {
      spdlog::error("parse http proxy path failed");
      self->Write(FAIL(std::make_error_code(std::errc::invalid_argument)).ToString());
      self->ShutDown();
      return;
    }
    auto resolver = std::make_shared<asio::ip::tcp::resolver>(self->GetLoop());
    resolver->async_resolve(ip, port,
                            [self, ip, port, resolver, req](std::error_code ec,
                                                            const asio::ip::tcp::resolver::results_type &endpoints)mutable {
                              AfterResolveHTTP(ec, endpoints, self, ip, port, req);
                            });
  }
}

void HandleUserConnection(ConnectionPtr ptr) {
  assert(!ptr->UserData().has_value());
  if (!ptr->Context().has_value()) {
    ptr->SetContext(std::make_any<HttpContext>(ptr));
  }
  auto &context = std::any_cast<HttpContext &>(ptr->Context());
  if (!context.GetRequest().Parse(ptr->ReadBuffer())) {
    return;
  }
  FirstConnection(context);
}
int main(int argc, char *argv[]) {

  short port = atoi(argv[1]);
  asio::io_context io_context;
  TcpServer server(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
  server.SetThreadNum(4);
  server.SetReadCallback([](ConnectionPtr ptr) {
    HandleUserConnection(ptr);
  });
  server.start();
  auto work = asio::make_work_guard(io_context);
  io_context.run();
}