//
// Created by 13454 on 2025/7/24.
//

#include <fstream>
#include "uploader.h"
#include "http/HttpRequest.h"
#include "TcpClient.h"
#include "spdlog/spdlog.h"
int32_t BLOCK_SIZE = 32 * 1024 * 1024;

struct Block {
  int32_t block_id;
  uint64_t block_size;
};

void uploader::uploadFile(const fs::path &filename) {
  if (!fs::is_regular_file(filename)) {
    throw std::runtime_error("file not exist");
  }

  // 获取文件大小
  uint64_t file_size = fs::file_size(filename);
  std::cout << "file size: " << file_size << std::endl;
  // 计算块数
  int32_t block_num = file_size / BLOCK_SIZE;
  if (file_size % BLOCK_SIZE != 0) {
    block_num++;
  }
  NETCPP::TcpClient client(io_context_);
  client.Connect(ip_, 8080);
  // 发送文件信息
  NETCPP::HttpRequest req;
  req.SetMethod("GET");
  req.SetPath("/info");
  req.SetHeader("FILE_NAME", filename.string());
  req.SetHeader("FILE_SIZE", std::to_string(file_size));
  req.SetHeader("BLOCK_NUM", std::to_string(block_num));
  req.SetHeader("BLOCK_SIZE", std::to_string(BLOCK_SIZE));
  client.Send(req.ToString());

  std::vector<Block> blocks;
  for (int32_t i = 0; i < block_num; ++i) {
    auto sz = i == block_num - 1 ? file_size % BLOCK_SIZE : BLOCK_SIZE;
    uploadBlock(filename, i, sz);
  }

}
void uploader::uploadBlock(const fs::path &filename, int32_t block_id, uint64_t block_size) {
  std::ifstream file(filename, std::ios::binary);
  file.seekg(block_id * BLOCK_SIZE);
  std::string data(block_size, 0);
  file.read(data.data(), block_size);
  auto &loop = pool_.GetNextLoop();
  asio::post(loop, [str = std::move(data), &loop, block_id, this]() {
    NETCPP::TcpClient client(loop);
    client.AsyncConnect(ip_,
                        port_,
                        [data = std::move(str), block_id](NETCPP::TcpClient::TcpClientPtr ptr) {
                          NETCPP::HttpRequest req;
                          req.SetMethod("POST");
                          req.SetPath("/upload");
                          req.SetHeader("BLOCK_ID", std::to_string(block_id));
                          req.SetHeader("BLOCK_SIZE", std::to_string(data.size()));
                          req.SetBody(data);
                          ptr->AsyncSend(req.ToString(), [](NETCPP::TcpClient::TcpClientPtr ptr) {
                            spdlog::info("send data success");
                          });
                        });
  });
}
