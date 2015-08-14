#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <malloc.h>
#include <cassert>
#include <cerrno>
#include <cstdlib>
// #include <iostream>
#include <vector>

#include "interface.h"

// std::vector<int> a(100);

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    // cout << "usage: "<< argv[0] << " ip port" << endl;
    return -1;
  }

  void *p = malloc(120);
  void *q = realloc(p, 200);
  void *r = realloc(q, 50);
  void *ss = realloc(r, 0);

  posix_memalign(&p, 4096, 4096);
  memalign(4096, 8192);
  valloc(8192);

  for (int i = 0; i < 100; ++i) {
    base *p = create_instance();
    p->foo();
    p->bar();
    if (i % 2 == 0)
      delete p;
  }

  fprintf(stdout, "Ready to start...\n");
  while (true) {
    sleep(1);
  }

  // void *p = malloc(120);
  // cout << "allocated: " << p << endl;

  // for (int i = 0; i < 10; ++i) {
  //   size_t n = 0;
  //   if (i % 3 == 0) {
  //     n = 120;
  //   } else {
  //     n = 200;
  //   }
  //   void *q = malloc(n);
  // }
  // free(p);

  // std::vector<int> *pv = new std::vector<int>(100);
  // void *qq = malloc(200);
  // void *rr = malloc(120);

  // p = realloc(p, 300);
  // cout << "realloced: " << p << endl;
  // free(p);

  in_addr_t ip = inet_addr(argv[1]);
  unsigned short port = htons(atoi(argv[2]));

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (0 != fcntl(fd, F_SETFL, O_NONBLOCK)) {
    fprintf(stderr, "Failed to set the socket to non-blocking mode\n");
    return -1;
  }

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = port;
  addr.sin_addr.s_addr = ip;

  int ret = connect(fd, (sockaddr*)&addr, sizeof(addr));
  if (ret == 0) {
    fprintf(stdout, "connect success\n");
    return 0;
  }

  int error = errno;

  switch (error) {
  case ECONNREFUSED:
    fprintf(stderr, "Connection refused\n");
    return -1;
  case EINPROGRESS:
    fprintf(stdout, "Connecting...\n");
    break;
  case EAGAIN:
    fprintf(stderr, "No free port available\n");
    return -1;
  default:
    fprintf(stderr, "Unknown error: %d\n", error);
    return -1;
  }

  while (true) {
    fd_set rds;
    fd_set wds;
    fd_set eds;
    FD_ZERO(&rds);
    FD_ZERO(&wds);
    FD_ZERO(&eds);
    FD_SET(fd, &rds);
    FD_SET(fd, &wds);

    ret = select(fd + 1, &rds, &wds, &eds, NULL);
    if (ret == 0) {
      fprintf(stderr, "Unknown issue\n");
      return -1;
    }

    if (ret > 0) {
      if (FD_ISSET(fd, &rds))
        fprintf(stdout, "Readable\n");
      if (FD_ISSET(fd, &wds)) {
        fprintf(stdout, "Writable\n");
        long opt = 0;
        socklen_t len = sizeof(opt);
        int s = getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<void *>(&opt), &len);
        assert(s == 0);
        fprintf(stdout, "len: %d\n", int(len));
        if (opt) {
          fprintf(stdout, "connection error, code: %ld\n", opt);
          return -1;
        } else {
          fprintf(stdout, "Connection success\n");
          return -1;
        }
      }

      if (FD_ISSET(fd, &eds)) {
        fprintf(stdout, "Error\n");
      }
      break;
    } else {
      fprintf(stdout, "Unknown Return: %d\n", ret);
    }
  }

  return 0;
}

