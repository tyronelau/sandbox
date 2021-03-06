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

  bool CreateChannel(const string &key, const string &name, uint32_t uid,
      bool decode_audio, bool decode_video);

  bool LeaveChannel();
  bool Destroy();

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
    recorder_->Destroy();
  }
}

bool AgoraRecorder::Stopped() const {
  return stopped_;
}

bool AgoraRecorder::Destroy() {
  if (recorder_) {
    recorder_->Destroy();
    recorder_ = NULL;
  }

  return true;
}

bool AgoraRecorder::CreateChannel(const string &key, const string &name,
    uint32_t uid, bool decode_audio, bool decode_video) {
  if ((recorder_ = agora::xcodec::Recorder::CreateRecorder(this)) == NULL)
    return false;

  return 0 == recorder_->JoinChannel(key.c_str(), name.c_str(), false,
      uid, decode_audio, decode_video, NULL, 300, 40000, 40003);
}

bool AgoraRecorder::LeaveChannel() {
  if (recorder_) {
    recorder_->LeaveChannel();
    stopped_ = true;
  }

  return true;
}

void AgoraRecorder::RecorderError(int error, const char *reason) {
  cerr << "Error: " << error << ", " << reason << endl;
  LeaveChannel();
}

void AgoraRecorder::RemoteUserJoined(unsigned uid) {
  cout << "User " << uid << " joined" << endl;
}

void AgoraRecorder::RemoteUserDropped(unsigned uid) {
  cout << "User " << uid << " dropped" << endl;
}

void AgoraRecorder::AudioFrameReceived(unsigned int uid, AudioFrame *frame) {
  (void)frame;
  static FILE *fp = fopen("temp.pcm", "wb");

  if (frame->type == agora::xcodec::kRawPCM) {
    cout << "User " << uid << ", received a raw PCM frame" << endl;
    if (uid == 0) {
      agora::xcodec::AudioPcmFrame *f = frame->frame.pcm;
      ::fwrite(&f->buf_[0], 1, f->buf_.size(), fp);
    }
  } else if (frame->type == agora::xcodec::kAAC) {
    cout << "User " << uid << ", received an AAC frame" << endl;
  }
}

void AgoraRecorder::VideoFrameReceived(unsigned int uid, VideoFrame *f) {
  if (f->type == agora::xcodec::kRawYuv) {
    agora::xcodec::VideoYuvFrame *frame = f->frame.yuv;

    cout << "User " << uid << ", received a yuv frame, width: "
        << frame->width_ << ", height: " << frame->height_ << endl;
  } else if (f->type == agora::xcodec::kH264) {
    agora::xcodec::VideoH264Frame *frame = f->frame.h264;

    cout << "User " << uid << ", received an h264 frame, timestamp: "
        << frame->frame_ms << ", frame no: " << frame->frame_num << endl;
  }
}

atomic_bool_t s_stop_flag;

void signal_handler(int signo) {
  (void)signo;

  // cerr << "Signal " << signo << endl;
  s_stop_flag = true;
}

int main(int argc, char * const argv[]) {
  uint32_t uid = 0;
  string key;
  string name;
  // bool dual = false;
  bool decode_audio = false;
  bool decode_video = false;

  s_stop_flag = false;
  signal(SIGQUIT, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGPIPE, SIG_IGN);

  opt_parser parser;
  parser.add_long_opt("uid", &uid);
  parser.add_long_opt("key", &key);
  parser.add_long_opt("name", &name);
  parser.add_long_opt("decode_audio", &decode_audio);
  parser.add_long_opt("decode_video", &decode_video);

  if (!parser.parse_opts(argc, argv) || key.empty() || name.empty()) {
    std::ostringstream sout;
    parser.print_usage(argv[0], sout);
    cerr << sout.str() << endl;
    return -1;
  }

  LOG(INFO, "uid %" PRIu32 " from vendor %s is joining channel %s",
      uid, key.c_str(), name.c_str());

  AgoraRecorder recorder;
  if (!recorder.CreateChannel(key, name, uid, decode_audio, decode_video)) {
    cerr << "Failed to create agora channel: " << name << endl;
    return -1;
  }

  while (!recorder.Stopped() && !s_stop_flag) {
    sleep(1);
  }

  if (s_stop_flag) {
    recorder.LeaveChannel();
    recorder.Destroy();
  }

  cerr << "Stopped \n";
  return 0;
}

