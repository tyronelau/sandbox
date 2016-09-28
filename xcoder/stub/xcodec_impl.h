#pragma once

#include <thread>

#include "include/xcodec_interface.h"
#include "base/async_pipe.h"
#include "base/event_loop.h"
#include "base/process.h"

namespace agora {
namespace base {
class async_pipe_reader;
class async_pipe_writer;
}

namespace xcodec {

class RecorderImpl : public Recorder, private base::pipe_read_listener,
    private base::pipe_write_listener {
 public:
  explicit RecorderImpl(RecorderCallback *callback=NULL);
  ~RecorderImpl();

  virtual int JoinChannel(const char *app_id, const char *channel_name,
      bool is_dual=false, uint_t uid=0);

  virtual int LeaveChannel();
  virtual int Destroy();
 private:
  virtual bool on_receive_packet(base::async_pipe_reader *reader,
      base::unpacker &pkr, uint16_t uri);

  virtual bool on_error(base::async_pipe_reader *reader, short events);

  virtual bool on_error(base::async_pipe_writer *writer, short events);
 private:
  int run_internal();
  int leave_channel();
 private:
  std::thread thread_;
  base::event_loop loop_;

  base::async_pipe_reader *reader_;
  base::async_pipe_writer *writer_;

  base::process process_;

  bool joined_;
  bool stopped_;

  RecorderCallback *callback_;
};

}
}

