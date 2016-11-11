#pragma once

#include <cstddef>
#include <vector>

namespace agora {
namespace base {

enum open_mode {
  kRead = 1,
  kWrite = 2,
};

class buffered_pipe {
 public:
  explicit buffered_pipe(int fd, open_mode mode=kRead);
  ~buffered_pipe();

  bool is_open() const;

  bool set_buffer_size(size_t size);

  size_t read_buffer(void *buf, size_t size, size_t nmemb);
  size_t write_buffer(const void *buf, size_t size);

  bool flush();

  bool is_eof();
  bool has_error() const;
 private:
  bool underflow();
  bool overflow();
  bool flush_internal();
  bool put_back(const void *buf, size_t n);
 private:
  int fd_;
  open_mode mode_;

  std::vector<char> buffer_;

  char *buf_start_;
  char *buf_end_;

  struct read_info {
    char *read_ptr;
    char *end_ptr;
  } read_;

  struct write_info {
    char *write_ptr;
    char *end_ptr;
  } write_;
};

}
}
