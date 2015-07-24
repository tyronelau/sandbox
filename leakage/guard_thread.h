#pragma once
#include <pthread.h>
#include <atomic>

class guard_thread {
 public:
  static bool create_and_run();
  static bool terminate();
 private:
  guard_thread();
  ~guard_thread();
 private:
  static void* dump_thread(void *);
  static void sigusr_handler(int signo);

  static std::atomic<bool> g_dump_memory;
  static std::atomic<bool> g_thread_inited;
  static pthread_t g_guard_thread_;
};

