//
// Created by 13454 on 2025/7/24.
//

#ifndef NETCPP_APP_FILEDOWNLOADER_UTILS_H_
#define NETCPP_APP_FILEDOWNLOADER_UTILS_H_

#include <string>
template<typename T>
std::string to_net_endian(T value) {
  std::string result(sizeof(T), 0);
  for (int i = 0; i < sizeof(T); ++i) {
    result[i] = value & 0xFF;
    value >>= 8;
  }
  return result;
}
template<typename T>
T from_net_endian(const std::string &data) {
  T result = 0;
  for (int i = 0; i < sizeof(T); ++i) {
    result <<= 8;
    result |= data[i];
  }
  return result;
}
template<typename T>
T from_net_endian(const char *p) {
  T result = 0;
  for (int i = 0; i < sizeof(T); ++i) {
    result <<= 8;
    result |= p[i];
  }
  return result;
}
#include "random"
// uuid
std::string generate_uuid() {

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 255);
  std::string uuid;
  for (int i = 0; i < 16; ++i) {
    uuid += std::to_string(dis(gen));
  }
  return uuid;
}

#endif //NETCPP_APP_FILEDOWNLOADER_UTILS_H_
