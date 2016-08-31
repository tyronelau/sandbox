#pragma once

#include <ifaddrs.h>
#include <event2/bufferevent.h>
#include "utils.h"
#include "algorithm.h"

namespace agora { namespace base {

    struct sockaddr_in_hasher
    {
        size_t operator() (const sockaddr_in& a) const {
            union {
                uint64_t n;
                struct {
                    uint32_t high;
                    uint32_t low;
                } bits;
            };

            bits.high = a.sin_addr.s_addr;
            bits.low = a.sin_port;
            return hasher_(n);
        }
    private:
        std::hash<uint64_t> hasher_;
    };

    struct sockaddr_in_equal {
        bool operator()(const sockaddr_in& l, const sockaddr_in& r) const
        {
            if (l.sin_addr.s_addr != r.sin_addr.s_addr) {
                return false;
            }

            return l.sin_port == r.sin_port;
        }
    };

    struct network_helper
    {
        static inline sockaddr_in bufferevent_get_address(bufferevent* bev)
        {
            evutil_socket_t fd = bufferevent_getfd(bev);
            return fd_to_address(fd);
        }

        static inline uint32_t bufferevent_get_ipv4(bufferevent* bev)
        {
            return address_to_ip(bufferevent_get_address(bev));
        }

        static inline sockaddr_storage bufferevent_get_address2(bufferevent* bev)
        {
            evutil_socket_t fd = bufferevent_getfd(bev);
            return fd_to_address2(fd);
        }

        static inline std::string bufferevent_address_string(bufferevent* bev)
        {
            return address_to_string(bufferevent_get_address(bev));
        }

        static inline std::string to_string(bufferevent* bev)
        {
            return address_to_string(bufferevent_get_address(bev));
        }
        static inline sockaddr_storage empty_common_address()
        {
            return {0, 0, {0}};
        }
        static inline sockaddr_in empty_ipv4_address()
        {
            return {AF_INET, 0, {0}, {0}};
        }
        static inline sockaddr_in6 empty_ipv6_address()
        {
            return {AF_INET6, 0, 0, IN6ADDR_ANY_INIT ,0};
        }

        static bool is_public_ip(uint32_t addr)
        {
            //A: 10.0.0.0 -- 10.255.255.255
            //   127.0.0.0 --127.255.255.255
            //B: 172.16.0.0--172.31.255.255
            //   169.254.0.0--169.254.255.255
            //C: 192.168.0.0--192.168.255.255
            //D: 100.64.0.0 -- 100.127.255.255
            /*
             *                                   10(A)          127(A)          172(B)          169(B)          192.168(C)       100.64(D)
             *                                   10.0.0.0       127.0.0.0       172.16.0.0      169.254.0.0     192.168.0.0      100.64.0.0
             *                                   10.255.255.255 127.255.255.255 172.31.255.255  169.254.255.255 192.168.255.255  100.127.255.255
             */
            static const uint32_t addrbegin[] = {0x0a000000,    0x7f000000,     0xac100000,     0xa9fe0000,     0xc0a80000,      0x64400000};
            static const uint32_t addrend[]   = {0x0affffff,    0x7fffffff,     0xac1fffff,     0xa9feffff,     0xc0a8ffff,      0x647fffff};

            for (size_t i = 0; i < sizeof(addrbegin)/sizeof(unsigned long); i++) {
                if (addr >= addrbegin[i] && addr <= addrend[i]) {
                    return false;
                }
            }
            return true;
        }

        static inline std::vector<uint32_t> local_public_addresses()
        {
            std::vector<uint32_t> ips = local_addresses();
            retain(ips, [&](uint32_t ip){
                return is_public_ip(ntohl(ip));
            });
            return ips;
        }

        static inline std::vector<uint32_t> local_addresses()
        {
            std::vector<uint32_t> ips;

            struct ifaddrs * if_address = nullptr;
            for (getifaddrs(&if_address); if_address != nullptr; if_address = if_address->ifa_next) {
                if ((if_address->ifa_addr != nullptr) && (if_address->ifa_addr->sa_family == AF_INET)) { // check it is IP4
                    uint32_t ip = ((struct sockaddr_in *)if_address->ifa_addr)->sin_addr.s_addr;
                    if (ip != htonl(INADDR_LOOPBACK)) {
                        ips.push_back(ip);
                    }
                }
            }
            if (if_address != nullptr) {
                freeifaddrs(if_address);
                if_address = nullptr;
            }
            return ips;
        }

        static inline bool is_global_ipv6(struct in6_addr & s){
            uint32_t x=0;
            memcpy(&x, s.s6_addr, sizeof(x));
            return ((x & htonl(0x70000000))  == htonl (0x20000000));
        }

        static inline std::vector<sockaddr_in6> local_public_addresses_ipv6()
        {
            std::vector<sockaddr_in6> ips;
            struct ifaddrs * if_address = nullptr;
            /*struct in6_addr invalid=IN6ADDR_LOOPBACK_INIT;*/
            for (getifaddrs(&if_address); if_address != nullptr; if_address = if_address->ifa_next) {
                if((if_address->ifa_addr != nullptr) && (if_address->ifa_addr->sa_family == AF_INET6)){
                    auto &a=((struct sockaddr_in6 *)if_address->ifa_addr)->sin6_addr;
                    if(is_global_ipv6(a)) { // check it is global IP6
                        ips.push_back(*(struct sockaddr_in6 *)if_address->ifa_addr);
                    }
                }
            }
            if (if_address != nullptr) {
                freeifaddrs(if_address);
                if_address = nullptr;
            }
            return ips;
        }
        static inline std::vector<sockaddr_in6> local_addresses_ipv6()
        {
            std::vector<sockaddr_in6> ips;
            struct ifaddrs * if_address = nullptr;
            struct in6_addr invalid=IN6ADDR_LOOPBACK_INIT;
            for (getifaddrs(&if_address); if_address != nullptr; if_address = if_address->ifa_next) {
                if ((if_address->ifa_addr != nullptr) && (if_address->ifa_addr->sa_family == AF_INET6)) { // check it is IP6
                    auto &a=*((struct sockaddr_in6 *)if_address->ifa_addr);
                    if(!IN6_ARE_ADDR_EQUAL(&invalid,&a)){
                        //std::cout<<address_to_string(*((struct sockaddr_in6 *)if_address->ifa_addr))<<std::endl;
                        ips.push_back(*(struct sockaddr_in6 *)if_address->ifa_addr);
                    }
                }
            }
            if (if_address != nullptr) {
                freeifaddrs(if_address);
                if_address = nullptr;
            }
            return ips;
        }

        static inline std::string ipv6_to_string(const std::vector<uint8_t>& ipv6) {
            sockaddr_in6 address = empty_ipv6_address();
            ::memcpy(&address.sin6_addr, ipv6.data(), ipv6.size());
            return agora::base::ip_to_string(address);
        }

        static inline std::string ipv4_to_string(const std::vector<uint8_t>& ipv4) {
            uint32_t ip = 0;
            ::memcpy(&ip, ipv4.data(), ipv4.size());
            return agora::base::ip_to_string(ip);
        }

        static inline std::string ip_to_string(const std::vector<uint8_t>& ip) {
            if (ip.size() == 4) {
                return ipv4_to_string(ip);
            }
            if (ip.size() == 16) {
                return ipv6_to_string(ip);
            }
            return "0.0.0.0";
        }

    };
} }
