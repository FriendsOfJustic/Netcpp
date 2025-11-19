//
// Created by 13454 on 2025/7/25.
//

#ifndef NETCPP_SRC_BUFFER_H_
#define NETCPP_SRC_BUFFER_H_
#include <vector>
#include "cstdint"

#include "string"
namespace NETCPP {

class Buffer {

 public:
  Buffer(size_t capacity = 1024);
  ~Buffer();
  size_t size();
  size_t capacity();

  std::string getLine();
  bool getCRLF(std::string &line);

  void discard(size_t len);
  void write(const char *data, size_t len);
  void read(char *data, size_t len);

  char *getWritePos() { return buffer_.data() + write_pos_; }
  char *getReadPos() { return buffer_.data() + read_pos_; }

  char *getBuffer(size_t &len) {
    if (len > size()) {
      len = size();
    }
    return buffer_.data() + read_pos_;
  }

    std::string Retrieve(size_t len);
 private:
  std::vector<char> buffer_;
  size_t read_pos_;
  size_t write_pos_;
  size_t capacity_;
};

} // NETCPP

#endif //NETCPP_SRC_BUFFER_H_
