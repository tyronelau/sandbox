#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "base/async_queue.h"
#include "xcoder/rtmp_packet.h"

namespace agora {
namespace recording {

struct rtmp_event_handler;

class rtmp_stream {
  typedef void *rtmp_handle_t;
  typedef std::shared_ptr<rtmp_packet> packet_t;

  struct rtmp_deleter {
    void operator()(void *ptr);
  };
 public:
  explicit rtmp_stream(const std::string &url,
      rtmp_event_handler *handler=NULL);

  ~rtmp_stream();

  int run_rtmp_thread();
  int stop_rtmp_thread();

  bool push_audio_packet(std::shared_ptr<rtmp_packet> pkt);
  bool push_video_packet(std::shared_ptr<rtmp_packet> pkt);
 private:
  int rtmp_thread();
  bool create_rtmp_connection();

  void report_bandwidth_usage();

  void send_audio_packet();
  void send_video_packet();

  bool send_audio_packet_internal(const char *frame, int size, uint32_t ts);
  bool send_video_packet_internal(const char *frame, int size, uint32_t ts);
 private:
  std::unique_ptr<void, rtmp_deleter> handle_;
  rtmp_event_handler *callback_;

  static const int kMaxPendingAudioFrames = 256;
  static const int kMaxPendingVideoFrames = 64;

  // At least 2 seconds before another retry to connect the rtmp server.
  static const int kMinRetryInterval = 2;

  std::string rtmp_url_;

  AsyncQueue<packet_t> audio_packets_;
  AsyncQueue<packet_t> video_packets_;

  uint32_t audio_start_ts_;
  uint32_t video_start_ts_;
  uint32_t last_retry_ts_;
  int32_t xferred_bytes_;

  int64_t last_report_ms_;

  std::atomic<bool> running_;
  std::atomic<bool> stopping_;

  std::thread rtmp_thread_;
};

}
}
