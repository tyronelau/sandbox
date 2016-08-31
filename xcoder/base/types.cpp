#include "types.h"
#include "utils.h"
using namespace agora::base;

std::string isp_service_address::to_string() const
{
    return "isp: " + isp + " address: " + address_to_string(ip, port);
}
