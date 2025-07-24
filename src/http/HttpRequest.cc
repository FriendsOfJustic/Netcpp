//
// Created by 13454 on 2025/7/22.
//

#include "HttpRequest.h"
bool NETCPP::HttpRequest::Parse(asio::streambuf &buffer) {

  while (state_ != kDone) {
    if (state_ == kBody) {
      if (content_length_ == -1) {
        if (headers_.find("Content-Length") == headers_.end())
          return true;
        else {
          content_length_ = std::stoll(headers_["Content-Length"]);
          body_.reserve(content_length_);
        }
      }
      ParseBody(buffer);
      continue;
    }
    std::istream input(&buffer);
    std::string line;
    if (!std::getline(input, line, '\r')) {
      return false;
    }
    input.ignore(1);
    if (state_ == kRequestLine) {
      ParseRequestLine(line);
      state_ = kHeaders;
      continue;
    }
    if (state_ == kHeaders) {
      if (line.empty()) {
        state_ = kBody;
        continue;
      }
      ParseHeader(line);
      continue;
    }
  }

  return true;
}
void NETCPP::HttpRequest::ParseRequestLine(const std::string &line) {
  auto pos = line.find(' ');
  if (pos == std::string::npos) {
    return;
  }
  method_ = line.substr(0, pos);
  auto pos2 = line.find(' ', pos + 1);
  if (pos2 == std::string::npos) {
    return;
  }
  path_ = line.substr(pos + 1, pos2 - pos - 1);
  version_ = line.substr(pos2 + 1);
}
void NETCPP::HttpRequest::ParseHeader(const std::string &line) {
  auto pos = line.find(':');
  if (pos == std::string::npos) {
    return;
  }
  auto key = line.substr(0, pos);
  auto value = line.substr(pos + 1);
  headers_[key] = value;
}
void NETCPP::HttpRequest::ParseBody(asio::streambuf &buffer) {
  if (content_length_ == 0) {
    state_ = kDone;
    return;
  }
  auto sz = std::min(buffer.size(), content_length_);
  std::copy(asio::buffers_begin(buffer.data()), asio::buffers_begin(buffer.data()) + sz,
            std::back_inserter(body_));
  content_length_ -= sz;
  buffer.consume(sz);
}
