#pragma once

#include <cstdint>
#include <string>

#include "base/packet.h"

namespace agora {
namespace protocol {

enum message_uri {
  LEAVE_URI = 1,
  USER_JOINED_URI = 2,
  USER_DROPPED_URI = 3,
  AUDIO_FRAME_URI = 4,
  VIDEO_FRAME_URI = 5,
  RECORDER_ERROR_URI = 6,
};

DECLARE_PACKET_1(leave_packet, LEAVE_URI, int32_t, reason);

DECLARE_PACKET_1(user_joined, USER_JOINED_URI, uint32_t, uid);
DECLARE_PACKET_1(user_dropped, USER_DROPPED_URI, uint32_t, uid);

DECLARE_PACKET_6(audio_frame, AUDIO_FRAME_URI, uint32_t, uid, uint32_t,
    frame_ms, uint8_t, channels, uint8_t, bits, uint32_t, sample_rates,
    std::string, data);

DECLARE_PACKET_8(video_frame, VIDEO_FRAME_URI, uint32_t, uid, uint32_t,
    frame_ms, uint16_t, width, uint16_t, height, uint16_t, ystride,
    uint16_t, ustride, uint16_t, vstride, std::string, data);

DECLARE_PACKET_2(recorder_error, RECORDER_ERROR_URI, int32_t, error_code,
  std::string, reason);

}
}
