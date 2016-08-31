#pragma once
#include <netinet/in.h>
#include <cstdint>
#include <set>
#include "packer.h"

namespace agora { namespace base {

    using namespace agora::base;

    DECLARE_PACKABLE_2(voice_stream, uint32_t, room, uint32_t, user)
    DECLARE_PACKABLE_2(VoiceStream, uint32_t, cid, uint32_t, uid)
    DECLARE_PACKABLE_2(VoiceStream64, uint64_t, cid, uint32_t, uid)

    // NOTE: port should use network order
    DECLARE_PACKABLE_2_START(address_info, uint32_t, ip, uint16_t, port)
        address_info(const sockaddr_in& address)
            : ip(address.sin_addr.s_addr)
            , port(address.sin_port)
        {}
    DECLARE_STRUCT_END
    typedef std::vector<address_info> address_list;

    DECLARE_PACKABLE_2(address_info2, uint32_t, ip, std::vector<uint16_t>, ports)
    typedef std::vector<address_info2> address_list2;

    DECLARE_PACKABLE_2(address_info3, std::string, ip, std::vector<uint16_t>, ports)
    typedef std::vector<address_info3> address_list3;

    DECLARE_PACKABLE_2(address_info4, std::string, ip, uint16_t, port)
    typedef std::vector<address_info4> address_list4;

    DECLARE_PACKABLE_3(sid_address_info, uint32_t, ip, uint16_t, port, uint32_t, sid)
    typedef std::vector<sid_address_info> server_address_list;


    DECLARE_PACKABLE_3(sid_address_info2, uint32_t, ip, std::vector<uint16_t>, ports, std::vector<uint32_t>, sids)
    typedef std::vector<sid_address_info2> server_address_list2;

    DECLARE_PACKABLE_3(sid_address_info3, std::string, ip, uint16_t, port, uint32_t, sid)
    typedef std::vector<sid_address_info3> server_address_list3;

    DECLARE_PACKABLE_2(stream_end_point, uint32_t, id, address_info, address)
  typedef std::vector<stream_end_point> end_point_list;

    typedef std::map<std::string, std::string> config_map;

    DECLARE_PACKABLE_3_START(isp_service_address, std::string, isp, uint32_t, ip, uint16_t, port) \
        std::string to_string() const;
    DECLARE_STRUCT_END
    typedef std::vector<isp_service_address> isp_service_address_list;

    DECLARE_STRUCT_2(isp_ip, std::string, isp, uint32_t, ip)
    typedef std::vector<isp_ip> isp_ip_list;

    DECLARE_PACKABLE_2(address_pair, address_info, first, address_info, second)
    typedef std::vector<address_pair> router_address_list;

    typedef std::set<uint32_t>  user_id_set;
}}

