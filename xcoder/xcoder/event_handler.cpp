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

#include "base/packer.h"
#include "base/process.h"
#include "base/safe_log.h"

#include "xcoder/event_handler.h"

#include "xcoder/audio_observer.h"
#include "xcoder/dispatcher_client.h"
#include "xcoder/video_observer.h"

static const int kFrameInterval = 10;
extern bool mosaicState;

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
using std::vector;

using namespace rtc;

atomic_bool_t event_handler::term_sig;

static std::vector<std::string> split_string(const std::string &s, char delim) {
  std::vector<std::string> elems;
  std::stringstream ss(s);
  std::string item;

  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }

  return elems;
}

event_handler::event_handler(uint32_t uid, uint32_t vendor_id,
    const string &vendor_key,
    const string &channel_name, // const string &record_folder,
    const std::string &rtmp_url, bool live, uint32_t mode, bool dual,
    uint16_t dispatcher_port)
    : uid_(uid), vendor_id_(vendor_id), vendor_key_(vendor_key),
    channel_name_(channel_name), rtmp_url_(rtmp_url), is_live_(live),
    is_dual_(dual), mode_(mode),
    dispatcher_port_(dispatcher_port) {
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

win_test::MosaicObserver ob;
win_test::MosaicAudioObserver aob;

int event_handler::run() {
#ifdef GOOGLE_PROFILE_FLAG
  profiler_guard guard("./profile");
#endif

  if (!disp_client_) {
    disp_client_.reset(new dispatcher_client(net_.event_base(), vendor_id_,
        channel_name_, dispatcher_port_, this));
  }

  stream_observer_.reset(new StreamObserver(split_string(rtmp_url_, ','),
        disp_client_.get()));

  disp_client_->connect();

  user_list_.clear();
  applite_ = dynamic_cast<rtc::IRtcEngineEx*>(createAgoraRtcEngine());
  if (applite_ == NULL) {
    SAFE_LOG(FATAL) << "Failed to create an Agora Rtc Engine!";
    return -1;
  }

  RegisterICMFileObserver(stream_observer_.get());

  ob.setMode(mode_);
  ob.setMaster(uid_);

  registerVideoFrameObserver(&ob);
  registerAudioFrameObserver(&aob);

  rtc::RtcEngineContextEx context;
  context.eventHandler = this;
  context.isExHandler = true;
  context.vendorKey = NULL;
  context.context = NULL;
  context.applicationCategory = rtc::APPLICATION_CATEGORY_LIVE_BROADCASTING;

  applite_->initializeEx(context);
  applite_->setLogCallback(true);
  applite_->enableVideo();

  applite_->setProfile("{\"audioEngine\":{\"useAudioExternalDevice\":true}}", true);
  applite_->setProfile("{\"audioEngine\":{\"audioSampleRate\":32000}}", true);

  if (is_live_) {
    if (is_dual_) {
      applite_->setClientRole(rtc::CLIENT_ROLE_DUAL_STREAM_AUDIENCE);
    } else {
      applite_->setClientRole(rtc::CLIENT_ROLE_AUDIENCE);
    }
  }

  applite_->setProfile("{\"audioEngine\":{\"receiveMode\":false}}", true);
  applite_->setProfile("{\"audioEngine\":{\"enableDualStream\":false}}", true);

  last_active_ts_ = now_ts();

  // setup signal handler
  term_sig = false;
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, term_handler);
  signal(SIGTERM, term_handler);

  agora::rtc::AParameter msp(*applite_);

  msp->setBool("rtc.audio.mute_me", true);
  msp->setBool("che.audio.mute_me", true);

  msp->setInt("che.video.local.camera_index", 1024);

  char resolution[100];
  snprintf(resolution, sizeof(resolution), "{\"width\":%d, \"height\":%d}", 640, 360);
  msp->setObject("che.video.local.resolution", resolution);

  // set mode
  msp->setBool("che.video.server_mode", false);
  if (applite_->joinChannel(vendor_key_.c_str(), channel_name_.c_str(),
        NULL, uid_) < 0) {
    SAFE_LOG(ERROR) << "Failed to create the channel " << channel_name_;
    return -2;
  }

  return run_internal();
}

void event_handler::set_mosaic_mode(bool mosaic) {
  stream_observer_->set_mosaic_mode(mosaic);

  agora::rtc::AParameter msp(*applite_);
  msp->setBool("che.video.server_mode", mosaic);
}

int event_handler::run_internal() {
  timer_ = net_.add_timer(500 * 1000llu, this);

  net_.run();

  LOG(WARN, "Before cleanup cnt %d, errno %d %d, joined_ %d, term_sig %d",
      (int)user_list_.size(), (int)errno, (int)EINTR, (int)joined_,
      (int)term_sig);

  cleanup();
  return 0;
}

void event_handler::term_handler(int sig_no) {
  (void)sig_no;
  term_sig = true;
}

