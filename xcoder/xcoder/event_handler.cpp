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

#include "base/event_queue.h"
#include "base/log.h"
#include "base/packet.h"
#include "base/safe_log.h"

#include "protocol/ipc_protocol.h"

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
namespace xcodec {

using std::string;

using base::async_pipe_reader;
using base::async_pipe_writer;
using base::unpacker;
using base::event_queue;

using std::cout;
using std::endl;

class peer_stream : public AgoraRTC::ICMFile {
 public:
  typedef std::unique_ptr<base::packet> frame_ptr_t;

  explicit peer_stream(event_queue<frame_ptr_t> *q, unsigned int uid=0);
  virtual ~peer_stream();

  // The following functions are left intentionally unimplemented.
  virtual int startAudioRecord();
  virtual int startVideoRecord();
  virtual int stopAudioRecord();
  virtual int stopVideoRecord();
  virtual int setVideoRotation(int rotation);

  virtual int onDecodeVideo(uint32_t video_ts, uint8_t payload_type,
      uint8_t *buffer, uint32_t length, uint32_t frame_num);

  virtual int onEncodeVideo(uint32_t video_ts, uint8_t payload_type,
      uint8_t *buffer, uint32_t length);

  virtual int onDecodeAudio(uint32_t audio_ts, uint8_t payload_type,
      uint8_t *buffer, uint32_t length);

