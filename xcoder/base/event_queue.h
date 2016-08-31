#pragma once
#include <vector>
#include <iterator>
#include <utility>

#include <event2/event.h>
#include "base/log.h"
#include "base/async_queue.h"

struct event_base;

namespace agora {
namespace base {

template <typename Elem>
struct async_event_handler {
  virtual void on_event(const Elem &e) = 0;
};

template <typename Elem>
class event_queue {
 public:
  event_queue(event_base *base, async_event_handler<Elem> *handler,
      size_t max_size);

  ~event_queue();

  template <typename elem>
  bool push(elem &&e);

  // Take all elements away in a non-blocked way
  // Return: the count of taken elements.
  template <typename output_iterator> size_t take_all(output_iterator out);
  bool empty() const;
  bool closed() const;
  void close();
 private:
  static void read_callback(evutil_socket_t fd, short event, void *contex);
  void handle_events();
 private:
  event_base *base_;
  async_event_handler<Elem> *handler_;
  event *event_;

  AsyncQueue<Elem> queue_;
};

template <typename Elem>
event_queue<Elem>::event_queue(event_base *base,
    async_event_handler<Elem> *handler, size_t size)
    : base_(base), handler_(handler), queue_(size) {
  LOG(INFO, "event base: %p", base_);
  int fd = queue_.GetEventFD();
  event_ = event_new(base_, fd, EV_READ | EV_PERSIST, read_callback, this);
  if (event_add(event_, NULL) != 0) {
    LOG(FATAL, "Failed to add an eventfd into libevent, %m");
    return;
  }
}

template <typename Elem>
event_queue<Elem>::~event_queue() {
  close();
}

template <typename Elem>
void event_queue<Elem>::close() {
  event_del(event_);
  event_free(event_);
  event_ = NULL;

  queue_.Close();
}

template <typename Elem>
bool event_queue<Elem>::closed() const {
  return event_ == NULL;
}

template <typename Elem>
bool event_queue<Elem>::empty() const {
  return queue_.Empty();
}

template <typename Elem>
template <typename output_iterator>
size_t event_queue<Elem>::take_all(output_iterator out) {
  return queue_.TakeAll(out);
}

template <typename Elem>
void event_queue<Elem>::read_callback(evutil_socket_t fd, short event,
    void *context) {
  (void)fd;
  (void)event;

  event_queue<Elem> *p = reinterpret_cast<event_queue<Elem> *>(context);
  assert(fd == p->queue_.GetEventFD());
  p->handle_events();
}

template <typename Elem>
void event_queue<Elem>::handle_events() {
  std::vector<Elem> events;
  queue_.TakeAll(std::back_inserter(events));
  for (const Elem &e: events) {
    handler_->on_event(e);
  }
}

template <typename Elem>
template <typename elem>
bool event_queue<Elem>::push(elem &&e) {
  return queue_.Push(std::forward<elem>(e));
}

}
}
