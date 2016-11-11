#include "base/buffered_pipe.h"

#include <unistd.h>
#include <cassert>
#include <cstring>

namespace agora {
namespace base {
buffered_pipe::buffered_pipe(int fd, open_mode mode) {
  fd_ = fd;
  mode_ = mode;

  static const unsigned kDefaultSize = 256 * 1024;

  buffer_.resize(kDefaultSize);
  read_ptr_ = &buffer_[0];
  end_ptr_ = read_ptr_;
}

size_t buffered_pipe::read_buffer(void *buf, size_t length) {
  if (length == 0 || mode_ != kRead)
    return 0;

  char *dst = reinterpret_cast<char *>(buf);
  size_t readed = 0;
  while (true) {
    size_t n = std::min<size_t>(length, end_ptr_ - read_ptr_);
    if (n == 0) {
      if (!underflow()) {
        break;
      }

      continue;
    }

    memcpy(dst, read_ptr_, n);

    length -= n;
    dst += n;
    read_ptr_ += n;
    readed += n;

    if (length == 0)
      break;
  }

  return readed;
}

bool buffered_pipe::underflow() {
  assert(buffer_.size() > 0);
  assert(read_ptr_ == end_ptr_);

  read_ptr_ = &buffer_[0];
  end_ptr_ = read_ptr_;

  ssize_t readed = read(fd_, read_ptr_, buffer_.size());
  if (readed <= 0)
    return false;

  end_ptr_ = read_ptr_ + readed;
  return true;
}

bool buffered_pipe::is_eof() {
  if (read_ptr_ != end_ptr_)
    return false;

  return true;
}

}
}
