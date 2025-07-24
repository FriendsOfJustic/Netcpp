//
// Created by 13454 on 2025/7/22.
//

#include <iostream>
#include "HttpResponse.h"

std::string NETCPP::HttpResponse::ToString() const {
  std::string response = "HTTP/1.1 " + std::to_string(status_code_) + " " + status_message_ + "\r\n";
  for (const auto &header : headers_) {
    response += header.first + ": " + header.second + "\r\n";
  }
  if (!body_.empty()) {
    response += "Content-Length: " + std::to_string(body_.size()) + "\r\n";
  }
  response += "\r\n";
  if (!body_.empty()) {
    response += body_;
  }
  return response;
}
void NETCPP::HttpResponse::JSON(const nlohmann::json &json) {
  SetHeader("Content-Type", "application/json;charset=utf-8");
  try {
    body_ = json.dump();
  } catch (const nlohmann::json::exception &e) {
    std::cerr << "JSON dump error: " << e.what() << std::endl;
  }
}

