#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <cassert>
#include <cerrno>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "protocol.h"

using namespace std;

void print_usage(int argc, char *argv[]) {
  cerr << "Usage: " << argv[0] << " port" << endl;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    print_usage(argc, argv);
    return -1;
  }

  uint16_t port = htons(atoi(argv[1]));

  int h = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = port;

  if (0 != bind(h, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr))) {
    cout << "bind failed: " << strerror(errno);
    close(h);
    return -1;
  }

  fcntl(h, F_SETFL, O_NONBLOCK);

  // int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
  // if (timer_fd < 0) {
  //   cerr << "Failed to create a timer fd!\n";
  //   return -1;
  // }

  int nr_events = 0;
  pollfd fds[] = {{h, POLLIN, 0}};
  sockaddr_in remote;

  enum {MAX_PACKET_SIZE = 2000};
  union {
    rtp_header header;
    char buf[MAX_PACKET_SIZE];
  };

  int64_t prev_recv_ts = -1;
  int64_t prev_send_ts;
  int32_t prev_size;
  double slope = 0.0;
  int32_t cnt = 0;

  while ((nr_events = poll(fds, 1, 1000)) >= 0) {
    if (nr_events == 0)
      continue;

    socklen_t sock_len;
    ssize_t len = recvfrom(h, buf, MAX_PACKET_SIZE, 0,
        reinterpret_cast<sockaddr *>(&remote), &sock_len);
    if (len <= 0)
      continue;

    assert(len == header.length);

    timeval t;
    struct timezone tz;
    gettimeofday(&t, &tz);
    int64_t now = t.tv_sec * 1000ll * 1000ll + t.tv_usec;
    if (prev_recv_ts != -1) {
      int64_t recv_gap = now - prev_recv_ts;
      int64_t send_gap = header.sample_ts - prev_send_ts;
      int32_t dL = header.length - prev_size;
      if (dL > 0 && abs(recv_gap -send_gap) > 1) {
        cnt++;
        double capacity = dL / double(recv_gap - send_gap);
        slope = (slope * cnt + 1.0 / capacity) / (cnt + 1);
        if (cnt == 1000) {
          printf("slope: %f\n", slope);
          cnt = 0;
          slope = 0.0;
        }
        // average = (average * cnt + capacity) / (cnt + 1);
        // printf("%" PRId64 " %d %d %f\n", now, int(recv_gap - send_gap), int(dL), slope);
      }
    }
    prev_recv_ts = now;
    prev_send_ts = header.sample_ts;
    prev_size = header.length;
  }

  return 0;
}

