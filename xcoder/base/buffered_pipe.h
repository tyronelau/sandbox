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

  size_t read_buffer(void *buf, size_t length);
  size_t write_buffer(const void *buf, size_t length);

  bool is_eof();
  bool has_error() const;
 private:
  bool underflow();
 private:
  int fd_;
  open_mode mode_;

  std::vector<char> buffer_;
  char *read_ptr_;
  char *end_ptr_;
};

}
}
