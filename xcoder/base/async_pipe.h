#pragma once

#include <memory>
#include <queue>

#include "protocol/ipc_protocol.h"

namespace agora {
namespace base {

class event_loop;

struct pipe_read_listener {
  virtual bool on_receive_packet(const packet &p) = 0;
  virtual bool on_error(short events) = 0;
};

struct pipe_write_listener {
  virtual bool on_ready_write(const packet &p) = 0;
  virtual bool on_error(short events) = 0;
};

class async_pipe_reader {
 public:
  async_pipe_reader(event_loop *loop, int fd, pipe_read_listener *listener);
  ~async_pipe_reader();

  // bool detach();
  bool close();
  bool is_closed() const;
 private:
  void setup_callback();
  void remove_callback();

  void on_read();
  void on_error();
  void destroy();

  static void read_callback(int fd, void *context);
  static void error_callback(int fd, void *context);
 private:
  event_loop *loop_;

  int pipe_fd_;
  FILE *fp_;

  uint32_t readed_;
  packet_common_header *packet_;
  pipe_read_listener *listener_;

  bool closed_;
  bool processing_;
};

class async_pipe_writer {
 public:
  async_pipe_writer(event_loop *loop, int fd, pipe_write_listener *listener);
  ~async_pipe_writer();

  // bool detach();
  bool close();

  bool is_closed() const;
  bool write_packet(const packet &p);
 private:
  static void write_callback(int fd, short events, void *context);
 private:
  event_loop *loop_;

  int pipe_fd_;
  FILE *fp_;

  size_t written_;
  packet_common_header *packet_;
  pipe_write_listener *listener_;

  bool closed_;
  bool processing_;
  bool writable_;

  size_t total_size_;
  std::queue<const packet_common_header *> pending_packets_;
};

}
}
