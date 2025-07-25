//
// Created by 13454 on 2025/7/22.
//

#include "HttpRequest.h"
bool NETCPP::HttpRequest::Parse(Buffer &buffer) {

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

    std::string line;
    if (!buffer.getCRLF(line)) {
      return false;
    }
    buffer.discard(line.size() + 2);
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
void NETCPP::HttpRequest::ParseBody(Buffer &buffer) {
  if (content_length_ == 0) {
    state_ = kDone;
    return;
  }
  auto sz = content_length_;
  auto begin = buffer.getBuffer(sz);
  body_.insert(body_.end(), begin, begin + sz);
  content_length_ -= sz;
  buffer.discard(sz);
}
std::string NETCPP::HttpRequest::ToString() const {
  std::string str;
  str += method_ + " " + path_ + " " + version_ + "\r\n";
  for (const auto &header : headers_) {
    str += header.first + ": " + header.second + "\r\n";
  }
  str += "\r\n";
  str += body_;
  return str;
}
