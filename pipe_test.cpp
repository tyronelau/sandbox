#include <poll.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>

using namespace std;

int64_t now_us() {
  timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000llu * 1000 + tv.tv_usec;
}

void reader_test(int fd) {
  static const int kBytes = 10 * 1024 * 1024;

  int64_t n = 0;
  int64_t start = now_us();
  while (true) {
    char buf[32768];
    int r = read(fd, buf, 32768);
    if (r <= 0) {
      cerr << "Error in reading pipe: " << strerror(errno) << endl;
      break;
    }
    n += r;
    if (n > kBytes)  {
      int64_t end = now_us();

      cout << "Received Mbps: " << n / double(end - start) << endl;
      start = end;
      n = 0;
    }
  }
}

void writer_test(int fd) {
  static const int kBytes = 10 * 1024 * 1024;

  uint64_t n = 0;
  int64_t start = now_us();
  while (true) {
    char buf[32768];
    int r = write(fd, buf, 32768);
    if (r <= 0) {
      cerr << "Error in reading pipe: " << strerror(errno) << endl;
      break;
    }

    n += r;
    if (n > kBytes)  {
      int64_t end = now_us();

      cout << "Sent Mbps: " << n / double(end - start) << endl;
      start = end;
      n = 0;
    }
  }
}

int main() {
  using namespace std;

  int fds[2];
  if (-1 == pipe(fds)) {
    cerr << "Error in creating pipes: " << strerror(errno) << endl;
    return -1;
  }

  pid_t p = fork();
  if (p < 0) {
    cerr << "Error in fork: " << strerror(errno) << endl;
  }

  if (p > 0) {
    writer_test(fds[1]);
  } else {
    reader_test(fds[0]);
  }

  return 0;
}
