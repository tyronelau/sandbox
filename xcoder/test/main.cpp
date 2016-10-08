#include <csignal>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

#include "include/xcodec_interface.h"

#include "base/atomic.h"
#include "base/log.h"
#include "base/opt_parser.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

using agora::base::opt_parser;
using agora::xcodec::AudioFrame;
using agora::xcodec::VideoFrame;
using agora::xcodec::Recorder;

class AgoraRecorder : public agora::xcodec::RecorderCallback {
 public:
  AgoraRecorder();
  ~AgoraRecorder();

  bool CreateChannel(const string &key, const string &name, uint32_t uid);
  bool DestroyChannel();

  bool Stopped() const;
 private:
  virtual void RecorderError(int error, const char *reason);

  virtual void RemoteUserJoined(unsigned int uid);
  virtual void RemoteUserDropped(unsigned int uid);

  virtual void AudioFrameReceived(unsigned int uid, AudioFrame *frame);
  virtual void VideoFrameReceived(unsigned int uid, VideoFrame *frame);
 private:
  atomic_bool_t stopped_;
  Recorder *recorder_;
};

AgoraRecorder::AgoraRecorder() {
  recorder_ = NULL;
  stopped_.store(false);
}

AgoraRecorder::~AgoraRecorder() {
  if (recorder_) {
    DestroyChannel();
  }
}

bool AgoraRecorder::Stopped() const {
  return stopped_;
}

bool AgoraRecorder::CreateChannel(const string &key, const string &name,
    uint32_t uid) {
  if ((recorder_ = agora::xcodec::Recorder::CreateRecorder(this)) == NULL)
    return false;

  return 0 == recorder_->JoinChannel(key.c_str(), name.c_str(), false, uid);
}

bool AgoraRecorder::DestroyChannel() {
  if (recorder_) {
    recorder_->LeaveChannel();
    recorder_->Destroy();
    recorder_ = NULL;
    stopped_ = true;
  }

  return true;
}

void AgoraRecorder::RecorderError(int error, const char *reason) {
  cerr << "Error: " << error << ", " << reason << endl;
  DestroyChannel();
}

void AgoraRecorder::RemoteUserJoined(unsigned uid) {
  cout << "User " << uid << " joined" << endl;
}

void AgoraRecorder::RemoteUserDropped(unsigned uid) {
  cout << "User " << uid << " dropped" << endl;
}

void AgoraRecorder::AudioFrameReceived(unsigned int uid, AudioFrame *frame) {
  (void)frame;
  cout << "User " << uid << ", received an audio frame" << endl;
}

void AgoraRecorder::VideoFrameReceived(unsigned int uid, VideoFrame *frame) {
  cout << "User " << uid << ", received a video frame, width: "
      << frame->width_ << ", height: " << frame->height_ << endl;
}

atomic_bool_t s_stop_flag;

void signal_handler(int signo) {
  (void)signo;

  cerr << "Signal " << signo << endl;
  s_stop_flag = true;
}

int main(int argc, char * const argv[]) {
  uint32_t uid = 0;
  string key;
  string name;
  // bool dual = false;

  s_stop_flag = false;
  signal(SIGQUIT, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGPIPE, SIG_IGN);

  opt_parser parser;
  parser.add_long_opt("uid", &uid);
  parser.add_long_opt("key", &key);
  parser.add_long_opt("name", &name);

  if (!parser.parse_opts(argc, argv) || key.empty() || name.empty()) {
    std::ostringstream sout;
    parser.print_usage(argv[0], sout);
    cerr << sout.str() << endl;
    return -1;
  }

  LOG(INFO, "uid %" PRIu32 " from vendor %s is joining channel %s",
      uid, key.c_str(), name.c_str());

  AgoraRecorder recorder;
  recorder.CreateChannel(key, name, uid);

  while (!recorder.Stopped() && !s_stop_flag) {
    sleep(1);
  }

  if (s_stop_flag) {
    recorder.DestroyChannel();
  }

  return 0;
}

