#include "xcoder/engine_callback.h"

#include <cstring>
#include <iostream>

#include "base/time_util.h"
#include "xcoder/aac_encoder.h"

namespace agora {
namespace recording {

using std::vector;
using std::string;

class StreamObserver::PeerStream : public AgoraRTC::ICMFile {
 public:
  PeerStream(rtmp_manager &mgr, unsigned int uid);
  ~PeerStream();

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
  rtmp_manager &mgr_;
  unsigned int uid_;
  HANDLE_AACENCODER encoder_;
};

StreamObserver::PeerStream::PeerStream(rtmp_manager &mgr, unsigned int uid)
    :mgr_(mgr), uid_(uid) {
  encoder_ = NULL;
}

StreamObserver::PeerStream::~PeerStream() {
  if (encoder_) {
    aac_encode_free(encoder_);
  }
}

int StreamObserver::PeerStream::startAudioRecord() {
  return 0;
}

int StreamObserver::PeerStream::startVideoRecord() {
  return 0;
}

int StreamObserver::PeerStream::stopAudioRecord() {
  return 0;
}

int StreamObserver::PeerStream::stopVideoRecord() {
  return 0;
}

int StreamObserver::PeerStream::setVideoRotation(int rotation) {
  (void)rotation;
  return 0;
}

int StreamObserver::PeerStream::onDecodeVideo(uint32_t video_ts,
    uint8_t payload_type, uint8_t *buffer, uint32_t length,
    uint32_t frame_num) {
  (void)frame_num;

  const char *buf = reinterpret_cast<const char *>(buffer);

  // only record 264
  if ((payload_type & 0xFC) == 40 || payload_type == 127) {
    mgr_.add_video_packet(uid_, buf, length, video_ts);
  }

  return 0;
}

int StreamObserver::PeerStream::onEncodeVideo(uint32_t video_ts,
    uint8_t payload_type, uint8_t *buffer, uint32_t length) {
  // only record 264
  if ((payload_type & 0xFC) == 40 || payload_type == 127) {
    const char *buf = reinterpret_cast<const char *>(buffer);
    mgr_.add_video_packet(uid_, buf, length, video_ts);
  }

  return 0;
}

int StreamObserver::PeerStream::onDecodeAudio(uint32_t audio_ts,
    uint8_t payload_type, uint8_t *buffer, uint32_t length) {
  (void)audio_ts;
  (void)payload_type;
  (void)buffer;
  (void)length;

  return 0;
}

int StreamObserver::PeerStream::onEncodeAudio(uint32_t audio_ts,
    uint8_t payload_type, uint8_t *buffer, uint32_t length) {
  (void)audio_ts;
  (void)payload_type;

  if (!mgr_.is_mosaic_mode())
    return 0;

  if (encoder_ == NULL) {
    if (aac_encode_create(&encoder_) != 0)
      return -1;

    AACENC_InfoStruct info;
    aac_encode_init(encoder_, 32000, 32000, &info);
  }

  short len;
  unsigned char encoded[10240];

  if ((len = aac_encode((HANDLE_AACENCODER)encoder_, (short *)buffer,
      length / 2, encoded)) > 0) {
    const char *buf = reinterpret_cast<const char *>(encoded);
    mgr_.add_audio_packet(uid_, buf, len, audio_ts);
  }

  return 0;
}

StreamObserver::StreamObserver(const vector<string> &urls,
    rtmp_event_handler *handler) :rtmp_mgr_(urls, handler) {
}

StreamObserver::~StreamObserver() {
  // Nothing to do
}

bool StreamObserver::add_rtmp_url(const string &url) {
  return rtmp_mgr_.add_rtmp_url(url);
}

bool StreamObserver::remove_rtmp_url(const string &url) {
  return rtmp_mgr_.remove_rtmp_url(url);
}

bool StreamObserver::replace_rtmp_urls(const vector<string> &urls) {
  return rtmp_mgr_.replace_rtmp_urls(urls);
}

vector<string> StreamObserver::get_rtmp_urls() const {
  return rtmp_mgr_.get_rtmp_urls();
}

bool StreamObserver::set_mosaic_mode(bool mosaic) {
  return rtmp_mgr_.set_mosaic_mode(mosaic);
}

AgoraRTC::ICMFile* StreamObserver::GetICMFileObject(unsigned int uid) {
  std::lock_guard<std::mutex> auto_lock(lock_);
  auto it = peers_.find(uid);
  if (it == peers_.end()) {
    it = peers_.insert(std::make_pair(uid, peer_stream_t(
        new PeerStream(rtmp_mgr_, uid)))).first;
  }

  return it->second.get();
}

static void adtsDataForPacketLength(char packet[7], int16_t packetLength) {
  const int adtsLength = 7;
  // Variables Recycled by addADTStoPacket
  int profile = 2;  // AAC LC
  // 39=MediaCodecInfo.CodecProfileLevel.AACObjectELD;
  int freqIdx = 5;  // 32KHz
  int chanCfg = 1;  // MPEG-4 Audio Channel Configuration. 1 Channel front-center
  // NSUInteger fullLength = adtsLength + packetLength;
  unsigned long fullLength = adtsLength + packetLength;
  // fill in ADTS data
  packet[0] = (char)0xFF;  // 11111111    = syncword
  packet[1] = (char)0xF9;  // 1111 1 00 1  = syncword MPEG-2 Layer CRC
  packet[2] = (char)(((profile-1)<<6) + (freqIdx<<2) +(chanCfg>>2));
  packet[3] = (char)(((chanCfg&3)<<6) + (fullLength>>11));
  packet[4] = (char)((fullLength&0x7FF) >> 3);
  packet[5] = (char)(((fullLength&7)<<5) + 0x1F);
  packet[6] = (char)0xFC;
}

int StreamObserver::InsertRawAudioPacket(unsigned int uid,
    const unsigned char *payload, unsigned short size, int type,
    unsigned int ts, unsigned short seq) {
  (void)seq;
  (void)type;

  // ts = static_cast<uint32_t>(base::now_ms()) + kAdvanceInterval;

  std::unique_ptr<char[]> data;
  if (type == 77) { // hardware AAC
    data.reset(new (std::nothrow)char[size - 3 + 7]);
    adtsDataForPacketLength(data.get(), size - 3);
    memcpy(data.get() + 7, payload + 3, size - 3);
    size = size - 3 + 7;
  } else if (type == 79) { // software HEAAC
    data.reset(new (std::nothrow)char[size - 3]);
    memcpy(data.get(), payload + 3, size - 3);
    size -= 3;
  } else {
    return -1;
  }

  if (size > 0) {
    rtmp_mgr_.add_audio_packet(uid, data.get(), size, ts);
  }

  return 0;
}

}
}
