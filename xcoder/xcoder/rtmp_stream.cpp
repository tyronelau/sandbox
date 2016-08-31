#include "xcoder/rtmp_stream.h"

#include <sys/time.h>
#include <poll.h>

#include <cerrno>
#include <climits>
#include <cstring>
#include <vector>

#include <iostream>

#include "base/safe_log.h"
#include "base/time_util.h"

#include "internal/tick_util.h"
#include "srs_librtmp/srs_librtmp.h"

#include "xcoder/rtmp_event.h"

namespace agora {
namespace recording {

using std::vector;
using std::back_inserter;

static const int ERROR_SOCKET_WRITE = 1009;

inline uint32_t now_seconds() {
  timeval t = {0, 0};
  ::gettimeofday(&t, NULL);
  return static_cast<uint32_t>(t.tv_sec);
}

enum RtmpErrorCode {
  RTMP_ERROR_CREATE    = 0,
  RTMP_ERROR_CONNECT   = 1,
  RTMP_ERROR_PUBLISH   = 2,
  RTMP_ERROR_RECONNECT = 3,
};

inline void rtmp_stream::rtmp_deleter::operator()(void *ptr) {
  if (ptr) srs_rtmp_destroy(ptr);
}

rtmp_stream::rtmp_stream(const std::string &url, rtmp_event_handler *handler)
    :rtmp_url_(url), audio_packets_(kMaxPendingAudioFrames),
    video_packets_(kMaxPendingVideoFrames) {
  callback_ = handler;

  running_ = false;
  stopping_ = false;

  audio_start_ts_ = 0;
  video_start_ts_ = 0;

  last_retry_ts_ = 0;
  last_report_ms_ = static_cast<int64_t>(base::now_ms());
}

rtmp_stream::~rtmp_stream() {
  // do nothing.
}

int rtmp_stream::run_rtmp_thread() {
  if (running_)
    return 0;

  rtmp_thread_ = std::thread(&rtmp_stream::rtmp_thread, this);
  running_ = true;

  return 0;
}

int rtmp_stream::stop_rtmp_thread() {
  if (running_) {
    stopping_ = true;
    rtmp_thread_.join();
    running_ = false;
  }

  return 0;
}

int rtmp_stream::rtmp_thread() {
  int audio_fd = audio_packets_.GetEventFD();
  int video_fd = video_packets_.GetEventFD();

  int cnt = 0;
  pollfd fds[] = {{audio_fd, POLLIN, 0}, {video_fd, POLLIN, 0}};
  while ((cnt = poll(fds, 2, 500)) >= 0 || (cnt == -1 && errno == EINTR)) {
    if (stopping_)
      break;

    report_bandwidth_usage();

    if (cnt <= 0)
      continue;

    for (unsigned i = 0; i < sizeof(fds) / sizeof(fds[0]); ++i) {
      if (fds[i].revents & POLLIN) {
        if (i == 0)
          send_audio_packet();
        else if (i == 1)
          send_video_packet();
        fds[i].revents = 0;
      }
    }
  }

  if (cnt == -1) {
    SAFE_LOG(ERROR) << "Rtmp thread quits. reason: " << strerror(errno);
    return -1;
  }

  if (stopping_) {
    SAFE_LOG(INFO) << "Rtmp thread ended";
  }

  return 0;
}

void rtmp_stream::send_audio_packet() {
  vector<packet_t> packets;
  audio_packets_.TakeAll(back_inserter(packets));

  for (const packet_t &pkt : packets) {
    const char *frame = pkt->frame_.get();
    int frame_size = pkt->size_;
    uint32_t ts = pkt->frame_ts_;
    send_audio_packet_internal(frame, frame_size, ts);
  }
}

void rtmp_stream::send_video_packet() {
  vector<packet_t> packets;
  video_packets_.TakeAll(back_inserter(packets));

  for (const packet_t &pkt : packets) {
    const char *frame = pkt->frame_.get();
    int frame_size = pkt->size_;
    uint32_t ts = pkt->frame_ts_;
    send_video_packet_internal(frame, frame_size, ts);
  }
}

bool rtmp_stream::send_video_packet_internal(const char *frame, int size,
    uint32_t ts) {
  LOG(DEBUG, "[RTMP] push video size=%d", size);

  uint32_t dts = ts;
  // ts = static_cast<uint32_t>(base::now_ms());
  // // ts = static_cast<uint32_t>(AgoraRTC::TickTime::MicrosecondTimestamp() / 1000);
  // if (video_start_ts_ == 0) {
  //   video_start_ts_ = ts;
  // } else {
  //   // dts = (ts - video_start_ts_) / 90;
  //   dts = ts - video_start_ts_;
  // }

  // std::cout << "dts: " << dts << std::endl;

  if (!handle_ && !create_rtmp_connection()) {
    return false;
  }

  char *buf = const_cast<char *>(frame);
  int ret = srs_h264_write_raw_frames(handle_.get(), buf, size, dts, dts);

  if (ret == ERROR_SOCKET_WRITE) {
    LOG(WARN, "[RTMP] error socket write, url=%s, code=%d",
        rtmp_url_.c_str(), ret);
    handle_.reset();
    create_rtmp_connection();
  } else {
    xferred_bytes_ += size;
  }

  return true;
}

bool rtmp_stream::send_audio_packet_internal(const char *frame, int size,
    uint32_t ts) {
  LOG(DEBUG, "[RTMP] push audio size=%d", size);

  uint32_t dts = ts;
  // ts = 0;

  // if (ts == 0) {
  //   if (audio_start_ts_ == 0) {
  //     audio_start_ts_ = static_cast<uint32_t>(base::now_ms());
  //     // audio_start_ts_ = static_cast<uint32_t>(AgoraRTC::TickTime::MicrosecondTimestamp() / 1000);
  //   } else {
  //     dts = static_cast<uint32_t>(base::now_ms() - audio_start_ts_);
  //     // dts = static_cast<uint32_t>(AgoraRTC::TickTime::MicrosecondTimestamp() / 1000 - audio_start_ts_);
  //   }
  // } else {
  //   dts = ts;
  // }

  if (!handle_ && !create_rtmp_connection()) {
    return false;
  }

  rtmp_handle_t handle = handle_.get();
  char *buf = const_cast<char *>(frame);
  int ret = srs_audio_write_raw_frame(handle, 10, 0, 1, 1, buf, size, dts);

  if (ret != 0) {
    LOG(ERROR, "[RTMP] error write audio failed url=%s, ret=%d",
        rtmp_url_.c_str(), ret);

    if (ret == ERROR_SOCKET_WRITE) {
      handle_.reset();
      create_rtmp_connection();
    }
  } else {
    xferred_bytes_ += size;
  }

  return true;
}

bool rtmp_stream::create_rtmp_connection() {
  if (handle_) {
    LOG(INFO, "Rtmp connection already exists! %s", rtmp_url_.c_str());
    return true;
  }

  uint32_t now_ts = now_seconds();
  if (last_retry_ts_ + kMinRetryInterval > now_ts) {
    // LOG(INFO, "too early to reconnect to %s", rtmp_url_.c_str());
    return false;
  }

  last_retry_ts_ = now_ts;

  LOG(INFO, "[RTMP] ready to create rtmp %s and handshake", rtmp_url_.c_str());

  rtmp_handle_t rtmp = srs_rtmp_create(rtmp_url_.c_str());
  if (!rtmp) {
    SAFE_LOG(ERROR) << "Failed to create a socket to Rtmp server" << rtmp_url_;
    return false;
  }

  handle_.reset(rtmp);

  if (srs_rtmp_handshake(rtmp) != 0) {
    LOG(ERROR, "[RTMP] error handshake: %s", rtmp_url_.c_str());
    handle_.reset();

    if (callback_)
      callback_->on_rtmp_error(rtmp_url_, RTMP_ERROR_CREATE);
    return false;
  }

  if (srs_rtmp_connect_app(rtmp) != 0) {
    LOG(INFO, "[RTMP] error connect app: %s", rtmp_url_.c_str());
    handle_.reset();

    if (callback_)
      callback_->on_rtmp_error(rtmp_url_, RTMP_ERROR_CONNECT);
    return false;
  }

  if (srs_rtmp_publish_stream(rtmp) != 0) {
    LOG(INFO, "[RTMP] error publish stream %s", rtmp_url_.c_str());
    handle_.reset();

    if (callback_)
      callback_->on_rtmp_error(rtmp_url_, RTMP_ERROR_PUBLISH);
    return false;
  }

  return true;
}

bool rtmp_stream::push_audio_packet(std::shared_ptr<rtmp_packet> pkt) {
  return audio_packets_.Push(std::move(pkt));
}

bool rtmp_stream::push_video_packet(std::shared_ptr<rtmp_packet> pkt) {
  return video_packets_.Push(std::move(pkt));
}

void rtmp_stream::report_bandwidth_usage() {
  static const int64_t kReportInterval = 10 * 1000;
  int64_t now = base::now_ms();
  if (last_report_ms_ + kReportInterval < now) {
    double kbps = CHAR_BIT * xferred_bytes_ / double(now - last_report_ms_);

    xferred_bytes_ = 0;
    last_report_ms_ = now;
    if (callback_) {
      callback_->on_bandwidth_report(rtmp_url_, static_cast<int>(kbps));
    }
  }
}

}
}
