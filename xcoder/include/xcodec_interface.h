#pragma once

#include <string>

namespace agora {
namespace xcodec {

typedef unsigned char uchar_t;
typedef unsigned int uint_t;

enum ErrorCode {
  kJoinOK = 0,
  kJoinFailed = 1,
};

class AudioFrame {
  friend struct Recorder;
 public:
  AudioFrame(uint_t frame_ms, uint_t sample_rates, uint_t samples);
  ~AudioFrame();
 public:
  uint_t frame_ms_;
  uint_t channels_; // 1

  uint_t sample_bits_; // 16
  uint_t sample_rates_; // 8k, 16k, 32k

  uint_t samples_;
  std::string buf_; // samples * sample_bits_ / CHAR_BIT * channels_
};

class VideoYuvFrame {
  friend class RecorderImpl;
 public:
  VideoYuvFrame(uint_t frame_ms, uint_t width, uint_t height, uint_t ystride,
      uint_t ustride, uint_t vstride);

  ~VideoYuvFrame();
 public:
  uint_t frame_ms_;

  uchar_t *ybuf_;
  uchar_t *ubuf_;
  uchar_t *vbuf_;

  uint_t width_;
  uint_t height_;

  uint_t ystride_;
  uint_t ustride_;
  uint_t vstride_;
 private:
  std::string data_;
};

struct VideoH264Frame {
  uint_t frame_ms;
  uint_t frame_num;
  std::string payload;
};

enum FrameType {
  kRawYuv = 0,
  kH264 = 1,
};

struct VideoFrame {
  FrameType type;
  union {
    VideoYuvFrame *yuv;
    VideoH264Frame *h264;
  } frame;
};

struct RecorderCallback {
  virtual ~RecorderCallback() {}

  virtual void RecorderError(int error, const char *reason) = 0;

  virtual void RemoteUserJoined(unsigned int uid) = 0;
  virtual void RemoteUserDropped(unsigned int uid) = 0;

  virtual void AudioFrameReceived(unsigned int uid, AudioFrame *frame) = 0;

  // For performance concerns, it is safe to take the ownership of real
  // payload contained in |*frame|.
  virtual void VideoFrameReceived(unsigned int uid, VideoFrame *frame) = 0;
};

struct Recorder {
  static Recorder* CreateRecorder(RecorderCallback *callback);

  virtual ~Recorder() {}

  virtual int JoinChannel(const char *app_id, const char *channel_name,
      bool is_dual=false, uint_t uid=0, const char *path_prefix=NULL) = 0;

  virtual int LeaveChannel() = 0;

  // should be called after |LeaveChannel|
  // NOTE: DO NOT call |Destroy| in |RecorderCallback|.
  virtual int Destroy() = 0;
};

}
}
