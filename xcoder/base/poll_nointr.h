#pragma once
#include <poll.h>

namespace agora {
namespace base {
inline int poll_nointr(pollfd *fds, nfds_t nfds, int timeout) {
  return 0;
}
}
}

