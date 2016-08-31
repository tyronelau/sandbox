#pragma once

#include <cstdint>
#include <cstdio>
#include <climits>

namespace agora {
namespace base {
class packet;
class unpacker;

enum ReadState {kNoData=0, kEof, kError, kReceived};
enum WriteState {kWriteOk=0, kWriteError, kRetryExceeded};

// A non-blocking packet-oriented pipe reader
class pipe_reader {
  static const unsigned int kSize = 4096;
  static const unsigned int kMaxPacketLength = 512;
 public:
  explicit pipe_reader(int fd);
  ~pipe_reader();

  bool is_good() const;
  bool is_closed() const;

  bool close();
  ReadState read_packet(unpacker *p, uint16_t *uri);
 private:
  char partial_packet_[kMaxPacketLength];
  int partial_packet_length_;
  FILE *pipe_;
 private:
  pipe_reader(const pipe_reader &) = delete;
  pipe_reader(pipe_reader &&) = delete;
  pipe_reader& operator=(const pipe_reader &) = delete;
  pipe_reader& operator=(pipe_reader &&) = delete;
};

// A blocking packet-oriented pipe writer
class pipe_writer {
 public:
  explicit pipe_writer(int fd);
  ~pipe_writer();

  bool is_good() const;

  // false means the pipe has been closed by the peer
  WriteState write_packet(const packet &p, int retry=INT_MAX);
  WriteState write_buffer(const void *p, size_t n, int retry=1);
 private:
  int pipe_;
 private:
  pipe_writer(const pipe_writer &) = delete;
  pipe_writer(pipe_writer &&) = delete;
  pipe_writer& operator=(const pipe_writer &) = delete;
  pipe_writer& operator=(pipe_writer &&) = delete;
};

}
}

