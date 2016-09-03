#pragma once

#include <poll.h>
#include <unordered_map>
#include <vector>

namespace agora {
namespace base {

typedef void (*event_callback_t)(int fd, void *context);

// A stupid but simple async event driver
class event_loop {
  struct event {
    int fd;
    void *context;

    event_callback_t read_callback;
    event_callback_t write_callback;
    event_callback_t error_callback;
  };
 public:
  event_loop();
  ~event_loop();

  int run();
  int stop();

  int add_watcher(int fd, void *context, event_callback_t read_handler,
      event_callback_t write_handler, event_callback_t error_handler);

  int remove_watcher(int fd, void *context, event_callback_t read_handler,
      event_callback_t write_handler, event_callback_t error_handler);
 private:
  void prepare_poll_events();

  void on_read_event(int fd, int events);
  void on_write_event(int fd, int events);
  void on_error_event(int fd, int events);
 private:
  bool stop_;
  std::vector<pollfd> pollfds_;
  std::unordered_map<int, event> events_;
};

}
}
