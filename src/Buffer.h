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
    enum Edian {
      BIG_ENDIAN,
      LITTLE_ENDIAN,
    };

    static Edian hostEndian;


    static int32_t NetToHostInt32(int32_t value);

    static int32_t HostToNetInt32(int32_t value);

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

    std::string retrieveString(size_t len);


    int32_t peekInt32();

    int32_t retrieveInt32();


    void writeInt32(int32_t value);

  private:
    std::vector<char> buffer_;
    size_t read_pos_;
    size_t write_pos_;
    size_t capacity_;
  };
} // NETCPP

#endif //NETCPP_SRC_BUFFER_H_
