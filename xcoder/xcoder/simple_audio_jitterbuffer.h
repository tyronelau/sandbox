#pragma once

#include <map>

namespace agora {
namespace xcodec {

struct RawAudioPacket {
  char* payloadData;
  short payloadSize;
  int payloadType;
  unsigned int timeStamp;
  unsigned short seqNumber;
};

class SimpleAudioJitterBuffer {
public:
  SimpleAudioJitterBuffer();
  int InsertPacket(RawAudioPacket& packet);
  int GetPacket(RawAudioPacket& packet);
  void SetExpectedDelay(int delay_ms) { expected_delay_ = delay_ms; }
  void reset();
private:
  std::map<unsigned int, RawAudioPacket> audio_packet_;
  unsigned int current_ts_;
  unsigned int expected_delay_;
  unsigned int fs_;
};

} //recording
} //agora
