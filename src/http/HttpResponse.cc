//
// Created by 13454 on 2025/7/22.
//

#include <iostream>
#include "HttpResponse.h"

#include <fstream>

std::string NETCPP::HttpResponse::ToString() const {
  std::string response = "HTTP/1.1 " + std::to_string(status_code_) + " " + status_message_ + "\r\n";
  for (const auto &header: headers_) {
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

void NETCPP::HttpResponse::SendFile(const std::filesystem::path &path) {
  // 获取文件后缀
  auto extension = path.extension().string();
  // 根据后缀设置Content-Type
  if (extension == ".html") {
    SetHeader("Content-Type", "text/html;charset=utf-8");
  } else if (extension == ".css") {
    SetHeader("Content-Type", "text/css;charset=utf-8");
  } else if (extension == ".js") {
    SetHeader("Content-Type", "application/javascript;charset=utf-8");
  } else if (extension == ".json") {
    SetHeader("Content-Type", "application/json;charset=utf-8");
  } else if (extension == ".png") {
    SetHeader("Content-Type", "image/png");
  } else if (extension == ".jpg" || extension == ".jpeg") {
    SetHeader("Content-Type", "image/jpeg");
  } else if (extension == ".gif") {
    SetHeader("Content-Type", "image/gif");
  } else {
    SetHeader("Content-Type", "application/octet-stream");
  }
  std::ifstream ifs(path, std::ios::in);
  if (!ifs.is_open()) {
    std::cerr << "Open file error: " << path.string() << std::endl;
    return;
  }
  // 读取文件内容
  std::stringstream ss;
  ss << ifs.rdbuf();
  body_ = ss.str();
}

