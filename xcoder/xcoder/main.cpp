#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cinttypes>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "base/log.h"
#include "base/opt_parser.h"
#include "xcoder/event_handler.h"

using namespace std;
using namespace agora;
using namespace agora::base;
using namespace agora::recording;

int main(int argc, char *argv[]) {
  uint32_t uid = 0;
  string key;
  string name;
  bool dual = false;
  int read_fd = -1;
  int write_fd = -1;

  LOG(INFO, "video recorder, based on version " GIT_DESC);

  opt_parser parser;
  parser.add_long_opt("uid", &uid);
  parser.add_long_opt("key", &key);
  parser.add_long_opt("name", &name);
  parser.add_long_opt("dual", &dual);
  parser.add_long_opt("write", &write_fd);
  parser.add_long_opt("read", &read_fd);

  if (!parser.parse_opts(argc, argv) || key.empty() || name.empty()) {
    std::ostringstream sout;
    parser.print_usage(argv[0], sout);
    LOG(ERROR, "%s", sout.str().c_str());
    return -1;
  }

  if (read_fd <= 0 || write_fd <= 0) {
    std::ostringstream sout;
    parser.print_usage(argv[0], sout);
    LOG(ERROR, "No pipe fd designated! %s", sout.str().c_str());
    return -1;
  }

  LOG(INFO, "uid %" PRIu32 " from vendor %s is joining channel %s",
      uid, key.c_str(), name.c_str());

  event_handler handler(uid, key, name, dual, read_fd, write_fd);
  return handler.run();
}
