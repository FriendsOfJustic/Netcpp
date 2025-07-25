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
void FirstConnection(HttpContext &ctx) {
  HttpRequest req = ctx.GetRequest();
  auto &resp = ctx.GetResponse();
  auto self = ctx.GetConnection();
  if (!self) {
    return;
  }
  if (req.GetMethod() == "CONNECT") {
    spdlog::info("HTTPS connection");

    std::string full_host = req.GetPath(); // e.g., "www.baidu.com:443"
    auto pos = full_host.find(':');
    std::string host = full_host;
    short port = 80;

    if (pos != std::string::npos) {
      host = full_host.substr(0, pos);
      port = static_cast<short>(std::stoi(full_host.substr(pos + 1)));
    }
    asio::ip::tcp::resolver resolver(self->GetLoop());

    std::error_code ec;
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port), ec);
    if (ec) {
      spdlog::error("resolve {}:{} failed: {}", host, port, ec.message());
      return;
    }
    auto socket = std::make_shared<asio::ip::tcp::socket>(self->GetLoop());
    socket->async_connect(*endpoints.begin(),
                          [socket, host, port, self](std::error_code ec)mutable {
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
                          });

  } else {

    // 处理普通的HTTP请求
    auto &url = req.GetPath();

    std::string ip;
    short port = 80;
    std::string path;
    auto pos = url.find_first_of("http://");
    if (pos != std::string::npos) {
      auto pos2 = url.find_first_of(pos + 7, '/');
      if (pos2 == std::string::npos) {
        path = "/";
        pos2 = url.size();
      } else {
        path = url.substr(pos2);
      }
      auto host = url.substr(pos + 7, pos2 - pos - 8);
      pos2 = host.find_first_of(':');
      if (pos2 != std::string::npos) {
        ip = host.substr(0, pos2);
        port = atoi(host.substr(pos2 + 1).c_str());
      } else {
        ip = host;
      }

    } else {
      // TODO
    }
    asio::ip::tcp::resolver resolver(self->GetLoop());
    std::error_code ec;
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(ip, std::to_string(port), ec);
    if (ec) {
      spdlog::error("resolve {}:{} failed: {}", ip, port, ec.message());
      self->Write(FAIL(ec).ToString());
      self->ShutDown();
      return;
    }
    auto socket = std::make_shared<asio::ip::tcp::socket>(self->GetLoop());
    socket->async_connect(*endpoints.begin(),
                          [self, req, path, ip, socket](const asio::error_code &ec)mutable {
                            if (ec) {
                              spdlog::error("connect error {}", ec.message());
                              self->Write(FAIL(ec).ToString());
                              self->ShutDown();
                              return;
                            }
                            HttpRequest new_req;
                            new_req.SetMethod(req.GetMethod());
                            new_req.SetPath(path);
                            new_req.SetVersion(req.GetVersion());
                            new_req.SetHeader("Host", ip);
                            new_req.SetBody(std::move(req.GetBody()));
                            new_req.SetHeaders(req.GetHeaders());
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
                            proxy_conn->Write(new_req.ToString());

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

  server.SetReadCallback([](ConnectionPtr ptr) {
    HandleUserConnection(ptr);
  });
  server.start();
  auto work = asio::make_work_guard(io_context);
  io_context.run();
}