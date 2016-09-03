#pragma once

namespace agora {
namespace base {

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
  async_pipe_reader(int fd, pipe_read_listener *listener);
  ~async_pipe_reader();

  bool detach();
  bool close();
  bool is_closed() const;
 private:
  static void read_callback(int fd, short events, void *context);
 private:
  int pipe_fd_;
  pipe_read_listener *listener_;
};

class async_pipe_writer {
 public:
  async_pipe_writer(int fd, pipe_write_listener *listener);
  ~async_pipe_writer();

  bool detach();
  bool close();

  bool is_closed() const;
  bool write_packet(const packet &p);
 private:
  static void write_callback(int fd, short events, void *context);
 private:
  int pipe_fd_;
  pipe_write_listener *listener_;
};
}

