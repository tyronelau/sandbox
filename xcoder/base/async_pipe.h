#pragma once

#include <cstdint>
#include <cstdio>
#include <queue>
#include <vector>

#include "base/buffered_pipe.h"

#include "protocol/ipc_protocol.h"

namespace agora {
namespace base {

class async_pipe_reader;
class async_pipe_writer;
class event_loop;
class packet;
class unpacker;

struct pipe_read_listener {
  virtual bool on_receive_packet(async_pipe_reader *reader, unpacker &pkr,
      uint16_t uri) = 0;

  virtual bool on_error(async_pipe_reader *reader, int events) = 0;
};

struct pipe_write_listener {
  // virtual bool on_ready_write(async_pipe_writer *writer) = 0;
  virtual bool on_error(async_pipe_writer *writer, int events) = 0;
};

class async_pipe_reader {
 public:
  async_pipe_reader(event_loop *loop, int fd, pipe_read_listener *listener);
  ~async_pipe_reader();

  // bool detach();
  bool close();
  bool is_closed() const;
  int get_pipe_fd() const;
 private:
  void setup_callback();
  void remove_callback();

  void on_read();
  void on_error(int events);
  void destroy();

  static void read_callback(int fd, void *context);
  static void error_callback(int fd, void *context, int events);
 private:
  event_loop *loop_;

  int pipe_fd_;
  FILE *fp_;
  // FILE *raw_;

  uint32_t packet_size_;
  uint32_t readed_;
  char *buffer_;

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
  int get_pipe_fd() const;

  bool write_packet(const packet &p);
 private:
  void enable_write_callback();
  void disable_write_callback();
  void remove_callback();

  bool on_write();
  void on_error(int events);
  void destroy();

  static void write_callback(int fd, void *context);
  static void error_callback(int fd, void *context, int events);
 private:
  event_loop *loop_;

  int pipe_fd_;
  FILE *fp_;
  // FILE *raw_;

  size_t written_;
  std::vector<char> buffer_;
  pipe_write_listener *listener_;

  bool closed_;
  bool processing_;
  bool writable_;

  size_t total_size_;
  std::queue<std::vector<char> > pending_packets_;
};

}
}

