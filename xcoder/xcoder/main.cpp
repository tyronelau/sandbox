#include <sys/stat.h>
#include <sys/types.h>
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

using std::string;
using namespace agora;
using namespace agora::base;
using namespace agora::xcodec;

int main(int argc, char *argv[]) {
  uint32_t uid = 0;
  string key;
  string name;
  bool dual = false;
  bool audio_decode = false;
  bool video_decode = false;
  int read_fd = -1;
  int write_fd = -1;
  int idle = 30000;

  LOG(INFO, "video recorder, based on version " GIT_DESC);

  opt_parser parser;
  parser.add_long_opt("uid", &uid);
  parser.add_long_opt("key", &key);
  parser.add_long_opt("name", &name);
  parser.add_long_opt("dual", &dual);
  parser.add_long_opt("write", &write_fd);
  parser.add_long_opt("read", &read_fd);
  parser.add_long_opt("decode_audio", &audio_decode);
  parser.add_long_opt("decode_video", &video_decode);
  parser.add_long_opt("idle", &idle);

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

  if (idle < 10) {
    idle = 10;
  }

  LOG(INFO, "uid %" PRIu32 " from vendor %s is joining channel %s",
      uid, key.c_str(), name.c_str());

  event_handler handler(uid, key, name, dual, read_fd, write_fd,
      audio_decode, video_decode, idle);

  return handler.run();
}
