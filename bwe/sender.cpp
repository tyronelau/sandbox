#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

#include <cassert>
#include <cerrno>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "protocol.h"

using namespace std;

void print_usage(int argc, char *argv[]) {
  cerr << "Usage: " << argv[0] << "ip port" << endl;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    print_usage(argc, argv);
    return -1;
  }

  uint32_t ip;
  if ((ip = inet_addr(argv[1])) == INADDR_NONE) {
    print_usage(argc, argv);
    return -1;
  }

  uint16_t port = htons(atoi(argv[2]));

  int h = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(8000);

  if (0 != bind(h, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr))) {
    cout << "bind failed: " << strerror(errno);
    close(h);
    return -1;
  }

  fcntl(h, F_SETFL, O_NONBLOCK);

  int len = 2 * 1024 * 1024;
  setsockopt(h, SOL_SOCKET, SO_SNDBUF, &len, sizeof(len));

  int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
  if (timer_fd < 0) {
    cerr << "Failed to create a timer fd!\n";
    return -1;
  }

  int nr_events = 0;
  pollfd fds[] = {{timer_fd, POLLIN, 0}};

  sockaddr_in remote;
  remote.sin_port = port;
  remote.sin_family = AF_INET;
  remote.sin_addr.s_addr = ip;

  union {
    rtp_header header;
    char buf[2000];
  };

  int i = 0;

  while ((nr_events = poll(fds, 1, 100)) >= 0) {
    i = (i + 1) % 9;

    int cnt = (i + 1) * (i + 2) * 16;
    timeval t;
    struct timezone tz;
    gettimeofday(&t, &tz);
    int64_t start = t.tv_sec * 1000ll * 1000ll + t.tv_usec;

    header.length = cnt;
    header.sample_ts = start;

    ssize_t len = sendto(h, buf, cnt, 0, reinterpret_cast<sockaddr *>(&remote),
        sizeof(remote));
  }

  return 0;
}

