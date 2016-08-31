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

#include "base/atomic.h"
#include "base/log.h"
#include "base/opt_parser.h"
#include "xcoder/event_handler.h"
#include "xcoder/dispatcher_client.h"

using namespace std;
using namespace agora;
using namespace agora::base;
using namespace agora::recording;

int main(int argc, char *argv[]) {
  bool live = false;
  uint32_t uid = 0;
  string key;
  string name;
  string folder;
  string tag;
  string url;
  uint32_t vid = 0;
  uint32_t mode = 2;
  uint32_t port = 0;
  bool dual = false;

  LOG(INFO, "video mosaic, based on version " GIT_DESC);

  opt_parser parser;
  parser.add_long_opt("uid", &uid);
  parser.add_long_opt("key", &key);
  parser.add_long_opt("name", &name);
  parser.add_long_opt("live", &live);
  parser.add_long_opt("tag", &tag);
  parser.add_long_opt("mode", &mode);
  parser.add_long_opt("dual", &dual);
  parser.add_long_opt("url", &url);
  parser.add_long_opt("port", &port);
  parser.add_long_opt("vendor_id", &vid);

  if (!parser.parse_opts(argc, argv) || key.empty() || name.empty()) {
    parser.print_usage(argv[0], cout);
    return -1;
  }

  LOG(INFO, "uid %" PRIu32 " from vendor %s is joining channel %s",
      uid, key.c_str(), name.c_str());

  event_handler handler(uid, vid, key, name, url, live, mode, dual, port);
  return handler.run();
}
