//
// Created by 13454 on 2025/7/22.
//

#include "HttpServer.h"
using namespace NETCPP;
#include "Type.h"
void NETCPP::HttpServer::TcpReadCallback(NETCPP::ConnectionPtr ptr) {
  if (!ptr->Context().has_value()) {
    ptr->SetContext(std::make_any<HttpContext>(ptr));
  }
  auto &context = std::any_cast<HttpContext &>(ptr->Context());
  if (!context.GetRequest().Parse(ptr->ReadBuffer())) {
    return;
  }
  if (!handler_) {
    handler_ = [this](HttpContext &ctx) {
      Dispatch(ctx);
    };
  }
  handler_(context);
  ptr->Write(context.GetResponse().ToString());
  ptr->ShutDown();
}
void NETCPP::HttpServer::AddRoute(const HTTP_METHOD &method,
                                  const std::string &path,
                                  const HttpHandler &handler) {
  router_.addRoute(method, path, handler);
}
void HttpServer::POST(const std::string &path, const HttpHandler &handler) {
  AddRoute(HTTP_METHOD::POST, path, handler);
}
void HttpServer::PUT(const std::string &path, const HttpHandler &handler) {
  AddRoute(HTTP_METHOD::PUT, path, handler);
}
void HttpServer::GET(const std::string &path, const HttpHandler &handler) {
  AddRoute(HTTP_METHOD::GET, path, handler);
}
void HttpServer::Dispatch(HttpContext &ctx) {
  auto &req = ctx.GetRequest();
  auto &resp = ctx.GetResponse();
  auto handler = router_.getHandler(req.GetMethod(), req.GetPath());
  if (handler) {
    handler(ctx);
  } else {
    resp.SetStatusCode(404);
    resp.SetStatusMessage("Not Found");
    resp.SetHeader("Content-Type", "text/plain;charset=utf-8");
    resp.SetBody("404 Not Found");
  }
}

void Router::addRoute(const HTTP_METHOD &method, const std::string &path, const HttpHandler &handler) {
  routes_[method][path] = handler;
}
HttpHandler Router::getHandler(const std::string &method, const std::string &path) const {
  auto method_it = routes_.find(STRING2HTTP_METHOD[method]);
  if (method_it != routes_.end()) {
    auto path_it = method_it->second.find(path);
    if (path_it != method_it->second.end()) {
      return path_it->second;
    }
  }
  return nullptr;
}
