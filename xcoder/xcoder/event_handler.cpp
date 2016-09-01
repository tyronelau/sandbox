#include "xcoder/event_handler.h"

#include <poll.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/timerfd.h>

#include <cinttypes>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#ifdef GOOGLE_PROFILE_FLAG
#include <gperftools/profiler.h>
#endif

#include "base/log.h"
#include "base/safe_log.h"

#include "xcoder/audio_observer.h"
#include "xcoder/video_observer.h"

inline int64_t now_us() {
  timeval now;
  gettimeofday(&now, NULL);
  return now.tv_sec * 1000ll * 1000 + now.tv_usec;
}

inline int32_t now_ts() {
  return static_cast<int32_t>(now_us() / 1000llu / 1000);
}

namespace agora {
namespace recording {

using std::string;

atomic_bool_t event_handler::s_term_sig_;

event_handler::event_handler(uint32_t uid, const string &vendor_key,
    const string &channel_name, bool dual)
    : uid_(uid), vendor_key_(vendor_key), channel_name_(channel_name),
    is_dual_(dual) {
  applite_ = NULL;
  joined_ = false;
}

event_handler::~event_handler() {
  if (applite_) {
    cleanup();
  }
}

#ifdef GOOGLE_PROFILE_FLAG
struct profiler_guard {
  explicit profiler_guard(const char *file) {
    LOG(INFO, "Starting profiling...");
    ProfilerStart(file);
  }

  ~profiler_guard() {
    LOG(INFO, "Stopping profiling....");
    ProfilerStop();
  }
};
#endif

void event_handler::cleanup() {
  LOG(INFO, "Leaving channel and ready to cleanup");

  if (applite_) {
    applite_->leaveChannel();
    applite_->release();
    applite_ = NULL;
  }

  sleep(1);
}

int event_handler::run() {
#ifdef GOOGLE_PROFILE_FLAG
  profiler_guard guard("./profile");
#endif

  audio_ = std::unique_ptr<audio_observer>(new audio_observer());
  video_ = std::unique_ptr<video_observer>(new video_observer());

  applite_ = dynamic_cast<rtc::IRtcEngineEx *>(createAgoraRtcEngine());
  if (applite_ == NULL) {
    SAFE_LOG(FATAL) << "Failed to create an Agora Rtc Engine!";
    return -1;
  }

  registerAudioFrameObserver(audio_.get());
  registerVideoFrameObserver(video_.get());

  rtc::RtcEngineContextEx context;
  context.eventHandler = this;
  context.isExHandler = true;
  context.vendorKey = NULL;
  context.context = NULL;
  context.applicationCategory = rtc::APPLICATION_CATEGORY_LIVE_BROADCASTING;

  applite_->initializeEx(context);
  applite_->setLogCallback(true);
  applite_->enableVideo();

  applite_->setProfile("{\"audioEngine\":{\"audioSampleRate\":32000}}", true);

  if (is_dual_) {
    applite_->setClientRole(rtc::CLIENT_ROLE_DUAL_STREAM_AUDIENCE);
  } else {
    applite_->setClientRole(rtc::CLIENT_ROLE_AUDIENCE);
  }

  last_active_ts_ = now_ts();

  // setup signal handler
  s_term_sig_ = false;

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, term_handler);
  signal(SIGTERM, term_handler);

  agora::rtc::AParameter msp(*applite_);

  // set the server_mode to true to enable the callback
  msp->setBool("che.video.server_mode", true);

  if (applite_->joinChannel(vendor_key_.c_str(), channel_name_.c_str(),
      NULL, uid_) < 0) {
    SAFE_LOG(ERROR) << "Failed to create the channel " << channel_name_;
    return -1;
  }

  return run_internal();
}

int event_handler::run_internal() {
  // FIXME: run your event loop here.
  while (!s_term_sig_) {
    sleep(1);
  }

  SAFE_LOG(INFO) << "Ready to leave";

  cleanup();
  return 0;
}

void event_handler::term_handler(int sig_no) {
  (void)sig_no;
  s_term_sig_ = true;
}

void event_handler::onError(int rescode, const char *msg) {
  switch (rescode) {
    default:
      LOG(INFO, "Error in mediasdk: %d, %s", rescode, msg);
      break;
  }
}

void event_handler::onJoinChannelSuccess(const char *channel, uid_t uid,
    int ts) {
  SAFE_LOG(INFO) << uid << " logined successfully in " << channel
    << ", elapsed: " << ts << " ms";

  joined_ = true;
}

void event_handler::onRejoinChannelSuccess(const char *channel, uid_t uid,
    int elapsed) {
  SAFE_LOG(INFO) << uid << " rejoin to channel: " << channel << ", time offset "
    << elapsed << " ms";
}

void event_handler::onWarning(int warn, const char *msg) {
  SAFE_LOG(WARN) << "code: " << warn << ", " << msg;
}

void event_handler::onUserJoined(uid_t uid, int elapsed) {
  SAFE_LOG(INFO) << "offset " << elapsed << " ms: " << uid
    << " joined the channel";
}

void event_handler::onUserOffline(uid_t uid,
    rtc::USER_OFFLINE_REASON_TYPE reason) {
  const char *detail = reason == rtc::USER_OFFLINE_QUIT ? "quit" : "dropped";
  SAFE_LOG(INFO) << "User " << uid << " " << detail;
}

void event_handler::onFirstRemoteVideoDecoded(uid_t uid, int width, int height,
    int elapsed) {
  (void)uid;
  (void)width;
  (void)height;
  (void)elapsed;

  SAFE_LOG(INFO) << "onFirstRemoteVideoDecoded User " << uid;
}

void event_handler::onRtcStats(const rtc::RtcStats &stats) {
  int32_t nowts = now_ts();
  if (stats.users > 1) {
    last_active_ts_ = nowts;
  }

  if (nowts - last_active_ts_ > 30) {
    LOG(WARN, "No users in channel for 30 seconds, aborting.");
    s_term_sig_ = true;
  }
}

void event_handler::onLogEvent(int level, const char *msg, int length) {
  (void)length;

  LOG(INFO, "level %d: %s", level, msg);
}

}
}
