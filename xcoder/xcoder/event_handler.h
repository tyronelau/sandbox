#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <map>

#include "internal/rtc/IAgoraRtcEngine.h"
#include "internal/rtc/rtc_engine_i.h"

#include "base/atomic.h"
#include "base/packet_pipe.h"
#include "base/network_loop.h"

#include "xcoder/dispatcher_client.h"
#include "xcoder/engine_callback.h"
#include "xcoder/rtmp_event.h"

struct event;

namespace agora {
namespace base {
class network_loop;
}

namespace recording {

struct view_size {
  int width;
  int height;
};

class event_handler : private rtc::IRtcEngineEventHandlerEx,
    private base::timer_listener, private dispatcher_event_handler {
 public:
  event_handler(uint32_t uid,
      const uint32_t vendor_id,
      const std::string &vendor_key,
      const std::string &channel_name,
      const std::string &rtmp_url,
      bool is_live, uint32_t mode, bool is_dual, uint16_t port);

  ~event_handler();

  int run();
  base::network_loop& get_main_loop();
 private:
  int run_internal();
  void set_mosaic_mode(bool mosaic);

  // The following four functions inherited from |dispatcher_event_handler|
  virtual std::vector<std::string> get_rtmp_urls() const;

  virtual void on_add_publisher(const std::string &rtmp_url);
  virtual void on_remove_publisher(const std::string &rtmp_url);
  virtual void on_replace_publisher(const std::vector<std::string> &urls);

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

  virtual void on_timer();

  void cleanup();
  void changeSize();
  void setMosaicSize(int width, int height);
  void handle_request(base::unpacker &pk, uint16_t uri);

  static void term_handler(int sig_no);

  event_handler(const event_handler &) = delete;
  event_handler(event_handler &&) = delete;
  event_handler& operator=(const event_handler &) = delete;
  event_handler& operator=(event_handler &&) = delete;
 private:
  const uint32_t uid_;
  const uint32_t vendor_id_;
  const std::string vendor_key_;
  const std::string channel_name_;
  const std::string rtmp_url_;
  const bool is_live_;
  const bool is_dual_;
  const uint32_t mode_;
  const uint16_t dispatcher_port_;

  atomic_bool_t joined_;
  int32_t last_active_ts_;

  base::network_loop net_;
  std::unique_ptr<dispatcher_client> disp_client_;

  std::map<uid_t, view_size> user_list_;

  rtc::IRtcEngineEx *applite_;
  event *timer_;

  std::unique_ptr<StreamObserver> stream_observer_;

  typedef std::vector<char> packet_buffer_t;
  std::queue<packet_buffer_t> pending_packets_;

  static atomic_bool_t term_sig;

  static const unsigned char kBytesPerSample = 2;
  static const unsigned char kChannels = 1;
};

inline base::network_loop& event_handler::get_main_loop() {
  return net_;
}

}
}
