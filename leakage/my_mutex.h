#pragma once

#include <pthread.h>

class my_mutex {
 public:
  my_mutex();
  ~my_mutex();

  void lock();
  void unlock();
 private:
  my_mutex(const my_mutex&) = delete;
  my_mutex(my_mutex&&) = delete;

  my_mutex& operator=(const my_mutex &) = delete;
  my_mutex& operator=(my_mutex &&) = delete;
 private:
  pthread_mutex_t mutex_;
};

inline my_mutex::my_mutex() {
  mutex_ = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_init(&mutex_, NULL);
}

inline my_mutex::~my_mutex() {
  pthread_mutex_destroy(&mutex_);
}

inline void my_mutex::lock() {
  pthread_mutex_lock(&mutex_);
}

inline void my_mutex::unlock() {
  pthread_mutex_unlock(&mutex_);
}

template <typename mutex_type>
class my_lock_guard {
 public:
  my_lock_guard(mutex_type &m) :mutex_(m) { mutex_.lock(); }
  ~my_lock_guard() { mutex_.unlock(); }
 private:
  mutex_type &mutex_;
};

