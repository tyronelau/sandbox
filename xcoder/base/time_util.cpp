#include "base/time_util.h"

#include <time.h>

namespace agora {
namespace base {

int64_t now_ms() {
  timespec spec = {0, 0};
  clock_gettime(CLOCK_MONOTONIC, &spec);
  return spec.tv_sec * 1000 + (spec.tv_nsec / 1000 / 1000);
}

}
}
