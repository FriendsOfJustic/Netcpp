//
// Created by 13454 on 2025/7/25.
//

#include "Buffer.h"

#include <stdexcept>

namespace NETCPP {
  Buffer::Edian getEdian() {
    uint32_t test = 0x01020304;
    return (*(char *) &test == 0x01) ? Buffer::BIG_ENDIAN : Buffer::LITTLE_ENDIAN;
  }

  Buffer::Edian Buffer::hostEndian = getEdian();

  uint32_t Buffer::NetToHostInt32(uint32_t value) {
    if (hostEndian == BIG_ENDIAN) {
      return value;
    } else {
      return (value << 24) | ((value << 8) & 0x00ff0000) | ((value >> 8) & 0x0000ff00) | (value >> 24);
    }
  }

  uint32_t Buffer::HostToNetInt32(uint32_t value) {
    if (hostEndian == BIG_ENDIAN) {
      return value;
    } else {
      return (value << 24) | ((value << 8) & 0x00ff0000) | ((value >> 8) & 0x0000ff00) | (value >> 24);
    }
  }

  Buffer::Buffer(size_t capacity) : capacity_(capacity), read_pos_(0), write_pos_(0) {
    buffer_.resize(capacity_);
  }


  Buffer::~Buffer() {
    buffer_.clear();
  }

  size_t Buffer::size() {
    return write_pos_ - read_pos_;
  }

  void Buffer::write(const char *data, size_t len) {
    // 如果len大于剩余空间 扩容
    if (len > capacity_ - size()) {
      std::vector<char> new_buffer(capacity_ + len);
      std::copy(buffer_.begin() + read_pos_, buffer_.end(), new_buffer.begin());
      buffer_ = std::move(new_buffer);
      capacity_ += len;
    }
    if (len > capacity_ - write_pos_) {
      auto sz = size();
      std::copy(buffer_.begin() + read_pos_, buffer_.end(), buffer_.begin());
      write_pos_ = sz;
      read_pos_ = 0;
    }
    std::copy(data, data + len, buffer_.begin() + write_pos_);
    write_pos_ += len;
  }

  void Buffer::read(char *data, size_t len) {
    if (len > size()) {
      len = size();
    }
    std::copy(buffer_.begin() + read_pos_, buffer_.begin() + read_pos_ + len, data);
    read_pos_ += len;
  }

  std::string Buffer::retrieveString(size_t len) {
    std::string ret;
    auto sz = std::min(len, size());
    ret.append(getReadPos(), sz);
    read_pos_ += sz;
    return ret;
  }

  uint32_t Buffer::peekUInt32() {
    if (size() < sizeof(int32_t)) {
      throw std::runtime_error("Buffer::peekInt32: not enough data");
    }
    uint32_t ret = 0;
    std::copy(buffer_.begin() + read_pos_, buffer_.begin() + read_pos_ + sizeof(uint32_t),
              reinterpret_cast<char *>(&ret));
    ret = NetToHostInt32(ret);
    return ret;
  }

  uint32_t Buffer::retrieveUInt32() {
    uint32_t ret = peekUInt32();
    read_pos_ += sizeof(uint32_t);
    return ret;
  }


  void Buffer::writeUInt32(uint32_t value) {
    value = HostToNetInt32(value);
    write((char *) &value, sizeof(int32_t));
  }

  void Buffer::discard(size_t len) {
    if (len > size()) {
      len = size();
    }
    read_pos_ += len;
  }

  std::string Buffer::getLine() {
    std::string crlf;
    for (auto i = read_pos_; i < write_pos_ - 1; ++i) {
      if (buffer_[i] == '\n') {
        crlf += buffer_[i];
        crlf += buffer_[i + 1];
        break;
      }
    }
    return crlf;
  }

  bool Buffer::getCRLF(std::string &line) {
    std::string crlf;
    bool ret = false;
    for (auto i = read_pos_; i < write_pos_ - 1; ++i) {
      if (buffer_[i] == '\r' && buffer_[i + 1] == '\n') {
        ret = true;
        break;
      }
      crlf += buffer_[i];
    }

    if (ret) {
      line = crlf;
    }
    return ret;
  }
} // NETCPP
