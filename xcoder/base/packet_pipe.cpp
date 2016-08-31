#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <cerrno>
#include <cstdio>
#include <utility>

#include "base/log.h"
#include "base/packer.h"
#include "base/packet.h"
#include "packet_pipe.h"

namespace agora {
namespace base {
pipe_reader::pipe_reader(int fd) {
  fcntl(fd, F_SETFL, O_NONBLOCK);

  pipe_ = fdopen(fd, "rb");
  if (!pipe_) {
    LOG(FATAL, "Failed to open the fd: %m");
  }

  setvbuf(pipe_, NULL, _IOFBF, 4096);

  partial_packet_length_ = 0;
}

pipe_reader::~pipe_reader() {
  close();
}

bool pipe_reader::close() {
  if (!pipe_ || (pipe_ && !fclose(pipe_))) {
    pipe_ = NULL;
    return true;
  }

  return false;
}

bool pipe_reader::is_good() const {
  pollfd pfd = {fileno(pipe_), POLLIN, 0};
  if (poll(&pfd, 1, 0) < 0 || (pfd.revents & POLLERR) ||
      (pfd.revents & POLLRDHUP) || (pfd.revents & POLLHUP))
    return false;
  return true;
}

bool pipe_reader::is_closed() const {
  return pipe_ == NULL;
}

ReadState pipe_reader::read_packet(unpacker *p, uint16_t *uri) {
  // First, check if a pending packet already in the buffer.
  if (partial_packet_length_ < 2) {
    char *buf = partial_packet_ + partial_packet_length_;
    int n = fread(buf, 1, 2 - partial_packet_length_, pipe_);
    partial_packet_length_ += n;
    if (partial_packet_length_ != 2) {
      if (feof(pipe_) || errno == EPIPE) {
        // broken pipe
        return kError;
      }

      return kNoData;
    }
  }

  union {
    char bytes[2];
    uint16_t len;
  };

  bytes[0] = partial_packet_[0];
  bytes[1] = partial_packet_[1];

  if (len < 6) {
    // A packet has no user defined data.
    return kError;
  }

  if (len > kMaxPacketLength) {
    LOG(ERROR, "A huge packet received, Not expected: %d", len);
    return kError;
  }

  if (partial_packet_length_ < len) {
    char *buf = partial_packet_ + partial_packet_length_;
    int n = fread(buf, 1, len - partial_packet_length_, pipe_);
    partial_packet_length_ += n;

    if (partial_packet_length_ != len) {
      if (feof(pipe_) || errno == EPIPE) {
        // eof
        return kError;
      }

      return kNoData;
    }
  }

  // OK, we have received a full packet.
  unpacker parser(partial_packet_, len);

  // pop up the field |packet_length|
  parser.pop_uint16();

  // unused server_type
  parser.pop_uint16();

  *uri = parser.pop_uint16();

  parser.rewind();

  *p = std::move(parser);

  partial_packet_length_ = 0;
  return kReceived;
}

pipe_writer::pipe_writer(int fd) {
  pipe_ = fd;
}

pipe_writer::~pipe_writer() {
  close(pipe_);
}

bool pipe_writer::is_good() const {
  pollfd pfd = {pipe_, POLLOUT, 0};
  if (poll(&pfd, 1, 0) < 0 || (pfd.revents & POLLERR) ||
      (pfd.revents & POLLRDHUP) || (pfd.revents & POLLHUP))
    return false;
  return true;
}

WriteState pipe_writer::write_packet(const packet &p, int retry) {
  packer pk;
  p.pack(pk);

  return write_buffer(pk.buffer(), pk.length(), retry);
}

WriteState pipe_writer::write_buffer(const void *p, size_t n, int retry) {
  if (n > PIPE_BUF) {
    LOG(ERROR, "A huge buffer %d not supported", static_cast<int>(n));
    return kWriteError;
  }

  const char *buf = reinterpret_cast<const char *>(p);
  ssize_t written = 0;
  int i = 0;

  while ((written = write(pipe_, buf, n)) != ssize_t(n)) {
    if (written >= 0) {
      buf += written;
      n -= written;
      if (written == 0) {
        LOG(INFO, "zero packets");
      }
      continue;
    }

    if (written == -1 && errno == EAGAIN) {
      if (i++ < retry)
        continue;
      else
        return kRetryExceeded;
    }

    //  EPIPE/EFAULT, etc
    return kWriteError;
  }

  return kWriteOk;
}

}
}

