//
// Created by 13454 on 2025/7/22.
//

#ifndef NETCPP_SRC_HTTPRESPONSE_H_
#define NETCPP_SRC_HTTPRESPONSE_H_
#include "string"
#include "Connection.h"
#include "map"
#include "asio.hpp"
#include "nlohmann/json.hpp"
namespace NETCPP {
class HttpResponse {
 public:
  HttpResponse() = default;
  ~HttpResponse() = default;
  HttpResponse(std::weak_ptr<Connection> connection) : connection_(std::move(connection)) {}
  void SetStatusCode(int status_code) { status_code_ = status_code; }
  void SetStatusMessage(const std::string &status_message) { status_message_ = status_message; }
  void SetHeader(const std::string &key, const std::string &value) { headers_[key] = value; }
  void SetBody(const std::string &body) { body_ = body; }
  int GetStatusCode() const { return status_code_; }
  std::string GetStatusMessage() const { return status_message_; }
  std::string GetHeader(const std::string &key) const { return headers_.at(key); }
  std::string GetBody() const { return body_; }
  void SetBody(const char *data, size_t len) { body_ = std::string(data, len); }
  void SetBody(const std::vector<char> &data) { body_ = std::string(data.data(), data.size()); }
  void JSON(const nlohmann::json &json) { body_ = json.dump(); }
  std::string ToString() const;

 private:

  std::weak_ptr<Connection> connection_;
  int status_code_ = 200;
  std::string status_message_ = "OK";
  std::map<std::string, std::string> headers_;
  std::string body_;
};
}

#endif //NETCPP_SRC_HTTPRESPONSE_H_
