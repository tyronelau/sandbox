#include "guard_thread.h"
#include <unistd.h>
#include <csignal>

void dump_memory_snapshot();

std::atomic<bool> guard_thread::g_dump_memory(false);
std::atomic<bool> guard_thread::g_thread_inited(false);
pthread_t guard_thread::g_guard_thread_ = -1;

guard_thread::guard_thread() {
}

guard_thread::~guard_thread() {
}

void guard_thread::sigusr_handler(int signo) {
  (void)signo;
  g_dump_memory = true;
}

bool guard_thread::create_and_run() {
  if (g_thread_inited)
    return true;

  if (0 != pthread_create(&g_guard_thread_, NULL, dump_thread, NULL))
    return false;

  signal(SIGUSR1, sigusr_handler);
  g_thread_inited = true;

  return true;
}

bool guard_thread::terminate() {
  if (!g_thread_inited)
    return true;
  signal(SIGUSR1, SIG_IGN);

  // FIXME(liuyong): destroy the thread
  // pthread_destroy(
  g_thread_inited = false;

  return true;
}

void* guard_thread::dump_thread(void *arg) {
  (void)arg;
  while (true) {
    sleep(1);
    if (g_dump_memory) {
      g_dump_memory = false;
      dump_memory_snapshot();
    }
  }

  return NULL;
}

