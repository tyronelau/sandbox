#pragma once

#include <thread>

#include "include/xcodec_interface.h"

#include "base/async_pipe.h"
#include "base/process.h"

namespace agora {
namespace xcodec {

class async_pipe_reader;
class async_pipe_writer;

class RecorderImpl : public Recorder {
 public:
  explicit RecorderImpl(RecorderCallback *callback=NULL);
  ~RecorderImpl();

  virtual int JoinChannel(const char *vendor_key, const char *channel_name,
      bool is_dual=false, uint_t uid=0);

  virtual int LeaveChannel();
  virtual int Destroy();
 private:
  int run_internal();
 private:
  std::thread thread_;

  async_pipe_reader *reader_;
  async_pipe_writer *writer_;

  base::process process_;

  bool joined_;
  bool stopped_;

  RecorderCallback *callback_;
};

}
}
