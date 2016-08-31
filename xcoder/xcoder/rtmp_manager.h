#pragma once

#include <pthread.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace agora {
namespace recording {

class rtmp_stream;
struct rtmp_event_handler;

// NOTE(liuyong):
// These class methods are not thread-safe. must be called from a same thread
class rtmp_manager {
  typedef std::unique_ptr<rtmp_stream> rtmp_stream_ptr_t;
 public:
  explicit rtmp_manager(const std::vector<std::string> &urls,
      rtmp_event_handler *handler=NULL);

  ~rtmp_manager();

  // The following three functions are thread-safe.
  bool add_audio_packet(uint32_t uid, const char *frame, int size, uint32_t ts);
  bool add_video_packet(uint32_t uid, const char *frame, int size, uint32_t ts);

  bool set_mosaic_mode(bool mosaic);
  bool is_mosaic_mode() const;

  std::vector<std::string> get_rtmp_urls() const;

  bool add_rtmp_url(const std::string &url);
  bool remove_rtmp_url(const std::string &url);
  bool replace_rtmp_urls(const std::vector<std::string> &urls);

  bool stop_all();
 private:
  bool start(const std::vector<std::string> &urls);
  void init_start_ts();
 private:
  rtmp_event_handler *handler_;

  mutable std::mutex stream_lock_;
  std::atomic<uint32_t> stream_start_ts_;

  std::atomic<bool> mosaic_mode_;
  mutable pthread_rwlock_t rwlock_;
  std::unordered_map<std::string, rtmp_stream_ptr_t> streams_;
};

inline bool rtmp_manager::is_mosaic_mode() const {
  return mosaic_mode_.load();
}

}
}
