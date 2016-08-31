#pragma once

struct event_base;

namespace agora {
class packet;

struct pipe_read_listener {
  virtual bool on_receive_packet(const packet &p) = 0;
  virtual bool on_error(short events) = 0;
};

struct pipe_write_listener {
  virtual bool on_ready_write(const packet &p) = 0;
  virtual bool on_error(short events) = 0;
};

class pipe_async_reader {
 public:
  pipe_async_reader(int fd, event_base *base, pipe_read_listener *listener);
  ~pipe_async_reader();

  bool detach();
  bool close();
  bool is_closed() const;

  // bool read_packet(packet *p);
 private:
  static void read_callback(int fd, short events, void *context);
 private:
  int pipe_fd_;
  event_base *base_;
  pipe_read_listener *listener_;
};

class pipe_async_writer {
 public:
  pipe_async_writer(int fd, event_base *base, pipe_write_listener *listener);
  ~pipe_async_writer();

  bool detach();
  bool close();

  bool is_closed() const;
  bool write_packet(const packet &p);
 private:
  static void write_callback(int fd, short events, void *context);
 private:
  int pipe_fd_;
  event_base *base_;
  pipe_write_listener *listener_;
};
}

