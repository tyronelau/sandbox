#include "base/async_pipe.h"

#include <unistd.h>
#include <fcntl.h>

#include <cassert>
#include <cerrno>
#include <cstring>

#include "base/event_loop.h"
#include "base/packer.h"
#include "base/packet.h"
#include "base/safe_log.h"

namespace agora {
namespace base {

async_pipe_reader::async_pipe_reader(event_loop *loop, int fd,
    pipe_read_listener *listener) {
  loop_ = loop;
  pipe_fd_ = fd;

  int flags = fcntl(pipe_fd_, F_GETFL);
  fcntl(pipe_fd_, F_SETFL, flags | O_NONBLOCK);

  fp_ = new buffered_pipe(pipe_fd_, kRead);
  // if ((fp_ = fdopen(pipe_fd_, "rb")) == NULL) {
  //   SAFE_LOG(FATAL) << "Failed to open the pipe to read: " << strerror(errno);
  //   return;
  // }

  // char file_str[256];
  // snprintf(file_str, 256, "%d_%d_r.dat", getpid(), fd);

  // if ((raw_ = fopen(file_str, "wb")) == NULL) {
  //   SAFE_LOG(FATAL) << "Failed to open the pipe to write: " << strerror(errno);
  //   return;
  // }

  readed_ = 0;
  buffer_ = NULL;
  listener_ = listener;
  closed_ = false;
  processing_ = false;

  setup_callback();
}

async_pipe_reader::~async_pipe_reader() {
  remove_callback();

  if (fp_) {
    // fclose(fp_);
    delete fp_;
  }
}

bool async_pipe_reader::close() {
  if (!closed_) {
    loop_->remove_watcher(pipe_fd_, this, NULL, NULL, NULL);
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

int async_pipe_reader::get_pipe_fd() const {
  return pipe_fd_;
}

void async_pipe_reader::read_callback(int fd, void *context) {
  (void)fd;

  async_pipe_reader *reader = reinterpret_cast<async_pipe_reader *>(context);
  assert(fd == reader->get_pipe_fd());
  reader->on_read();
}

void async_pipe_reader::error_callback(int fd, void *context, int events) {
  (void)fd;

  async_pipe_reader *reader = reinterpret_cast<async_pipe_reader *>(context);
  assert(fd == reader->get_pipe_fd());
  reader->on_error(events);
}

void async_pipe_reader::on_read() {
  assert(fp_ != NULL);
  processing_ = true;

  while (!is_closed()) {
    if (buffer_ == NULL) {
      packet_size_ = 0;
      // Read next packet
      // if (fread(&packet_size_, sizeof(packet_size_), 1, fp_) != 1) {
      //   clearerr(fp_);
      //   break;
      // }

      if (fp_->read_buffer(&packet_size_, sizeof(packet_size_), 1) != 1) {
        // clearerr(fp_);
        break;
      }

      // while (fwrite(&packet_size_, sizeof(packet_size_), 1, raw_) != 1) {
      // }

      if (packet_size_ <= 6 || packet_size_ > 64 * 1024 * 104) {
        SAFE_LOG(ERROR) << "Illegal packet size: " << packet_size_;
        if (listener_) {
          listener_->on_error(this, 0x1000);
        }
        break;
      }

      buffer_ = new (std::nothrow)char[packet_size_];
      // SAFE_LOG(INFO) << "Readed: " << packet_size_ << "buffer: " << (void*)buffer_;

      uint32_t *p = reinterpret_cast<uint32_t *>(buffer_);
      *p = packet_size_;

      readed_ = sizeof(packet_size_);
    }

    char *next = buffer_ + readed_;
    size_t toread = packet_size_ - readed_;
    size_t readed = 0;
    if ((readed = fp_->read_buffer(next, 1, toread)) != toread) {
      // clearerr(fp_);

      // while (fwrite(next, readed, 1, raw_) != 1) {
      // }
      // SAFE_LOG(INFO) << "packet : " << (void*)buffer_ << ", p: " << (void*)next
      //   << ", packet size: " << packet_size_ << ", toread: " << toread
      //   << ", read: " << readed;
      readed_ = static_cast<uint32_t>(readed_ + readed);
      break;
    }

      // while (fwrite(next, readed, 1, raw_) != 1) {
      // }

      // SAFE_LOG(INFO) << "packet : " << (void*)buffer_ << ", p: " << (void*)next
      //   << ", packet size: " << packet_size_ << ", toread: " << toread
      //   << ", read: " << readed;

    if (listener_) {
      unpacker unpkr(buffer_, packet_size_, false);
      unpkr.pop_uint32();
      uint16_t uri = unpkr.pop_uint16();
      unpkr.rewind();

      listener_->on_receive_packet(this, unpkr, uri);
    }

    delete [] buffer_;
    buffer_ = NULL;
  }

  if (is_closed()) {
    destroy();
    return;
  }

  processing_ = false;

  // if (feof(fp_) && listener_) {
  //   listener_->on_error(this, 0x2000);
  // }
}

void async_pipe_reader::on_error(int events) {
  if (listener_) {
    listener_->on_error(this, events);
  }
}

void async_pipe_reader::destroy() {
  delete this;
}

async_pipe_writer::async_pipe_writer(event_loop *loop, int fd,
    pipe_write_listener *listener) {
  loop_ = loop;
  pipe_fd_ = fd;

  int flags = fcntl(pipe_fd_, F_GETFL);
  fcntl(pipe_fd_, F_SETFL, flags | O_NONBLOCK);

  fp_ = new buffered_pipe(pipe_fd_, kWrite);

  // if ((fp_ = fdopen(pipe_fd_, "wb")) == NULL) {
  //   SAFE_LOG(FATAL) << "Failed to open the pipe to write: " << strerror(errno);
  //   return;
  // }

  // char file_str[256];
  // snprintf(file_str, 256, "%d_%d_w.dat", getpid(), fd);

  // if ((raw_ = fopen(file_str, "wb")) == NULL) {
  //   SAFE_LOG(FATAL) << "Failed to open the pipe to write: " << strerror(errno);
  //   return;
  // }

  written_ = 0;
  listener_ = listener;

  closed_ = false;
  processing_ = false;
  writable_ = true;
  total_size_ = 0;
}

async_pipe_writer::~async_pipe_writer() {
  if (fp_) {
    delete fp_;
    // fclose(fp_);
  }

  remove_callback();
}

bool async_pipe_writer::close() {
  if (!closed_) {
    loop_->remove_watcher(pipe_fd_, this, NULL, NULL, NULL);
    closed_ = true;
  }

  return closed_;
}

bool async_pipe_writer::is_closed() const {
  return closed_;
}

bool async_pipe_writer::write_packet(const packet &p) {
  packer pkr;
  pkr << p;
  pkr.pack();

  // takes rvalue away
  std::vector<char> buffer = pkr.take_buffer();
  // SAFE_LOG(INFO) << "packet size: " << buffer.size() << ", header: " << *reinterpret_cast<uint32_t *>(&buffer[0]);

  if (writable_) {
    // Send it immediately
    assert(pending_packets_.size() == 0u);
    assert(buffer_.empty());

    buffer_.swap(buffer);
    // written_ = fwrite(&buffer_[0], 1, buffer_.size(), fp_);
    written_ = fp_->write_buffer(&buffer_[0], buffer_.size());
        
    // SAFE_LOG(INFO) << "packet size: " << buffer_.size() << ", header: " << *reinterpret_cast<uint32_t*>(&buffer_[0]);
    // SAFE_LOG(INFO)  << "write: " << (void*)(&buffer_[0]) << ", towrite: " << buffer_.size() << ", written: " << written_;

    // while (fwrite(&buffer_[0], written_, 1, raw_) != 1) {
    // }

    if (written_ < buffer_.size()) {
      writable_ = false;
      if (fp_->eof()) {
        listener_->on_error(this, 0x4000);
        return false;
      }
      // clearerr(fp_);
      enable_write_callback();
      return false;
    }

    written_ = 0;
    buffer_.clear();

    fp_->flush();
    // fflush(fp_);
    return true;
  }

  // static const size_t kMaxPendingSize = 10 * 1024 * 1024;

  SAFE_LOG(INFO) << "Pending: " << pending_packets_.size();
  pending_packets_.push(std::move(buffer));
  return false;
}

bool async_pipe_writer::on_write() {
  writable_ = true;

  while (writable_) {
    if (!buffer_.empty() && written_ < buffer_.size()) {
      size_t towrite = buffer_.size() - written_;
      const char *p = &buffer_[0] + written_;
      // size_t n = fwrite(p, 1, towrite, fp_);
      size_t n = fp_->write_buffer(p, towrite);
    //   if (written_ == 0) {
    //     SAFE_LOG(INFO) << "packet size: " << buffer_.size() << ", header: " << *reinterpret_cast<const uint32_t*>(p);
    //   }

    // fwrite(p, 1, n, raw_);
    // while (fwrite(p, n, 1, raw_) != 1) {
    // }

    // SAFE_LOG(INFO) << "packet: " << (void*)&buffer_[0] << ", write: " << (void*)p << ", towrite: " << towrite << ", written: " << n;
      written_ += n;

      if (n < towrite) {
        writable_ = false;
        // clearerr(fp_);
        break;
      }
    }

    if (pending_packets_.size() > 0) {
      // Schedule next packet
      buffer_ = std::move(pending_packets_.front());
      pending_packets_.pop();
      written_ = 0;
    } else {
      buffer_.clear();
      written_ = 0;
      break;
    }
  }

  if (writable_ && buffer_.empty()) {
    disable_write_callback();
  } else if (!writable_) {
    assert(!buffer_.empty());
    enable_write_callback();
  }

  fp_->flush();
  // fflush(fp_);
  return true;
}

void async_pipe_writer::write_callback(int fd, void *context) {
  (void)fd;

  SAFE_LOG(INFO) << "Writable " << fd << ": " << context;
  async_pipe_writer *writer = reinterpret_cast<async_pipe_writer *>(context);
  writer->on_write();
}

void async_pipe_writer::disable_write_callback() {
  loop_->add_watcher(pipe_fd_, this, NULL, NULL, &error_callback);
}

void async_pipe_writer::enable_write_callback() {
  loop_->add_watcher(pipe_fd_, this, NULL, &write_callback, &error_callback);
}

void async_pipe_writer::error_callback(int fd, void *context, int events) {
  (void)fd;

  async_pipe_writer *writer = reinterpret_cast<async_pipe_writer *>(context);
  assert(fd == writer->get_pipe_fd());
  writer->on_error(events);
}

int async_pipe_writer::get_pipe_fd() const {
  return pipe_fd_;
}

void async_pipe_writer::remove_callback() {
  loop_->remove_watcher(pipe_fd_, this, NULL, NULL, NULL);
}

void async_pipe_writer::on_error(int events) {
  if (listener_) {
    listener_->on_error(this, events);
  }
}

}
}
