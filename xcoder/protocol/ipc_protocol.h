#pragma once

namespace agora {
namespace protocol {

struct packet_common_header {
  uint32_t size;
  uint16_t uri;
  uint16_t reserved; 
};

struct packet {
  packet_common_header header;
  packet(uint32_t size, uint16_t uri, uint16_t reserved=0);
};

class leave_packet : public packet {
 public:
  leave_packet();
  ~leave_packet();
 private:
};

}
}
