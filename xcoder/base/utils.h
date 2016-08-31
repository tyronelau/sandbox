#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <climits>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>


#include <string>
#include <vector>
#include <memory.h>
#include <cstdlib>
#include <sstream>
#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>

#include <event2/bufferevent.h>

#include "types.h"
#include <memory>

namespace agora { namespace base {

        inline std::string stringToHex(const std::string& in)
        {
            static const char hexTable[]= "0123456789abcdef";

            if (in.empty()) {
                return std::string();
            }
            std::string out(in.size()*2, '\0');
            for (uint32_t i = 0; i < in.size(); ++i){
                out[i*2 + 0] = hexTable[(in[i] >> 4) & 0x0F];
                out[i*2 + 1] = hexTable[(in[i]     ) & 0x0F];
            }
            return out;
        }

        template <class CharContainer>
        inline std::string show_hex(const CharContainer& c)
        {
            std::string hex;
            char buf[16];
            for (auto& i: c) {
                std::sprintf(buf, "%02X ", static_cast<unsigned>(i) & 0xFF);
                hex += buf;
            }
            return hex;
        }

        inline std::vector<std::string> split(const std::string &s, char delim) {
            std::vector<std::string> elems;
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, delim)) {
                elems.push_back(item);
            }
            return elems;
        }

        inline std::string local_ipv4()
        {
            int fd;
            struct ifreq ifr;

            fd = socket(AF_INET, SOCK_DGRAM, 0);
            ifr.ifr_addr.sa_family = AF_INET;               // IPv4 IP address
            strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);      // attached to "eth0"
            ioctl(fd, SIOCGIFADDR, &ifr);
            close(fd);

            return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
        }

        template<typename _Ty>
        std::ostream& operator<<(std::ostream& _This, const std::vector<_Ty>& _Vector)
        {
            _This << "[";
            for (const auto& i: _Vector) {
                _This << i << ",";
            }
            return _This << "]";
        }

        template<typename _Ty1, typename _Ty2>
        inline std::ostream& operator<<(std::ostream &_This, const std::pair<_Ty1, _Ty2>& _Pair)
        {
            return _This << _Pair.first << ":" << _Pair.second;
        }

        inline std::ostream& operator <<(std::ostream& oss, const voice_stream& stream)
        {
            return oss << "stream " << stream.room << " " << stream.user;
        }

        template<typename T>
        inline std::string make_string(const T & c, const std::string& separator)
        {
            std::ostringstream oss;
            std::string s = "";
            for (const auto & v: c) {
                oss << s << v;
                s = separator;
            }

            return oss.str();
        }

        template<typename T, typename F>
        inline std::string make_string(const T & c, const std::string& separator, F f)
        {
            std::ostringstream oss;
            std::string s = "";
            for (const auto & v: c) {
                oss << s << f(v);
                s = separator;;
            }

            return oss.str();
        }

        template<typename T>
        inline std::string make_string(const T & c, size_t max_count, const std::string& separator)
        {
            std::ostringstream oss;
            std::string s = "";
            size_t count = 0;
            for (const auto & v: c) {
                if (count >= max_count) {
                    break;
                }
                oss << s << v;
                s = separator;
                ++count;
            }

            if (c.size() > count) {
                oss << " ...";
            }

            return oss.str();
        }

        template<typename T>
        inline bool value_in_range(T v, T ref, T left, T right)
        {
            if (ref - v <= left)
            {
                return true;
            }

            if ( v - ref >= right)
            {
                return true;
            }

            return false;
        }

    inline uint64_t now_us()
    {
            timeval t = {0, 0};
      ::gettimeofday(&t, NULL);
      return (uint64_t)(t.tv_sec) * 1000 * 1000 + t.tv_usec;
    }
        inline uint64_t now_ms()
        {
            return now_us() / 1000;
        }

    inline uint32_t now_seconds()
    {
      uint64_t us = now_us();
      return static_cast<uint32_t>(us / 1000 / 1000);
    }

        inline uint32_t random(uint32_t max)
        {
            static bool seeded = false;
            if (!seeded) {
                srand(now_seconds());
                seeded = true;
            }
            return rand() % max;
        }

        inline uint32_t ip_to_int(const std::string& ip)
        {
            struct sockaddr_in ip_addr;
            inet_aton(ip.c_str(), &ip_addr.sin_addr);
            return ip_addr.sin_addr.s_addr;
        }

        inline std::vector<uint8_t> ipv4_to_vector(const std::string& ip)
        {
            std::vector<uint8_t> v(4);
            struct sockaddr_in ip_addr;
            inet_aton(ip.c_str(), &ip_addr.sin_addr);
            memcpy(v.data(), &ip_addr.sin_addr.s_addr, sizeof(ip_addr.sin_addr.s_addr));
            return v;
        }

        inline std::vector<uint8_t> ipv6_to_vector(const std::string& ip)
        {
            std::vector<uint8_t> v(16);
            struct sockaddr_in6 ip_addr;
            inet_pton(AF_INET6, ip.c_str(), &ip_addr.sin6_addr);
            memcpy(v.data(), &ip_addr.sin6_addr.s6_addr, sizeof(ip_addr.sin6_addr.s6_addr));
            return v;
        }
        inline std::vector<uint8_t> ip_to_vector(const std::string& ip){
            if(ip.size() >15 && ip.find(':') != std::string::npos) {
                return ipv6_to_vector(ip);
            }else{
                return ipv4_to_vector(ip);
            }
        }


        /*inline std::string ip_to_string(const std::vector<uint8_t> &ip)*/
        /*{*/
        /*std::stringstream ss;*/
        /*for(auto &x:ip){*/
        /*ss<<x<<":";*/
        /*}*/
        /*return ss.str();*/
        /*}*/

    inline std::string ip_to_string(uint32_t ip)
    {
      in_addr ip_addr = { 0 };
      ip_addr.s_addr = ip;
      return inet_ntoa(ip_addr);
    }

        inline sockaddr_in to_address(uint32_t ip, uint16_t port) {
            sockaddr_in address;
            ::memset(&address, 0, sizeof(address));
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = ip;
            address.sin_port = port;
            return address;
        }

        inline std::string ip_to_string(const sockaddr_in6& peer_addr){
            char str[INET6_ADDRSTRLEN]={0};
            inet_ntop(AF_INET6,  &(peer_addr.sin6_addr), (char*)(str), INET6_ADDRSTRLEN);
            return std::string(str);
        }

        inline std::string ip_to_string(const sockaddr_in& peer_addr){
            char str[INET_ADDRSTRLEN]={0};
            inet_ntop(AF_INET,  &(peer_addr.sin_addr), (char*)(str), INET_ADDRSTRLEN);
            return str;
        }

        inline std::string ip_to_string(const sockaddr_storage& peer_addr){
            if(peer_addr.ss_family == AF_INET6) {
                return ip_to_string(*((sockaddr_in6*)&peer_addr));
            }else if(peer_addr.ss_family == AF_INET){
                return ip_to_string(*((sockaddr_in*)&peer_addr));
            }else{
                return "";
            }
        }
        inline std::string address_to_string(const sockaddr_in6& peer_addr)
    {
            std::stringstream ss;
            char str[INET6_ADDRSTRLEN]={0};
            inet_ntop(AF_INET6,  &(peer_addr.sin6_addr), (char*)(str), INET6_ADDRSTRLEN);
            ss<<std::string(str)<<":"<<ntohs(peer_addr.sin6_port);
            return ss.str();
    }

    inline std::string address_to_string(const sockaddr_in& peer_addr)
    {
            char buffer[32];
            sprintf(buffer, "%s:%u", ::inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
      return std::string(buffer);
    }

        inline uint32_t address_to_ip(const sockaddr_in& addr)
        {
            return addr.sin_addr.s_addr;
        }

        inline std::vector<uint8_t > address_to_ip(const sockaddr_in6& addr)
        {
            std::vector<uint8_t > ip((uint8_t*)addr.sin6_addr.s6_addr, (uint8_t*)(addr.sin6_addr.s6_addr+16));
            return ip;
        }
        inline uint32_t address_to_ip(const sockaddr_storage& addr)
        {
            if(addr.ss_family == AF_INET) {
                return address_to_ip(*(sockaddr_in*)&addr);
            }else if(addr.ss_family == AF_INET6 ){
                auto a(std::move(address_to_ip(*(sockaddr_in6*)&addr)));
                uint32_t ret=0;
                memcpy(&ret,a.data()+12, sizeof(ret));
                return ret;
            }else {
                return 0;
            }
        }
        inline std::string address_to_string(const sockaddr_storage& peer_addr)
    {
            if(peer_addr.ss_family == AF_INET) {
                return address_to_string((*(sockaddr_in*)&peer_addr));
            }else if(peer_addr.ss_family == AF_INET6 ){
                return address_to_string((*(sockaddr_in6*)&peer_addr));
            } else{
                return "";
            }
        }
        inline bool address_equals(const sockaddr_in6 & l, const sockaddr_in6 & r)
    {
      if (!IN6_ARE_ADDR_EQUAL(l.sin6_addr.s6_addr , r.sin6_addr.s6_addr)) return false;
      if (l.sin6_port != r.sin6_port) return false;
      return true;
    }

    inline bool address_equals(const sockaddr_in & l, const sockaddr_in & r)
    {
      if (l.sin_addr.s_addr != r.sin_addr.s_addr) return false;
      if (l.sin_port != r.sin_port) return false;
      return true;
    }

        inline bool address_equals(const sockaddr_storage& l, const sockaddr_storage& r)
        {
            if (l.ss_family ==AF_INET && r.ss_family == AF_INET) {
                return address_equals((*(sockaddr_in*)&l), (*(sockaddr_in*)&r));
            } else if (l.ss_family == AF_INET6 && r.ss_family == AF_INET6) {
                return address_equals((*(sockaddr_in6*)&l), (*(sockaddr_in6*)&r));
            }
            return false;
        }
    inline std::string address_to_string(uint32_t ip, uint16_t port)
    {
            sockaddr_in addr = {AF_INET, 0, {0}, {0}};
      addr.sin_addr.s_addr = ip;
      addr.sin_port = port;
      return address_to_string(addr);
    }

        inline std::string address_to_string(const std::string &ip, const uint16_t &port)
    {
            std::stringstream ss;
            ss<<ip<<":"<<ntohs(port);
      return ss.str();
    }

        /*inline std::string address_to_string(const std::vector<uint8_t> &ip, const uint16_t &port)*/
        /*{*/
        /*std::stringstream ss;*/
        /*for(auto i=0;i<16;i++){*/
        /*ss<<ip[i]<<":";*/
        /*}*/
        /*ss<<ntohs(port);*/
        /*return ss.str();*/
        /*}*/


        inline std::string address_to_string(const address_info& addr)
        {
            return address_to_string(addr.ip, addr.port);
        }

        inline std::string address_to_string(const sid_address_info2 &addr)
        {
            std::stringstream ss;
            ss << ip_to_string(addr.ip);

            for (uint16_t port : addr.ports) {
                ss << ":" << static_cast<uint32_t>(port);
            }

            std::string addr_str;
            ss >> addr_str;
            return addr_str;
        }

        inline std::string to_string(const address_info& address)
        {
            return address_to_string(address);
        }
        inline sockaddr_in fd_to_address(int s)
        {
            sockaddr_in address = {AF_INET, 0, {0}, {0}};
            socklen_t length = sizeof(address);
            ::getpeername(s, (struct sockaddr*)&address, &length);
            return address;
        }
        inline sockaddr_storage fd_to_address2(int s)
    {
            sockaddr_storage address =  {0, 0, {0}};
      socklen_t length = sizeof(address);
      ::getpeername(s, (struct sockaddr*)&address, &length);
      return address;
    }
        inline std::string bev_to_address(bufferevent* bev)
        {
            return address_to_string(fd_to_address(bufferevent_getfd(bev)));
        }
    inline std::vector<uint32_t> resolve_host(const std::string & host)
    {
      std::vector<uint32_t> ips;

      hostent * he = ::gethostbyname(host.c_str());
      if (he == NULL) {
        return ips;
      }
      int i = 0;
      while (he->h_addr_list[i] != NULL)
      {
        ips.push_back(*(uint32_t*)he->h_addr_list[i]);
        ++i;
      }
      return ips;
    }

    inline void daemonize(void) {
      int pid = fork();
      if (pid!= 0) {
        printf("daemon pid %u\n", pid);
        exit(0); // parent exits
      }
      umask(0);
      setsid(); // create a new session. get a new pid for this child process
            if (chdir("/tmp") == -1)
            {
                printf("change dir to /tmp failed");
                exit(1);
            }
      // Every output goes to /dev/null.
      int fd = -1;
      if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO) close(fd);
      }
    }

        template <class T>
        class singleton
        {
        public:
            static T* instance()
            {
                static T inst;
                return &inst;
            }
        protected:
            singleton(){}
            virtual ~singleton(){}
        private:
            singleton(const singleton&);
            singleton& operator = (const singleton& rhs);
        };

        class noncopyable
        {
        protected:
            noncopyable() {}
            ~noncopyable() {}
        private:
            noncopyable(const noncopyable&) = delete;
            noncopyable& operator=(const noncopyable&) = delete;
        };

        template <typename T>
        class holdon
        {
            typedef std::map<T, uint32_t> container;
            typedef typename container::iterator container_it;
            typedef std::multimap<uint32_t, container_it> holdon_queue;

        public:
            std::pair<int, bool> exists(const T &task) const
            {
                auto it = container_.find(task);
                if (it == container_.end()) {
                    return std::make_pair(0u, false);
                }
                return std::make_pair(it->second - now_seconds(), true);
            }
            std::pair<int, bool> insert(const T &done_task, uint32_t holdon_second)
            {
                uint32_t now = now_seconds();
                uint32_t when_expire = now + holdon_second;
                std::pair<container_it, bool> result = container_.insert(std::make_pair(done_task, when_expire));

                int left = result.first->second - now;
                if (!result.second) {
                    return std::make_pair(left, false);
                }
                queue_.insert(std::make_pair(when_expire, result.first));
                return std::make_pair(left, true);

            }
            std::pair<int, uint32_t> cleanup(uint32_t now_second)
            {
                uint32_t before = queue_.size();
                for (typename holdon_queue::iterator it = queue_.begin(); it != queue_.end();) {
                    if (it->first > now_second) {
                        break;
                    }
                    container_.erase(it->second);
                    it = queue_.erase(it);
                }
                uint32_t after = queue_.size();
                return std::make_pair(before - after, after);
            }
        private:
            container container_;
            holdon_queue queue_;
        };

        class recycle_bin : private noncopyable
        {
        public:
            DECLARE_STRUCT_2(deadline, uint32_t, when_it_dropped, uint32_t, when_to_destroy)
            typedef std::unordered_map<uint32_t, deadline> cabinet;

            void drop(uint32_t cid, uint32_t now_second, uint32_t when_to_destroy);
            cabinet destroy(uint32_t now_second);
            bool cancle(uint32_t cid, uint32_t now_second);

            cabinet m_;  // key: cid; value: when_to_destroy
        };

        struct utils
        {
            static isp_ip_list parse_isp_ip(const std::string & s);
            static uint64_t now_ms();
            template<typename T>
            static std::string pack_string(const T& p)
            {
                packer pk;
                pk << p;
                return pk.pack().body();
            }

            static bool less(uint16_t l, uint16_t r)
            {
                return ((uint16_t)(l - r)) > SHRT_MAX;
            }
        };
  }
}

