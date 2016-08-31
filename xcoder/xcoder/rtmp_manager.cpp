#include "xcoder/rtmp_manager.h"

#include <set>

#include "base/safe_log.h"
#include "base/time_util.h"

#include "xcoder/rtmp_stream.h"

namespace agora {
namespace recording {

using std::vector;
using std::set;
using std::string;
using std::vector;
using std::shared_ptr;

rtmp_manager::rtmp_manager(const vector<string> &urls,
    rtmp_event_handler *handler) : mosaic_mode_(false) {
  handler_ = handler;

  stream_start_ts_ = 0;

  pthread_rwlock_init(&rwlock_, NULL);
  start(urls);
}

rtmp_manager::~rtmp_manager() {
  stop_all();

  pthread_rwlock_destroy(&rwlock_);
}

bool rtmp_manager::start(const vector<string> &urls) {
  for (const string &url : urls) {
    if (streams_.find(url) != streams_.end())
      continue;

    rtmp_stream_ptr_t ptr(new rtmp_stream(url, handler_));
    if (ptr->run_rtmp_thread() != 0) {
      SAFE_LOG(ERROR) << "Failed to run rtmp thread for " << url;
      continue;
    }

    streams_[url] = std::move(ptr);
  }

  return true;
}

bool rtmp_manager::add_rtmp_url(const string &url) {
  pthread_rwlock_wrlock(&rwlock_);

  auto it = streams_.find(url);
  if (it == streams_.end()) {
    rtmp_stream_ptr_t p(new (std::nothrow)rtmp_stream(url, handler_));
    if (p->run_rtmp_thread() != 0) {
      pthread_rwlock_unlock(&rwlock_);
      return false;
    }

    streams_[url] = std::move(p);
  }

  pthread_rwlock_unlock(&rwlock_);
  return true;
}

bool rtmp_manager::remove_rtmp_url(const string &url) {
  pthread_rwlock_wrlock(&rwlock_);

  auto it = streams_.find(url);
  if (it == streams_.end()) {
    pthread_rwlock_unlock(&rwlock_);
    return true;
  }

  it->second->stop_rtmp_thread();
  streams_.erase(it);

  pthread_rwlock_unlock(&rwlock_);
  return true;
}

bool rtmp_manager::replace_rtmp_urls(const vector<string> &urls) {
  set<string> added(urls.begin(), urls.end());

  pthread_rwlock_wrlock(&rwlock_);
  for (const auto &url : added) {
    if (streams_.find(url) == streams_.end()) {
      rtmp_stream_ptr_t p(new (std::nothrow)rtmp_stream(url, handler_));
      if (p->run_rtmp_thread() != 0) {
        SAFE_LOG(ERROR) << "Failed to spawn a thread for " << url;
        continue;
      }
      streams_[url] = std::move(p);
    }
  }

  for (auto it = streams_.begin(); it != streams_.end();) {
    if (added.find(it->first) == added.end()) {
      SAFE_LOG(INFO) << "Remove cdn url: " << it->first;
      it = streams_.erase(it);
    } else {
      ++it;
    }
  }

  pthread_rwlock_unlock(&rwlock_);

  return true;
}

bool rtmp_manager::stop_all() {
  for (auto &p : streams_) {
    p.second->stop_rtmp_thread();
  }

  streams_.clear();
  return true;
}

bool rtmp_manager::add_audio_packet(uint32_t uid, const char *frame, int size,
    uint32_t ts) {
  if ((mosaic_mode_ && uid != 0) || (!mosaic_mode_ && uid == 0))
    return false;

  init_start_ts();
  ts = static_cast<uint32_t>(base::now_ms()) - stream_start_ts_;

  static const uint32_t kAdvanceInterval = 1500;

  if (!mosaic_mode_)
    ts += kAdvanceInterval;

  shared_ptr<rtmp_packet> pkt(new rtmp_packet(frame, size, ts, ts));

  pthread_rwlock_rdlock(&rwlock_);
  for (auto &p : streams_) {
    p.second->push_audio_packet(pkt);
  }

  pthread_rwlock_unlock(&rwlock_);
  return true;
}

void rtmp_manager::init_start_ts() {
  if (stream_start_ts_ == 0) {
    std::lock_guard<std::mutex> auto_lock(stream_lock_);
    if (stream_start_ts_ == 0) {
      stream_start_ts_ = static_cast<uint32_t>(base::now_ms());
    }
  }
}

bool rtmp_manager::add_video_packet(uint32_t uid, const char *frame, int size,
    uint32_t ts) {
  if (mosaic_mode_ && uid != 0)
    return false;

  init_start_ts();
  ts = static_cast<uint32_t>(base::now_ms()) - stream_start_ts_;

  shared_ptr<rtmp_packet> pkt(new rtmp_packet(frame, size, ts, ts));

  pthread_rwlock_rdlock(&rwlock_);
  for (auto &p : streams_) {
    p.second->push_video_packet(pkt);
  }

  pthread_rwlock_unlock(&rwlock_);
  return true;
}

bool rtmp_manager::set_mosaic_mode(bool mosaic) {
  mosaic_mode_.store(mosaic);
  return true;
}

vector<string> rtmp_manager::get_rtmp_urls() const {
  vector<string> urls;

  pthread_rwlock_rdlock(&rwlock_);

  for (const auto &p : streams_) {
    urls.push_back(p.first);
  }

  pthread_rwlock_unlock(&rwlock_);
  return urls;
}

}
}