  virtual int onEncodeAudio(uint32_t audio_ts, uint8_t payload_type,
      uint8_t *buffer, uint32_t length);
 private:
  event_queue<frame_ptr_t> *queue_;
  unsigned uid_;
};

peer_stream::peer_stream(event_queue<frame_ptr_t> *q, unsigned int uid) {
  queue_ = q;
  uid_ = uid;
}

peer_stream::~peer_stream() {
}

int peer_stream::startAudioRecord() {
  return 0;
}

int peer_stream::startVideoRecord() {
  return 0;
}

int peer_stream::stopAudioRecord() {
  return 0;
}

int peer_stream::stopVideoRecord() {
  return 0;
}

int peer_stream::setVideoRotation(int rotation) {
  (void)rotation;

  return 0;
}

int peer_stream::onDecodeVideo(uint32_t video_ts, uint8_t payload_type,
    uint8_t *buffer, uint32_t length, uint32_t frame_num) {
  (void)payload_type;
  (void)frame_num;

  if (!queue_)
    return 0;

  protocol::h264_frame *f = new protocol::h264_frame;
  f->uid = uid_;
  f->frame_ms = video_ts;
  f->frame_num = frame_num;

  std::string &data = f->data;
  data.reserve(length);

  const char *start = reinterpret_cast<const char *>(buffer);
  data.insert(data.end(), start, start + length);

  queue_->push(frame_ptr_t(f));
  return 0;
}

int peer_stream::onEncodeVideo(uint32_t video_ts, uint8_t payload_type,
    uint8_t *buffer, uint32_t length) {
  (void)video_ts;
  (void)payload_type;
  (void)buffer;
  (void)length;

  assert(false);
  return -1;
}

int peer_stream::onDecodeAudio(uint32_t audio_ts, uint8_t payload_type,
    uint8_t *buffer, uint32_t length) {
  (void)audio_ts;
  (void)payload_type;
  (void)buffer;
  (void)length;

  // We have handled this audio packet in audio_observer, ignore it.
  // if (handler_) {
  //   handler_->on_audio_frame(uid_, audio_ts, buffer, length);
  // }

  return 0;
}

int peer_stream::onEncodeAudio(uint32_t audio_ts, uint8_t payload_type,
    uint8_t *buffer, uint32_t length) {
  (void)audio_ts;
  (void)payload_type;
  (void)buffer;
  (void)length;

  assert(false);
  return -1;
}

atomic_bool_t event_handler::s_term_sig_;

event_handler::event_handler(uint32_t uid, const string &vendor_key,
    const string &channel_name, bool dual, int read_fd, int write_fd,
    bool decode) : uid_(uid), vendor_key_(vendor_key),
    channel_name_(channel_name), is_dual_(dual), decode_(decode),
    frames_(&loop_, this, 128) {
  applite_ = NULL;
  joined_ = false;
  timer_ = NULL;

  reader_ = new (std::nothrow)async_pipe_reader(&loop_, read_fd, this);
  writer_ = new (std::nothrow)async_pipe_writer(&loop_, write_fd, this);
}

event_handler::~event_handler() {
  if (applite_) {
    cleanup();
  }

  delete reader_;
  delete writer_;

  // if (timer_) {
  //   loop_.remove_timer(timer_);
  // }
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

void event_handler::set_mosaic_mode(bool mosaic) {
  SAFE_LOG(INFO) << "Set mosaic mode: " << mosaic;

  agora::rtc::AParameter msp(*applite_);
  msp->setBool("che.video.server_mode", mosaic);
}

int event_handler::run() {
#ifdef GOOGLE_PROFILE_FLAG
  profiler_guard guard("./profile");
#endif

  audio_ = std::unique_ptr<audio_observer>(new audio_observer(&frames_));
  video_ = std::unique_ptr<video_observer>(new video_observer(&frames_));

  applite_ = dynamic_cast<rtc::IRtcEngineEx *>(createAgoraRtcEngine());
  if (applite_ == NULL) {
    SAFE_LOG(FATAL) << "Failed to create an Agora Rtc Engine!";
    return -1;
  }

  registerAudioFrameObserver(audio_.get());
  registerVideoFrameObserver(video_.get());

  RegisterICMFileObserver(this);

  rtc::RtcEngineContextEx context;
  context.eventHandler = this;
  context.isExHandler = true;
  context.appId = vendor_key_.c_str();
  context.context = NULL;

  applite_->initializeEx(context);
  applite_->setLogCallback(true);
  applite_->setChannelProfile(rtc::CHANNEL_PROFILE_LIVE_BROADCASTING);

  applite_->setProfile("{\"audioEngine\":{\"useAudioExternalDevice\":true}}", true);
  applite_->setProfile("{\"audioEngine\":{\"audioSampleRate\":32000}}", true);

  applite_->setClientRole(rtc::CLIENT_ROLE_AUDIENCE, NULL);

  if (is_dual_) {
    rtc::RtcEngineParameters param(*applite_);
    param.enableDualStreamMode(true);
  }

  applite_->enableVideo();

  last_active_ts_ = now_ts();

  // setup signal handler
  s_term_sig_ = false;

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, term_handler);
  signal(SIGTERM, term_handler);

  set_mosaic_mode(decode_);

  // agora::rtc::AParameter msp(*applite_);

  // set the server_mode to true to enable the callback
  // msp->setBool("che.video.server_mode", true);

  if (applite_->joinChannel(vendor_key_.c_str(), channel_name_.c_str(),
      NULL, uid_) < 0) {
    SAFE_LOG(ERROR) << "Failed to create the channel " << channel_name_;
    if (writer_) {
      protocol::recorder_error error;
      error.error_code = 1;
      error.reason = "Failed to create the channel";

      writer_->write_packet(error);
    }

    return -1;
  }

  return run_internal();
}

void event_handler::timer_callback(int fd, void *context) {
  (void)fd;

  event_handler *p = reinterpret_cast<event_handler *>(context);
  p->on_timer();
}

void event_handler::on_timer() {
  if (s_term_sig_) {
    loop_.stop();
  }
}

int event_handler::run_internal() {
  timer_ = loop_.add_timer(500, event_handler::timer_callback, this);
  return loop_.run();

  // FIXME: run your event loop here.
  // while (!s_term_sig_) {
  //   sleep(1);
  // }

  // SAFE_LOG(INFO) << "Ready to leave";

  // cleanup();
  // return 0;
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

  if (nowts - last_active_ts_ > 300) {
    LOG(WARN, "No users in channel for 300 seconds, aborting.");
    s_term_sig_ = true;
  }
}

void event_handler::onLogEvent(int level, const char *msg, int length) {
  (void)length;

  LOG(INFO, "level %d: %s", level, msg);
}

bool event_handler::on_receive_packet(async_pipe_reader *reader, unpacker &pkr,
    uint16_t uri) {
  (void)reader;
  assert(reader == reader_);

  switch (uri) {
  case protocol::LEAVE_URI: {
    protocol::leave_packet leave;
    leave.unmarshall(pkr);
    on_leave(leave.reason);
    break;
  }
  default: break;
  }

  return true;
}

void event_handler::on_leave(int reason) {
  LOG(INFO, "Required to stop :%d", reason);

  loop_.stop();
}

bool event_handler::on_error(base::async_pipe_reader *reader, short events) {
  (void)reader;
  (void)events;

  assert(reader == reader_);

  LOG(INFO, "Reading pipe broken");
  loop_.stop();

  return true;
}

bool event_handler::on_error(base::async_pipe_writer *writer, short events) {
  (void)writer;
  (void)events;

  assert(writer == writer_);

  LOG(INFO, "Writing pipe broken");
  loop_.stop();

  return true;
}

void event_handler::on_event(frame_ptr_t frame) {
  if (writer_) {
    writer_->write_packet(*frame.get());
  }
}

AgoraRTC::ICMFile* event_handler::GetICMFileObject(unsigned uid) {
  std::lock_guard<std::mutex> auto_lock(lock_);
  auto it = streams_.find(uid);
  if (it == streams_.end()) {
    it = streams_.insert(std::make_pair(uid, std::unique_ptr<peer_stream>(
        new peer_stream(&frames_, uid)))).first;
  }

  return it->second.get();
}

int event_handler::InsertRawAudioPacket(unsigned uid, const unsigned char *payload,
    unsigned short payload_size, int payload_type, unsigned int timestamp,
    unsigned short seq_no) {
  (void)uid;
  (void)payload;
  (void)payload_size;
  (void)payload_type;
  (void)timestamp;
  (void)seq_no;

  return 0;
}

// void event_handler::on_audio_frame(uint32_t uid, uint32_t audio_ts,
//     uint8_t *buffer, uint32_t length) {
//   (void)audio_ts;
//   (void)buffer;
// 
//   SAFE_LOG(INFO) << "audio frame received from " << uid << ", length: "
//       << length;
// }

void event_handler::on_video_frame(uint32_t uid, uint32_t video_ts,
    uint8_t *buffer, uint32_t length) {
  (void)video_ts;
  (void)buffer;

  SAFE_LOG(INFO) << "H264 video frame received from " << uid << ", length: "
      << length;
}

}
}
