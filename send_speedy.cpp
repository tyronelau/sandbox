#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <cerrno>
#include <cstring>
#include <iostream>

using namespace std;

int main() {
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

  int len = 512 * 1024;
  setsockopt(h, SOL_SOCKET, SO_SNDBUF, &len, sizeof(len));
  while (true) {
    char buf[64];

    sockaddr_in remote;
    remote.sin_port = htons(6000);
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = inet_addr("192.168.99.254");

    int failed = 0;
    int success = 0;
    
    timeval t;
    struct timezone tz;
    gettimeofday(&t, &tz);
    double start = t.tv_sec * 1e6 + t.tv_usec;

    for (int i = 0; i < 1000000; ++i) {
      ssize_t len = sendto(h, buf, 64, 0, reinterpret_cast<sockaddr *>(&remote),
          sizeof(remote));
      if (len != 64)
        ++failed;
      else
        ++success;
    }

    gettimeofday(&t, &tz);
    double end = t.tv_sec * 1e6 + t.tv_usec;
    cout << "Speed " << success / (end - start) * 1e6 << endl;
  }

  return 0;
}

