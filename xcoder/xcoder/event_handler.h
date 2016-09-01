#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

#include "internal/rtc/IAgoraRtcEngine.h"
#include "internal/rtc/rtc_engine_i.h"

namespace agora {
namespace recording {

class audio_observer;
class video_observer;

class event_handler : private rtc::IRtcEngineEventHandlerEx {
 public:
  event_handler(uint32_t uid,
      const std::string &vendor_key,
      const std::string &channel_name,
      bool is_dual);

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

  void cleanup();

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

  std::atomic<bool> joined_;
  int32_t last_active_ts_;

  rtc::IRtcEngineEx *applite_;

  std::unique_ptr<audio_observer> audio_;
  std::unique_ptr<video_observer> video_;

  static std::atomic<bool> s_term_sig_;

  static const unsigned char kBytesPerSample = 2;
  static const unsigned char kChannels = 1;
};

}
}
