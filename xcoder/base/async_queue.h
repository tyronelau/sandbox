#pragma once

#include <unistd.h>
#include <sys/eventfd.h>

#include <atomic>
#include <cstring>
#include <mutex>
#include <queue>
#include <utility>

// #include <glog/logging.h>
#include "base/log.h"

#ifndef DISALLOW_COPY_ASSIGN
#define DISALLOW_COPY_ASSIGN(class_name) \
  private: \
    class_name(const class_name&) /*=delete*/; \
    class_name& operator=(const class_name&) /*=delete*/;
#endif

/** Using this queue like this
 *  Thread 1
 *  while (true) {
 *    q.Push(new_event);
 *  }
 *
 *  Thread 2
 *  while (select/pselect/epoll to the event_fd,
 *     find if an element is available) {
 *    q.TakeAll();
 *    HandleAllElements();
 *  }
 * Note: this queue is non-blocked, that means if no element is available
 *  TakeAll will return immediately.
 */
template <typename Elem>
class AsyncQueue {
  DISALLOW_COPY_ASSIGN(AsyncQueue)
 public:
  AsyncQueue(size_t max_size=(std::numeric_limits<size_t>::max)());
  ~AsyncQueue();

  // push an element into the queue
  // Preconditions: !Closed()
  // PostConditions:
  // event_fd will be triggered
  // Return value:
  //   true means success, otherwise,
  //   false
  bool Push(Elem e);

  // Take all elements away in a non-blocked way
  // Return: the count of taken elements.
  template <typename OutputIterator>
  size_t TakeAll(OutputIterator out);
  // size_t TakeAll(std::vector<Elem> *out);

  size_t Size() const;
  bool Empty() const;
  void Close();
  bool Closed() const;
  int GetEventFD() const;
 private:
  typedef std::lock_guard<std::mutex> AutoLock;
 private:
  int event_fd_;
  std::atomic<bool> closed_;
  mutable std::mutex lock_;
  size_t max_size_;
  std::queue<Elem> q_;
};

template <typename Elem>
inline AsyncQueue<Elem>::AsyncQueue(size_t size) {
  max_size_ = size;

  closed_ = false;
  event_fd_ = eventfd(0, EFD_NONBLOCK);
  // PLOG_IF(FATAL, event_fd_ == -1) << " Failed to create an event fd.";
  if (event_fd_ == -1)
    log(agora::base::FATAL_LOG, " Failed to create an event fd: %s", strerror(errno));
}

template <typename Elem>
inline AsyncQueue<Elem>::~AsyncQueue() {
  close(event_fd_);
}

template <typename Elem>
bool AsyncQueue<Elem>::Push(Elem e) {
  AutoLock guard(lock_);
  if (closed_) {
    LOG(WARN, "You should not push an element after this queue was closed!");
    return false;
  }

  if (q_.size() >= max_size_) {
    LOG(ERROR, "Too many packets in queue: %d", static_cast<int>(q_.size()));
    return false;
  }

  uint64_t incr = 1;
  ssize_t kLen = sizeof(uint64_t);
  if (write(event_fd_, &incr, kLen) != kLen) {
    // LOG(WARNING) << " Failed to write an increment into the event fd";
    LOG(WARN, " Failed to write an increment into the event fd");
    return false;
  }

  q_.push(std::move(e));
  return true;
}

template <typename Elem>
template <typename OutputIterator>
size_t AsyncQueue<Elem>::TakeAll(OutputIterator out) {
  AutoLock guard(lock_);
  if (closed_) {
    // LOG(WARNING) << " This queue has been closed. ";
    LOG(WARN, " This queue has been closed. ");
    return 0;
  }

  int64_t len = sizeof(uint64_t);
  int64_t cnt = 0;
  if (read(event_fd_, &cnt, len) != len) {
    // PLOG(FATAL) << " Failed to read the event fd";
    LOG(FATAL, " Failed to read the event fd");
    return 0;
  }

  len = q_.size();
  if (len != cnt) {
    // LOG_IF(FATAL, len != cnt) << " Inconsistent data: " << len << ", " << cnt;
    LOG(FATAL, " Inconsistent data: %d, %d", len, cnt);
  }

  // std::copy(q_.begin(), q_.end(), out);
  while (!q_.empty()) {
    const Elem &e = q_.front();
    *out++ = e;
    q_.pop();
  }

  return len;
}

template <typename Elem>
inline size_t AsyncQueue<Elem>::Size() const {
  AutoLock guard(lock_);
  return q_.size();
}

template <typename Elem>
inline bool AsyncQueue<Elem>::Empty() const {
  return Size() == 0;
}

template <typename Elem>
inline void AsyncQueue<Elem>::Close() {
  if (Size() != 0) {
    LOG(WARN, " You should close this queue after"
    " taking all the elements!");
  }
  // LOG_IF(WARNING, Size() != 0) << " You should close this queue after"
  //   " taking all the elements!";
  closed_ = true;
}

template <typename Elem>
inline bool AsyncQueue<Elem>::Closed() const {
  return closed_;
}

template <typename Elem>
inline int AsyncQueue<Elem>::GetEventFD() const {
  return event_fd_;
}

