#include "base/event_loop.h"

#include <cassert>
#include <cerrno>
#include <cstring>

#include "base/safe_log.h"

namespace agora {
namespace base {

event_loop::event_loop() {
}

event_loop::~event_loop() {
}

int event_loop::run() {
  static const int kWaitMs = 500;

  while (!stop_) {
    prepare_poll_events();

    int n = poll(&pollfds_[0], pollfds_.size(), kWaitMs);
    if (n == -1) {
      SAFE_LOG(ERROR) << "Failed to call poll: " << strerror(errno);
      return -1;
    }

    if (n == 0 && stop_)
      break;

    for (unsigned int i = 0; i < pollfds_.size(); ++i) {
      pollfd &e = pollfds_[i];
      assert(events_.find(e.fd) != events_.end());

      if (e.revents & (POLLHUP | POLLRDHUP | POLLERR)) {
        on_error_event(fd, e.revents);
      }

      if (e.revents & POLLIN) {
        on_read_event(fd, e.revents);
      }

      if (e.revents & POLLOUT) {
        on_write_event(fd, e.revents);
      }
    }
  }

  return 0;
}

int event_loop::add_watcher(int fd, void *context,
    event_callback_t read_handler, event_callback_t write_handler,
    event_callback_t error_handler) {
  event e = {fd, context, read_handler, write_handler, error_handler};
  events_[fd] = e;

  return 0;
}

int event_loop::remove_watcher(int fd, void *context,
    event_callback_t read_handler, event_callback_t write_handler,
    event_callback_t error_handler) {
  auto it = events_.find(fd);
  if (it == events_.end())
    return -1;

  events_.erase(it);
  return 0;
}

void event_loop::prepare_poll_events() {
  pollfds_.resize(events_.size());

  unsigned int i = 0;
  typedef std::unordered_map<int, event>::const_iterator iter_t;
  for (iter_t it = events_.begin(); it != events_.end(); ++it) {
    const event &e = it->second;
    pollfd &p = pollfds_[i++];
    p.fd = e.fd;

    p.events = 0;
    p.revents = 0;

    if (e.read_callback) {
      p.events |= POLLIN;
    }

    if (e.write_callback) {
      p.events | = POLLOUT;
    }

    if (e.error_callback) {
      p.events |= POLLRDHUP | POLLERR;
    }
  }
}

int event_loop::stop() {
  stop_ = true;
  return 0;
}

void event_loop::on_read_event(int fd, int events) {
  auto it = events_.find(fd);
  if (it == events_.end())
    return;

  const event &e = it->second;
  if (e.read_callback)
    (*e.read_callback)(fd, e.context);
}

void event_loop::on_write_event(int fd, int events) {
  auto it = events_.find(fd);
  if (it == events_.end())
    return;

  const event &e = it->second;
  if (e.write_callback)
    (*e.write_callback)(fd, e.context);
}

void event_loop::on_error_event(int fd, int events) {
  auto it = events_.find(fd);
  if (it == events_.end())
    return;

  const event &e = it->second;
  if (e.error_callback)
    (*e.error_callback)(fd, e.context);
}

}
}