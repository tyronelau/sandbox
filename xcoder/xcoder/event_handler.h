#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "internal/ICodingModuleCallback.h"
#include "internal/rtc/IAgoraRtcEngine.h"
#include "internal/rtc/rtc_engine_i.h"

#include "base/async_pipe.h"
#include "base/atomic.h"
#include "base/event_loop.h"
#include "base/event_queue.h"
#include "base/packet.h"

#include "protocol/ipc_protocol.h"
#include "xcoder/simple_audio_jitterbuffer.h"

namespace agora {
namespace xcodec {

class audio_observer;
class video_observer;
class peer_stream;

typedef std::unique_ptr<base::packet> frame_ptr_t;

class event_handler : private rtc::IRtcEngineEventHandlerEx,
    private base::pipe_read_listener, private base::pipe_write_listener,
    private base::async_event_handler<frame_ptr_t>,
    private AgoraRTC::ICMFileObserver {
 public:
  event_handler(uint32_t uid,
      const std::string &vendor_key,
      const std::string &channel_name,
      bool is_dual, int read_fd,
      int write_fd, bool audio_decode,
      bool video_decode, int idle);

  ~event_handler();

  int run();

  // void on_audio_frame(uint32_t uid, uint32_t audio_ts, uint8_t *buffer,
  //     uint32_t length);

  void on_video_frame(uint32_t uid, uint32_t video_ts, uint8_t *buffer,
      uint32_t length);
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

  virtual bool on_error(base::async_pipe_reader *reader, int events);
  virtual bool on_error(base::async_pipe_writer *writer, int events);

  // Inherited from |async_event_handler|
  virtual void on_event(frame_ptr_t frame);

  // Inherited from |ICMFile|
  virtual AgoraRTC::ICMFile* GetICMFileObject(unsigned int uid);
  virtual int InsertRawAudioPacket(unsigned uid, const unsigned char *payload,
      unsigned short payload_size, int payload_type, unsigned int timestamp,
      unsigned short seq_no);

  void cleanup();
  void on_leave(int reason);

  void set_audio_mix_mode(bool mix);
  void set_video_mosaic_mode(bool mosaic);

  static void term_handler(int sig_no);

  event_handler(const event_handler &) = delete;
  event_handler(event_handler &&) = delete;
  event_handler& operator=(const event_handler &) = delete;
  event_handler& operator=(event_handler &&) = delete;
 private:
  void on_timer();
  static void timer_callback(int fd, void *context);
 private:
  const uint32_t uid_;
  const std::string vendor_key_;
  const std::string channel_name_;
  const bool is_dual_;
  const bool audio_decode_;
  const bool video_decode_;

  atomic_bool_t joined_;
  int32_t last_active_ts_;
  int32_t idle_;

  std::mutex lock_;
  rtc::IRtcEngineEx *applite_;

  std::unique_ptr<audio_observer> audio_;
  std::unique_ptr<video_observer> video_;
  std::unordered_map<uint32_t, std::unique_ptr<peer_stream> > streams_;

  base::async_pipe_reader *reader_;
  base::async_pipe_writer *writer_;
  base::event_loop loop_;

  base::timer_event *timer_;
  base::event_queue<frame_ptr_t> frames_;

  std::mutex buffer_mutex_;
  std::unordered_map<uint32_t, SimpleAudioJitterBuffer> jitter_;

  static atomic_bool_t s_term_sig_;
};

}
}
