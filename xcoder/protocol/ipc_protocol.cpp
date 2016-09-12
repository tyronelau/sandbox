#include "protocol/ipc_protocol.h"

namespace agora {
namespace protocol {

leave_packet::leave_packet() : packet(sizeof(*this), LEAVE_PROTOCOL, 0) {
}

}
}
