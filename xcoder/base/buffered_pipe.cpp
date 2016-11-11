#include "base/buffered_pipe.h"

#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <cstring>

namespace agora {
namespace base {
static const unsigned kDefaultSize = 128 * 1024;

buffered_pipe::buffered_pipe(int fd, open_mode mode) {
  fd_ = fd;
  mode_ = mode;
  eof_ = false;

  buffer_.resize(kDefaultSize);

  buf_start_ = &buffer_[0];
  buf_end_ = buf_start_ + buffer_.size();

  if (mode == kRead) {
    read_.read_ptr = &buffer_[0];
    read_.end_ptr = read_.read_ptr;
  } else {
    write_.write_ptr = buf_start_;
    write_.end_ptr = buf_start_;
  }
}

buffered_pipe::~buffered_pipe() {
  flush();
}

size_t buffered_pipe::read_buffer(void *buf, size_t size, size_t nmemb) {
  size_t length = size * nmemb;
  if (length == 0 || mode_ != kRead)
    return 0;

  char *dst = reinterpret_cast<char *>(buf);
  size_t readed = 0;
  while (length != 0) {
    size_t n = std::min<size_t>(length, read_.end_ptr - read_.read_ptr);
    if (n == 0) {
      if (!underflow()) {
        break;
      }

      continue;
    }

    memcpy(dst, read_.read_ptr, n);

    length -= n;
    dst += n;
    read_.read_ptr += n;
    readed += n;
  }

  size_t n = readed / size;
  size_t r = readed - n * size;
  if (r != 0) {
    dst = reinterpret_cast<char *>(buf) + n * size;
    put_back(dst, r);
  }

  return n;
}

bool buffered_pipe::put_back(const void *buf, size_t n) {
  // FIXME(liuyong)
  assert(mode_ == kRead);

  // const char *p = reinterpret_cast<const char *>(buf);

  size_t left = static_cast<size_t>(read_.read_ptr -  buf_start_);
  if (left < n) {
    size_t old_size = read_.end_ptr - read_.read_ptr;
    size_t new_size = n + old_size;
    new_size = ((new_size + kDefaultSize - 1) / kDefaultSize ) * kDefaultSize;

    std::vector<char> tmp;
    tmp.resize(new_size);

    memcpy(&tmp[n], read_.read_ptr, old_size);
    buffer_.swap(tmp);

    buf_start_ = &buffer_[0];
    buf_end_ = buf_start_ + buffer_.size();
    read_.read_ptr = buf_start_ + n;
    read_.end_ptr = read_.read_ptr + old_size;
  }

  char *begin = read_.read_ptr - n;
  memcpy(begin, buf, n);
  read_.read_ptr -= n;

  return true;
}

size_t buffered_pipe::write_buffer(const void *buf, size_t length) {
  if (length == 0 || mode_ != kWrite)
    return 0;

  const char *src = reinterpret_cast<const char *>(buf);
  size_t written = 0;
  while (length != 0) {
    size_t n = std::min<size_t>(length, buf_end_ - write_.end_ptr);
    if (n == 0) {
      if (!overflow())
        break;
      continue;
    }

    memcpy(write_.end_ptr, src, n);

    write_.end_ptr += n;
    src += n;
    length -= n;
    written += n;
  }

  return written;
}

bool buffered_pipe::underflow() {
  assert(buffer_.size() > 0);
  assert(read_.read_ptr == read_.end_ptr);

  read_.read_ptr = &buffer_[0];
  read_.end_ptr = read_.read_ptr;

  ssize_t readed = read(fd_, read_.read_ptr, buffer_.size());
  if (readed <= 0) {
    eof_ = (readed == 0);
    return false;
  }

  read_.end_ptr = read_.read_ptr + readed;
  return true;
}

bool buffered_pipe::overflow() {
  assert(buffer_.size() > 0);
  assert(buf_end_ == write_.end_ptr);

  return flush_internal();
}

bool buffered_pipe::flush_internal() {
  size_t n = static_cast<size_t>(write_.end_ptr - write_.write_ptr);
  ssize_t written = write(fd_, write_.write_ptr, n);
  if (written <= 0) {
    if (written == -1 && errno == EPIPE) {
      eof_ = true;
    }
    return false;
  }

  write_.write_ptr += written;
  if (write_.write_ptr == write_.end_ptr) {
    write_.write_ptr = buf_start_;
    write_.end_ptr = buf_start_;
  }

  return true;
}

bool buffered_pipe::flush() {
  if (mode_ != kWrite)
    return false;

  while (write_.write_ptr != write_.end_ptr && flush_internal()) {
  }

  return write_.write_ptr == write_.end_ptr;
}

}
}
