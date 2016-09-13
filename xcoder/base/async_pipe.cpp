#include "base/async_pipe.h"

#include <cassert>

#include "base/event_loop.h"

namespace agora {
namespace base {
async_pipe_reader::async_pipe_reader(event_loop *loop, int fd,
    pipe_read_listener *listener) {
  loop_ = loop;
  pipe_fd_ = fd;
  listener_ = listener;

  setup_callback();
}

async_pipe_reader::~async_pipe_reader() {
  remove_callback();

  if (fp_) {
    fclose(fp_);
  }
}

bool async_pipe_reader::close() {
  if (!closed_) {
    loop_->remove_watcher(fd, this, NULL, NULL, NULL);
    closed_ = true;
  }

  return closed_;
}

void async_pipe_reader::setup_callback() {
  loop_->add_watcher(pipe_fd_, this, &read_callback, NULL, &error_callback);
}

void async_pipe_reader::remove_callback() {
  loop_->remove_watcher(pipe_fd_, this, &read_callback, NULL, &error_callback);
}

bool async_pipe_reader::is_closed() const {
  return closed_;
}

void async_pipe_reader::read_callback(int fd, void *context) {
  async_pipe_reader *reader = reinterpret_cast<async_pipe_reader *>(context);
  reader->on_read();
}

void async_pipe_reader::error_callback(int fd, void *context) {
  async_pipe_reader *reader = reinterpret_cast<async_pipe_reader *>(context);
  reader->on_error();
}

void async_pipe_reader::on_read() {
  processing_ = true;

  while (!is_closed()) {
    if (packet_ == NULL) {
      // Read next packet
      uint32_t size = 0;
      if ((n = fread(&size, sizeof(size), 1, fp_)) == 1) {
        char *buf = new (std::nothrow)char[size];
        packet_common_header *pkt = reinterpret_cast<char *>(buf);
        pkt->size = size;
        readed_ = sizeof(size);
        packet_ = pkt;
      } else {
        break;
      }
    }

    char *next = reinterpret_cast<char *>(packet_) + readed_;
    size_t toread = packet_->size - readed_;
    size_t n = fread(next, 1, toread, fp_);
    if (n == toread) {
      if (listener_) {
        listener_->on_receive_packet(packet_);
      } else {
        delete packet_;
        packet_ = NULL;
      }
    } else {
      break;
    }
  }

  processing_ = false;
  if (is_closed())
    destroy();

  if (feof(fp_) && listener_) {
    listener_->on_error(0);
  }
}

void async_pipe_reader::on_error() {
  if (listener_) {
    listener_->on_error(0);
  }
}

void async_pipe_reader::destroy() {
  delete this;
}

async_pipe_writer::async_pipe_writer(event_loop *loop, int fd,
    pipe_write_listener *listener) {
  loop_ = loop;
  pipe_fd_ = fd;

  if ((fp_ = fdopen(pipe_fd_, "a")) == NULL) {
    SAFE_LOG(FATAL) << "Failed to open the pipe to write";
    return;
  }

  closed_ = false;
  processing_ = false;
  packet_ = NULL;

  listener_ = listener;
  setup_callback();
}

async_pipe_writer::~async_pipe_writer() {
  if (fp_) {
    fclose(fp_);
  }

  delete [] reinterpret_cast<char *>(packet_);
  remove_callback();
}

bool async_pipe_writer::close() {
  if (!closed_) {
    loop_->remove_watcher(fd, this, NULL, NULL, NULL);
    closed_ = true;
  }

  return closed_;
}

bool async_pipe_writer::is_closed() const {
  return closed_;
}

bool async_pipe_writer::write_packet(const packet &p) {
  if (writable_) {
    assert(pending_packets_.size() == 0u);
    // Send it immediately
    return true;
  }

  static const size_t kMaxPendingSize = 10 * 1024 * 1024;
  if (total_size_ + p.size() > kMaxPendingSize) {
    return false;
  }

  pending_packets_.push_back(p);
  return false;
}

bool async_pipe_writer::on_write() {
  writable_ = true;

  while (writable_ && packet_) {
    if (written_ < packet_->size) {
      size_t towrite = packet_->size - written_;
      const char *p = reinterpret_cast<const char *>(packet_) + written_;
      size_t n = fwrite(p, 1, towrite, fp_);
      written_ += n;

      if (n < towrite) {
        writable_ = false;
        break;
      }
    }

    if (pending_packets_.size() > 0) {
      // Schedule next packet
      packet_ = pending_packets_.front();
      pending_packets_.pop_front();
      written_ = 0;
    } else {
      packet_ = NULL;
    }
  }

  if (writable_ && !packet_) {
    disable_write_callback();
  } else if (!writable_) {
    assert(packet_ != NULL);
    enable_write_callback();
  }

  fflush(fp_);
  return true;
}

void async_pipe_writer::write_callback(int fd, short events, void *context) {
  async_pipe_writer *writer = reinterpret_cast<async_pipe_writer *>(context);
  writer->on_write();
}

void async_pipe_writer::disable_write_callback() {
  loop_->add_watcher(pipe_fd_, this, NULL, NULL, &error_callback);
}

void async_pipe_writer::enable_write_callback() {
  loop_->add_watcher(pipe_fd_, this, NULL, &write_callback, &error_callback);
}

}
}
