#include "xcoder/simple_audio_jitterbuffer.h"

namespace agora {
namespace xcodec {

SimpleAudioJitterBuffer::SimpleAudioJitterBuffer()
  : current_ts_(0), expected_delay_(0)
{
  fs_ = 32000;  //set fs = 32000 now
}

int SimpleAudioJitterBuffer::InsertPacket(RawAudioPacket& packet) {
  audio_packet_.insert(std::pair<unsigned int, RawAudioPacket>(packet.timeStamp, packet));
  current_ts_ = packet.timeStamp;
  //delete very old packets
  unsigned int old_ts = current_ts_ - expected_delay_ * (fs_ / 1000) - 5 * fs_;
  auto iter = audio_packet_.begin();
  while (iter != audio_packet_.end()) {
    if (iter->first > old_ts)
      break;
    delete[] iter->second.payloadData;
    iter = audio_packet_.erase(iter);
  }
  return 0;
}

int SimpleAudioJitterBuffer::GetPacket(RawAudioPacket& packet) {
  unsigned int target_ts = current_ts_ - expected_delay_ * (fs_ / 1000);
  auto iter = audio_packet_.begin();
  if (iter == audio_packet_.end()) //no packet
    return -1;
  if (iter->first > target_ts)  //no packet with ts older than target available
    return -1;
  packet = iter->second;
  audio_packet_.erase(iter);
  return 0;
}

void SimpleAudioJitterBuffer::reset() {
  audio_packet_.clear();
  current_ts_ = 0;
  expected_delay_ = 0;
}

} //recording
} //agora
