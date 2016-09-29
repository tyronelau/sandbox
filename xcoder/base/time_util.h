#pragma once

#include <sys/time.h>
#include <cstdint>

namespace agora {
namespace base {

inline uint64_t now_ms() {
  timeval t = {0, 0};
  ::gettimeofday(&t, NULL);
  return t.tv_sec * 1000llu + t.tv_usec / 1000;
}

}
}

