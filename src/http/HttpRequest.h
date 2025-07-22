//
// Created by 13454 on 2025/7/22.
//

#ifndef NETCPP_SRC_HTTPREQUEST_H_
#define NETCPP_SRC_HTTPREQUEST_H_
#include "string"

#include "map"

#include "asio.hpp"
namespace NETCPP {

class HttpRequest {
 public:

  enum ParseState {
    kRequestLine,
    kHeaders,
    kBody,
    kDone
  };
  HttpRequest() = default;
  ~HttpRequest() = default;
  void SetMethod(const std::string &method) { method_ = method; }
  void SetPath(const std::string &path) { path_ = path; }
  void SetVersion(const std::string &version) { version_ = version; }
  void SetHeader(const std::string &key, const std::string &value) { headers_[key] = value; }
  void SetBody(const std::string &body) { body_ = body; }
  std::string GetMethod() const { return method_; }
  std::string GetPath() const { return path_; }
  std::string GetVersion() const { return version_; }
  std::string GetHeader(const std::string &key) const { return headers_.at(key); }
  std::string GetBody() const { return body_; }

  bool Parse(asio::streambuf &buffer);
 private:

  void ParseRequestLine(const std::string &line);
  void ParseHeader(const std::string &line);
  void ParseBody(asio::streambuf &buffer);

  ParseState state_ = kRequestLine;
  std::string method_;
  std::string path_;
  std::string version_;
  std::map<std::string, std::string> headers_;
  std::string body_;

  uint64_t content_length_ = -1;

};
}

#endif //NETCPP_SRC_HTTPREQUEST_H_
