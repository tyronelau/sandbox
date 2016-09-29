#pragma once
#include <cassert>
#include <vector>
#include <iterator>
#include <utility>

#include "base/log.h"
#include "base/async_queue.h"

namespace agora {
namespace base {

class event_loop;

template <typename Elem>
struct async_event_handler {
  virtual void on_event(Elem e) = 0;
};

template <typename Elem>
class event_queue {
 public:
  event_queue(event_loop *base, async_event_handler<Elem> *handler,
      size_t max_size);

  ~event_queue();

  template <typename elem> bool push(elem &&e);

  // Take all elements away in a non-blocked way
  // Return: the count of taken elements.
  template <typename output_iterator> size_t take_all(output_iterator out);

  bool empty() const;
  bool closed() const;
  void close();
 private:
  static void read_callback(int fd, void *context);
  static void error_callback(int fd, void *context);

  void handle_events();
  void handle_error();
 private:
  event_loop *base_;
  async_event_handler<Elem> *handler_;
  bool closed_;
  // event *event_;

  AsyncQueue<Elem> queue_;
};

template <typename Elem>
event_queue<Elem>::event_queue(event_loop *base,
    async_event_handler<Elem> *handler, size_t size)
    : base_(base), handler_(handler), queue_(size) {
  LOG(INFO, "event base: %p", base_);
  closed_ = false;

  int fd = queue_.GetEventFD();
  if (base_->add_watcher(fd, this, read_callback, NULL, error_callback) != 0) {
    LOG(FATAL, "Failed to add watcher for event queue: %m");
    closed_ = true;
  }
}

template <typename Elem>
event_queue<Elem>::~event_queue() {
  close();
}

template <typename Elem>
void event_queue<Elem>::close() {
  int fd = queue_.GetEventFD();
  base_->remove_watcher(fd, this, NULL, NULL, NULL);

  queue_.Close();
  closed_ = true;
}

template <typename Elem>
bool event_queue<Elem>::closed() const {
  return closed_;
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
void event_queue<Elem>::read_callback(int fd, void *context) {
  (void)fd;
  (void)context;

  event_queue<Elem> *p = reinterpret_cast<event_queue<Elem> *>(context);
  assert(fd == p->queue_.GetEventFD());
  p->handle_events();
}

template <typename Elem>
void event_queue<Elem>::handle_events() {
  std::vector<Elem> events;
  queue_.TakeAll(std::back_inserter(events));
  for (auto f = events.begin(); f != events.end(); ++f) {
    handler_->on_event(std::move(*f));
  }
}

template <typename Elem>
void event_queue<Elem>::error_callback(int fd, void *context) {
  (void)fd;

  event_queue<Elem> *p = reinterpret_cast<event_queue<Elem> *>(context);
  assert(fd == p->queue_.GetEventFD());
  p->handle_error();
}

template <typename Elem>
void event_queue<Elem>::handle_error() {
  // FIXME
}

template <typename Elem>
template <typename elem>
bool event_queue<Elem>::push(elem &&e) {
  return queue_.Push(std::forward<elem>(e));
}

}
}
