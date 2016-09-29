#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "internal/rtc/IAgoraRtcEngine.h"
#include "internal/rtc/rtc_engine_i.h"

#include "base/async_pipe.h"
#include "base/atomic.h"
#include "base/packet.h"
#include "base/event_loop.h"
#include "base/event_queue.h"

namespace agora {
namespace recording {

class audio_observer;
class video_observer;

typedef std::unique_ptr<base::packet> frame_ptr_t;

class event_handler : private rtc::IRtcEngineEventHandlerEx,
    private base::pipe_read_listener, private base::pipe_write_listener,
    private base::async_event_handler<frame_ptr_t> {
 public:
  event_handler(uint32_t uid,
      const std::string &vendor_key,
      const std::string &channel_name,
      bool is_dual, int read_fd,
      int write_fd);

  ~event_handler();

  int run();
 private:
  int run_internal();

  virtual void onJoinChannelSuccess(const char *cid, uid_t uid, int elapsed);
  virtual void onRejoinChannelSuccess(const char *cid, uid_t uid, int elapsed);
  virtual void onWarning(int warn, const char *msg);
  virtual void onError(int err, const char *msg);
  virtual void onUserJoined(uid_t uid, int elapsed);
  virtual void onUserOffline(uid_t uid, rtc::USER_OFFLINE_REASON_TYPE reason);
  virtual void onRtcStats(const rtc::RtcStats &stats);
  virtual void onFirstRemoteVideoDecoded(uid_t uid, int width,
      int height, int elapsed);

  // inherited from IRtcEngineEventHandlerEx
  virtual void onLogEvent(int level, const char *msg, int length);

  virtual bool on_receive_packet(base::async_pipe_reader *reader,
      base::unpacker &pkr, uint16_t uri);

  virtual bool on_error(base::async_pipe_reader *reader, short events);
  virtual bool on_error(base::async_pipe_writer *writer, short events);

  // Inherited from |async_event_handler|
  virtual void on_event(frame_ptr_t frame);

  void cleanup();
  void on_leave(int reason);

  static void term_handler(int sig_no);

  event_handler(const event_handler &) = delete;
  event_handler(event_handler &&) = delete;
  event_handler& operator=(const event_handler &) = delete;
  event_handler& operator=(event_handler &&) = delete;
 private:
  const uint32_t uid_;
  const std::string vendor_key_;
  const std::string channel_name_;
  const bool is_dual_;

  atomic_bool_t joined_;
  int32_t last_active_ts_;

  rtc::IRtcEngineEx *applite_;

  std::unique_ptr<audio_observer> audio_;
  std::unique_ptr<video_observer> video_;

  base::async_pipe_reader *reader_;
  base::async_pipe_writer *writer_;
  base::event_loop loop_;

  base::event_queue<frame_ptr_t> frames_;

  static atomic_bool_t s_term_sig_;
  static const unsigned char kBytesPerSample = 2;
  static const unsigned char kChannels = 1;
};

}
}
