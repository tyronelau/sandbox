#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cout << "usage: "<< argv[0] << " ip port" << endl;
    return -1;
  }

  in_addr_t ip = inet_addr(argv[1]);
  unsigned short port = htons(atoi(argv[2]));

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (0 != fcntl(fd, F_SETFL, O_NONBLOCK)) {
    cerr << "Failed to set the socket to non-blocking mode" << endl;
    return -1;
  }

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = port;
  addr.sin_addr.s_addr = ip;

  int ret = connect(fd, (sockaddr*)&addr, sizeof(addr));
  if (ret == 0) {
    cout << "connect success" << endl;
    return 0;
  }

  int error = errno;

  switch (error) {
  case ECONNREFUSED:
    cerr << "Connection refused" << endl;
    return -1;
  case EINPROGRESS:
    cout << "Connecting.." << endl;
    break;
  case EAGAIN:
    cerr << "No free port available" << endl;
    return -1;
  default:
    cerr << "Unknown error: " << error << endl;
    return -1;
  }

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
    cerr << "unknown issue: " << endl;
    return -1;
  }

  if (ret > 0) {
    if (FD_ISSET(fd, &rds))
      cout << "readable\n";
    if (FD_ISSET(fd, &wds)) {
      cout << "writable\n";
      long opt = 0;
      socklen_t len = sizeof(opt);
      int s = getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<void *>(&opt), &len);
      assert(s == 0);
      cout << "len: " << len << endl;
      if (opt) {
        cout << "Connection error, error code: " << opt << endl;
        return -1;
      } else {
        cout << "Connection success!" << endl;
        return -1;
      }
    }

    if (FD_ISSET(fd, &eds))
      cout << "error\n";
  } else {
    cout << "unknown ret: " << ret << endl;
  }

  return 0;
}

