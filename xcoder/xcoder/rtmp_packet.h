#pragma once

#include <cstring>
#include <memory>
#include <new>
#include <utility>

namespace agora {
namespace recording {

class rtmp_packet {
  friend class rtmp_stream;
 public:
  rtmp_packet(const char *frame, int size, uint32_t frame_ts, uint32_t send_ts);
  ~rtmp_packet();

  rtmp_packet(rtmp_packet &&);
  rtmp_packet& operator=(rtmp_packet &&);
 private:
  rtmp_packet(const rtmp_packet &) = delete;
  rtmp_packet& operator=(const rtmp_packet &) = delete;
 private:
  std::unique_ptr<char[]> frame_;
  uint32_t size_;
  uint32_t frame_ts_;
  uint32_t send_ts_;
};

inline rtmp_packet::rtmp_packet(const char *frame, int size,
    uint32_t frame_ts, uint32_t send_ts) {
  frame_.reset(new (std::nothrow)char[size]);
  if (frame_) {
    memcpy(frame_.get(), frame, size);
  }

  size_ = size;
  frame_ts_ = frame_ts;
  send_ts_ = send_ts;
}

inline rtmp_packet::rtmp_packet(rtmp_packet &&rhs) {
  std::swap(frame_, rhs.frame_);

  size_ = rhs.size_;
  frame_ts_ = rhs.frame_ts_;
  send_ts_ = rhs.send_ts_;
}

inline rtmp_packet::~rtmp_packet() {
}

inline rtmp_packet& rtmp_packet::operator=(rtmp_packet &&rhs) {
  if (this != &rhs) {
    std::swap(frame_, rhs.frame_);

    std::swap(size_, rhs.size_);
    std::swap(frame_ts_, rhs.frame_ts_);
    std::swap(send_ts_, rhs.send_ts_);
  }

  return *this;
}

}
}
