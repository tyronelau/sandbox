#pragma once

#include <string>

namespace agora {
namespace xcodec {

typedef unsigned char uchar_t;
typedef unsigned int uint_t;

enum ErrorCode {
  kJoinOK = 0,
  kJoinFailed = 1,
  kInvalidArgument = 2,
};

enum AudioFrameType {
  kRawPCM = 0,
  kAAC = 1,
};

enum VideoFrameType {
  kRawYuv = 0,
  kH264 = 1,
};

class AudioPcmFrame {
  friend struct Recorder;
 public:
  AudioPcmFrame(uint_t frame_ms, uint_t sample_rates, uint_t samples);
  ~AudioPcmFrame();
 public:
  uint_t frame_ms_;
  uint_t channels_; // 1

  uint_t sample_bits_; // 16
  uint_t sample_rates_; // 8k, 16k, 32k

  uint_t samples_;
  std::string buf_; // samples * sample_bits_ / CHAR_BIT * channels_
};

class AudioAacFrame {
 public:
  explicit AudioAacFrame(uint_t frame_ms);
  ~AudioAacFrame();
 public:
  uint_t frame_ms_;
  std::string buf_;
};

struct AudioFrame {
  AudioFrameType type;
  union {
    AudioPcmFrame *pcm;
    AudioAacFrame *aac;
  } frame;
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

struct VideoFrame {
  VideoFrameType type;
  union {
    VideoYuvFrame *yuv;
    VideoH264Frame *h264;
  } frame;

  int rotation; // 0, 90, 180, 270
};

struct RecorderCallback {
  virtual ~RecorderCallback() {}

  virtual void RecorderError(int error, const char *reason) = 0;

  virtual void RemoteUserJoined(unsigned int uid) = 0;
  virtual void RemoteUserDropped(unsigned int uid) = 0;

  // For performance concerns, it is safe to take the ownership of real
  // payload contained in |*frame|.
  virtual void AudioFrameReceived(unsigned int uid, AudioFrame *frame) = 0;

  // For performance concerns, it is safe to take the ownership of real
  // payload contained in |*frame|.
  virtual void VideoFrameReceived(unsigned int uid, VideoFrame *frame) = 0;
};

struct Recorder {
  static Recorder* CreateRecorder(RecorderCallback *callback);

  virtual ~Recorder() {}

  // |idle|: If the channel has no broadcasters over |idle| seconds,
  // the xcoder will automatically quit, and you will receive a
  // |RecorderError| callback.
  //
  // If |udp_port_low|, |udp_port_high| are both given positive numbers,
  // all udp ports used in agora xcoding services are allocated from
  // [udp_port_low, udp_port_high).
  // NOTE:
  // The range [udp_port_low, udp_port_high) should contain AT LEAST 3
  // available ports.
  virtual int JoinChannel(const char *app_id, const char *channel_name,
      bool is_dual=false, uint_t uid=0, bool decode_audio=false,
      bool decode_video=false, const char *path_prefix=NULL, int idle=300,
      int udp_port_low=0, int udp_port_high=0) = 0;

  virtual int LeaveChannel() = 0;

  // should be called after |LeaveChannel|
  // NOTE: DO NOT call |Destroy| in |RecorderCallback|.
  virtual int Destroy() = 0;
};

}
}
