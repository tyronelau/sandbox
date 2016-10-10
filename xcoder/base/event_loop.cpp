#include "base/event_loop.h"

#include <cassert>
#include <cerrno>
#include <cstring>

#include "base/safe_log.h"
#include "base/time_util.h"

namespace agora {
namespace base {

struct timer_event {
  bool repeated;
  int32_t interval_ms;
  int64_t next_ms;
  void *context;
  event_callback_t callback;
  event_loop *loop;
};

inline bool event_loop::timer_comparator::operator()(const timer_event *a,
    const timer_event *b) const {
  return a->next_ms < b->next_ms;
}

event_loop::event_loop() {
  stop_ = false;
}

event_loop::~event_loop() {
}

int event_loop::run() {
  static const int kWaitMs = 500;

  while (!stop_) {
    // SAFE_LOG(INFO) << "Ready to setup poll";
    prepare_poll_events();

    int n = poll(&pollfds_[0], pollfds_.size(), kWaitMs);
    if (n == -1) {
      SAFE_LOG(ERROR) << "Failed to call poll: " << strerror(errno);
      return -1;
    }

    if (stop_)
      break;

    // SAFE_LOG(INFO) << "Checking events: ";

    for (unsigned int i = 0; i < pollfds_.size(); ++i) {
      pollfd &e = pollfds_[i];
      assert(events_.find(e.fd) != events_.end());

      if (e.revents & (POLLHUP | POLLRDHUP | POLLERR)) {
        on_error_event(e.fd, e.revents);
      }

      if (e.revents & POLLIN) {
        on_read_event(e.fd, e.revents);
      }

      if (e.revents & POLLOUT) {
        on_write_event(e.fd, e.revents);
      }

      if (stop_)
        return 0;
    }

    // SAFE_LOG(INFO) << "Checking timers";

    typedef std::set<timer_event *, timer_comparator> timer_map_t;
    typedef timer_map_t::iterator timer_iter_t;
    timer_iter_t it = timers_.begin();
    while (it != timers_.end()) {
      timer_event *timer = *it;
      if (timer->next_ms > base::now_ms())
        break;

      timers_.erase(it);

      (*timer->callback)(-1, timer->context);
      if (stop_)
        return 0;

      timer->next_ms += timer->interval_ms;
      timers_.insert(timer);
      it = timers_.begin();
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
  (void)context;
  (void)read_handler;
  (void)write_handler;
  (void)error_handler;

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
      p.events |= POLLOUT;
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
  (void)events;

  // SAFE_LOG(INFO) << "On Read: " << fd;
  auto it = events_.find(fd);
  if (it == events_.end())
    return;

  const event &e = it->second;
  if (e.read_callback)
    (*e.read_callback)(fd, e.context);
}

void event_loop::on_write_event(int fd, int events) {
  (void)events;

  auto it = events_.find(fd);
  if (it == events_.end())
    return;

  const event &e = it->second;
  if (e.write_callback)
    (*e.write_callback)(fd, e.context);
}

void event_loop::on_error_event(int fd, int events) {
  (void)events;

  auto it = events_.find(fd);
  if (it == events_.end())
    return;

  SAFE_LOG(INFO) << "Error " << fd << ", " << events;

  const event &e = it->second;
  if (e.error_callback)
    (*e.error_callback)(fd, e.context);
}

timer_event* event_loop::add_timer(int32_t interval, event_callback_t callback,
    void *context) {
  timer_event *e = new timer_event();
  e->loop = this;
  e->repeated = true;
  e->interval_ms = interval;
  e->next_ms = base::now_ms() + interval;
  e->context = context;
  e->callback = callback;

  timers_.insert(e);
  return e;
}

bool event_loop::remove_timer(timer_event *e) {
  if (e->loop != this)
    return false;

  delete e;
  return timers_.erase(e) > 0;
}

}
}
