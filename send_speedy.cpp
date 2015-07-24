#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <gperftools/profiler.h>

using namespace std;

int main() {
  int h = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(8888);

  if (0 != bind(h, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr))) {
    cout << "bind failed: " << strerror(errno);
    close(h);
    return -1;
  }

  fcntl(h, F_SETFL, O_NONBLOCK);

  int len = 2 * 1024 * 1024;
  setsockopt(h, SOL_SOCKET, SO_SNDBUF, &len, sizeof(len));
  ProfilerStart("./a.prof");
  int i = 0;
  while (i++ < 10) {
    char buf[64];

    sockaddr_in remote;
    remote.sin_port = htons(6666);
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = inet_addr("221.228.202.122");

    int failed = 0;
    int success = 0;
    
    timeval t;
    struct timezone tz;
    gettimeofday(&t, &tz);
    double start = t.tv_sec * 1e6 + t.tv_usec;

    for (int i = 0; i < 1000000; ++i) {
      ssize_t len = sendto(h, buf, 64, 0, reinterpret_cast<sockaddr *>(&remote),
          sizeof(remote));
      if (len == 64)
        ++success;
      else if (len == -1) {
        int error = errno;
        if (error == EAGAIN || error == EWOULDBLOCK) {
          fd_set fds;
          FD_ZERO(&fds);
          FD_SET(h, &fds);
          int ret = select(h + 1, NULL, &fds, NULL, NULL);
          if (ret == 1)
            continue;
          else
            break;
        }

      // if (len != 64)
      //   ++failed;
      // else
      //   ++success;
    } else
      break;
    }

    gettimeofday(&t, &tz);
    double end = t.tv_sec * 1e6 + t.tv_usec;
    cout << "Speed " << success / (end - start) * 1e6 << endl;
    cout << "Failed " << failed / (end - start) * 1e6 << endl;
  }

  ProfilerStop();
  return 0;
}

