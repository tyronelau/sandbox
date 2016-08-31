#pragma once

#include <cstdint>

#define AGORA_MAGIC_ID 0x56415761526f6741llu

namespace agora {
namespace recorder {
struct file_version {
  uint8_t major_version;
  uint8_t minor_version;
};

struct recorder_common_header {
  uint64_t magic_id;
  file_version version;
};

struct recorder_file_header1 {
  uint64_t magic_id;
  file_version version;
  uint16_t reserved1;
  uint32_t reserved2;
  uint32_t cid;
  uint32_t uid;
  int64_t start_ms;
};

struct recorder_file_header2 {
  uint64_t magic_id;
  file_version version; // must be greater or equal than 0x02
  uint16_t data_offset;
  uint32_t uid;
  int64_t start_ms;
  char channel_name[64]; // ends with '\0'
};

union recording_file_header {
  recorder_common_header common;
  recorder_file_header1 header1;
  recorder_file_header2 header2;
};

struct audio_packet {
  int32_t ts_offset; // the subtraction of received ts to start_us in ms.
  uint16_t payload_type;
  uint16_t seq_number;
  uint32_t timestamp;
  uint16_t payload_size;
  unsigned char payload[];
} __attribute__((packed));

}
}

