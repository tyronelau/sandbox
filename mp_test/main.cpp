#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>

std::atomic<bool> flag;

struct cpu_time_data {
  long user_time;
  long sys_time;
  long prev_ts;
};

/** return the cpu usage of self process in percentage */
double get_self_cpu_usage(cpu_time_data *prev);

double get_self_cpu_usage(cpu_time_data *prev) {
  assert(prev != NULL);
  cpu_time_data &last = *prev;
  tms cur;

  double percent = 0.0;
  clock_t now = times(&cur);
  if (now <= last.prev_ts || cur.tms_stime < last.sys_time ||
      cur.tms_utime < last.user_time){
    //  overflow detection. Just skip this value.
    percent = 0.0;
  } else {
    percent = (cur.tms_stime - last.sys_time) + (cur.tms_utime - last.user_time);
    percent /= static_cast<double>(now - last.prev_ts);
    percent *= 100;
  }

  last.prev_ts = now;
  last.sys_time = cur.tms_stime;
  last.user_time = cur.tms_utime;

  return percent;
}

int thread_func() {
  while (true) {
    for (volatile int i = 0; i < 10000; ++i)
      ;
    flag.store(true);
  }
  return 0;
}

using namespace std;

int main() {
  std::thread a(thread_func);
  std::thread b(thread_func);

  cpu_time_data data = {};
  get_self_cpu_usage(&data);

  while (true) {
    sleep(1);
    cout << "cpu: " << get_self_cpu_usage(&data) << endl;
  }

  a.join();
  b.join();
  return 0;
}