void event_handler::on_timer() {
  if (term_sig) {
    net_.remove_timer(timer_);
    disp_client_.reset();
    net_.stop(NULL);
  }
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

  LOG(INFO, "User %u join channel %s success", uid, channel);
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
  view_size newUser;
  newUser.width = 0;
  newUser.height = 0;
  std::map<uid_t,view_size>::iterator it = user_list_.find(uid);
  rtc::RtcEngineParameters param(*applite_);

  if (it == user_list_.end()) {
    //    if (user_list_.size() == 0) {
    user_list_.insert(std::pair<uid_t, view_size>(uid, newUser));
    param.setRemoteVideoStreamType(uid, rtc::REMOTE_VIDEO_STREAM_HIGH);

    SAFE_LOG(INFO) << "User " << uid << " request stream high";
  }

  if (user_list_.size() > 1)
    set_mosaic_mode(true);
}

void event_handler::onUserOffline(uid_t uid,
    rtc::USER_OFFLINE_REASON_TYPE reason) {
  const char *detail = reason == USER_OFFLINE_QUIT ? "quit" : "dropped";
  SAFE_LOG(INFO) << "User " << uid << " " << detail;

  std::map<uid_t, view_size>::iterator it = user_list_.find(uid);
  if(it != user_list_.end()) {
    user_list_.erase(it);
  }

  //changeSize();

  if (user_list_.size() <= 1)
    set_mosaic_mode(false);
  ob.setActiveRender(uid, false);
}

void event_handler::onFirstRemoteVideoDecoded(uid_t uid, int width, int height, int elapsed) {
  (void)elapsed;
  SAFE_LOG(INFO) << "onFirstRemoteVideoDecoded User " << uid;
  std::map<uid_t, view_size>::iterator it = user_list_.find(uid);
  if(it != user_list_.end()) {
    it->second.width = width;
    it->second.height = height;
  }
  //changeSize();
  setMosaicSize(width, height);

  ob.setActiveRender(uid, true);
}

void event_handler::changeSize() {
  int mwidth = 0, mheight = 0;
  if(user_list_.size() >= 1) {
    std::map<uid_t, view_size>::iterator it;
    if(mode_ == 0) {
      for(it = user_list_.begin(); it != user_list_.end(); it++) {
        if(it->second.height > mheight)
          mheight = it->second.height;
        mwidth += it->second.width;
      }
    }
    if(mode_ == 1) {
      it = user_list_.begin();
      mheight = it->second.height;
      mwidth = user_list_.size() * it->second.width;
    }
    if(mode_ == 2) {
      it = user_list_.begin();
      mheight = it->second.height;
      mwidth = it->second.width;
    }
    LOG(INFO, "Mosaic size change to width=%d, height=%d", mwidth, mheight);
    //set the resolution
    agora::rtc::AParameter msp(*applite_);
    char resolution[100];
    snprintf(resolution, sizeof(resolution), "{\"width\":%d, \"height\":%d}", mwidth, mheight);
    msp->setObject("che.video.local.resolution", resolution);
  }
}

void event_handler::setMosaicSize(int width, int height) {
  float ratio = (float)(width) / (float)(height);
  int mwidth, mheight;
  if(ratio > 1.5f)
    mwidth = 640, mheight = 360;
  else if(ratio > 1.15f)
    mwidth = 640, mheight = 480;
  else if(ratio > 0.85f)
    mwidth = 360, mheight = 360;
  else if(ratio > 0.65f)
    mwidth = 480, mheight = 640;
  else
    mwidth = 360, mheight = 640;

  LOG(INFO, "Mosaic size set to width=%d, height=%d", mwidth, mheight);
  //set the resolution
  agora::rtc::AParameter msp(*applite_);
  char resolution[100];
  snprintf(resolution, sizeof(resolution), "{\"width\":%d, \"height\":%d}", mwidth, mheight);
  msp->setObject("che.video.local.resolution", resolution);
}

void event_handler::onRtcStats(const rtc::RtcStats &stats) {
  int32_t nowts = now_ts();
  if (stats.users > 1) {
    last_active_ts_ = nowts;
  }
  if (nowts - last_active_ts_ > 30) {
    LOG(WARN, "No users in channel for 30 seconds, aborting.");
    term_sig = true;
  }
}

void event_handler::onLogEvent(int level, const char *msg, int length) {
  (void)length;
  LOG(INFO, "level %d: %s", level, msg);
}

vector<string> event_handler::get_rtmp_urls() const {
  assert(stream_observer_ != NULL);
  return stream_observer_->get_rtmp_urls();
}

void event_handler::on_add_publisher(const string &url) {
  assert(stream_observer_ != NULL);
  stream_observer_->add_rtmp_url(url);
}

void event_handler::on_remove_publisher(const string &url) {
  assert(stream_observer_ != NULL);
  stream_observer_->remove_rtmp_url(url);
}

void event_handler::on_replace_publisher(const vector<string> &urls) {
  assert(stream_observer_ != NULL);
  stream_observer_->replace_rtmp_urls(urls);
}

}
}
