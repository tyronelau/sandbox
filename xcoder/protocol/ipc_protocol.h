#pragma once

#include <cstdint>
#include <string>
#include <iostream>

#include "base/packet.h"

namespace agora {
namespace protocol {

enum message_uri {
  LEAVE_URI = 1,
  USER_JOINED_URI = 2,
  USER_DROPPED_URI = 3,
  PCM_FRAME_URI = 4,
  YUV_FRAME_URI = 5,
  RECORDER_ERROR_URI = 6,
  H264_FRAME_URI = 7,
  AAC_FRAME_URI = 8,
};

DECLARE_PACKET_1(leave_packet, LEAVE_URI, int32_t, reason);

DECLARE_PACKET_1(user_joined, USER_JOINED_URI, uint32_t, uid);
DECLARE_PACKET_1(user_dropped, USER_DROPPED_URI, uint32_t, uid);

DECLARE_PACKET_6(pcm_frame, PCM_FRAME_URI, uint32_t, uid, uint32_t,
    frame_ms, uint8_t, channels, uint8_t, bits, uint32_t, sample_rates,
    std::string, data);

DECLARE_PACKET_3(aac_frame, AAC_FRAME_URI, uint32_t, uid, uint32_t,
    frame_ms, std::string, data);

DECLARE_PACKET_8(yuv_frame, YUV_FRAME_URI, uint32_t, uid, uint32_t,
    frame_ms, uint16_t, width, uint16_t, height, uint16_t, ystride,
    uint16_t, ustride, uint16_t, vstride, std::string, data);

DECLARE_PACKET_4(h264_frame, H264_FRAME_URI, uint32_t, uid, uint32_t,
    frame_ms, uint32_t, frame_num, std::string, data);

DECLARE_PACKET_2(recorder_error, RECORDER_ERROR_URI, int32_t, error_code,
  std::string, reason);

// struct audio_frame : base::packet {
//   uint32_t uid;
//   uint32_t frame_ms;
//   uint8_t channels;
//   uint8_t bits;
//   uint32_t sample_rates;
//   uint32_t data;
//   // std::string data;
//
//   audio_frame() : packet(AUDIO_FRAME_URI) {
//     std::cout << "\naudio : " << this << std::endl;
//   }
//
//   ~audio_frame() {
//     std::cout << "\n~audio : " << this << std::endl;
//   }
//
//   virtual void unmarshall(base::unpacker &p) {
//     packet::unmarshall(p);
//     p >> uid >> frame_ms >> channels >> bits >> sample_rates >> data;
//   }
//
//   virtual void marshall(base::packer &p) const {
//     packet::marshall(p);
//     p << uid << frame_ms << channels << bits << sample_rates << data;
//   }
// };

}
}
