//
// Created by 13454 on 2025/7/24.
//

#ifndef NETCPP_APP_FILEDOWNLOADER_UPLOADER_H_
#define NETCPP_APP_FILEDOWNLOADER_UPLOADER_H_
#include "filesystem"
#include "asio.hpp"
#include "EventLoopThread.h"
namespace fs = std::filesystem;
class uploader {
 public:
  uploader(const std::string &url, uint16_t port);
  ~uploader();
  void uploadFile(const fs::path &filename);

 private:
  void uploadBlock(const fs::path &filename, int32_t block_id, uint64_t block_size);
  NETCPP::EventLoopThreadPool pool_;
  asio::io_context io_context_;
  std::string ip_;
  uint16_t port_;
  std::string filename_;
};

#endif //NETCPP_APP_FILEDOWNLOADER_UPLOADER_H_
