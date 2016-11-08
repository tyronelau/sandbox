#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <signal.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <thread>

#include "base/opt_parser.h"

using agora::base::opt_parser;
using agora::base::ipv4;
using std::cerr;
using std::cout;
using std::endl;
using std::thread;

int run_client(const ipv4 &remote, uint16_t port);
int run_server(uint16_t port);

int64_t now_ms() {
  timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec * 1000 + t.tv_usec / 1000;
}

int run_client(const ipv4 &remote, uint16_t port) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = remote.ip;

  int skt = socket(AF_INET, SOCK_STREAM, 0);
  if (::connect(skt, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) < 0) {
    in_addr t;
    t.s_addr = remote.ip;
    cerr << "failed to connect to " << inet_ntoa(t) << ": " << port << endl;
    goto cleanup;
  }

  struct timeval t;
  t.tv_sec = 10;
  t.tv_usec = 0;

  if (-1 == setsockopt(skt, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t))) {
    cerr << "Failed to set socket send time out " << skt << endl;
    goto cleanup;
  }

  if (-1 == setsockopt(skt, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t))) {
    cerr << "Failed to set socket recv time out " << skt  << endl;
    goto cleanup;
  }

  while (true) {
    iovec arr[2];
    char buf1[2048];
    char buf2[2048];

    arr[0].iov_base = const_cast<void *>(reinterpret_cast<const void *>(buf1));
    arr[0].iov_len = 2048;
    arr[1].iov_base = const_cast<void *>(reinterpret_cast<const void *>(buf2));
    arr[1].iov_len = 2048;

    cout << now_ms() << ": Ready to send data" << endl;
    if (::writev(skt, arr, 2) == -1) {
      cout << now_ms() << ": " << errno << strerror(errno) << endl;
      if (errno == ETIMEDOUT || errno == EAGAIN || errno == EPIPE || errno == EBADF) {
        // NOTE(liuyong): Treat it as an unrecoverable error
        cout << now_ms() << " timed out: " << endl;
        break;
      }
    }

    ::sleep(1);
  }
cleanup:
  close(skt);
  return 0;
}

void receive_data(int fd, const sockaddr_in &addr) {
  while (true) {
    char buf[11];
    ssize_t readed = recv(fd, buf, sizeof(buf), 0);
    if (readed == -1 && errno != EINTR) {
      cerr << now_ms() << ", readed: " << readed << ": Unrecoverable error! "
          << strerror(errno) << endl;
      break;
    }
  }

  close(fd);
  cout << "Socket " << fd << ", closed" << endl;
}

int run_server(uint16_t port) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  sockaddr_in client;
  socklen_t addr_len = sizeof(client);
  int fd = -1;

  int skt = socket(AF_INET, SOCK_STREAM, 0);
  if (::bind(skt, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) < 0) {
    cerr << "Failed to bind to the port: " << port << ", " << errno
        << strerror(errno) << endl;
    goto cleanup;
  }

  listen(skt, 5);

  while ((fd = accept(skt, reinterpret_cast<sockaddr *>(&client), &addr_len)) >= 0) {
    cout << "Accepted from " << inet_ntoa(client.sin_addr) << ", " << client.sin_port << endl;
    std::thread t(receive_data, fd, client);
    t.detach();
  }

cleanup:
  close(skt);
  return 0;
}

int main(int argc, char *argv[]) {
  bool is_client = false;
  ipv4 remote;
  uint32_t port = 4444;

  signal(SIGPIPE, SIG_IGN);

  opt_parser parser;
  parser.add_long_opt("client", &is_client);
  parser.add_long_opt("addr", &remote);
  parser.add_long_opt("port", &port);

  if (!parser.parse_opts(argc, argv)) {
    parser.print_usage(argv[0], cerr);
    return -1;
  }

  if (is_client) {
    run_client(remote, port);
  } else {
    run_server(port);
  }

  return 0;
}

