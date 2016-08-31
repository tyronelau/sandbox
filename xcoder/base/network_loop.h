#pragma once

#include <event2/event.h>
# include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <event2/http.h>

#include <set>
#include <utility>
#include "packet.h"
#include "utils.h"

namespace agora {
    namespace base {

        class bind_listener {
        public:
            virtual ~bind_listener() {}
            virtual void on_accept(evconnlistener *listener, evutil_socket_t fd, sockaddr *address, int socklen) = 0;
        };

        class connect_listener {
        public:
            virtual ~connect_listener() {}
            virtual void on_connect(bufferevent *bev, short events) = 0;
        };

        class tcp_listener {
        public:
            virtual ~tcp_listener() {}
            virtual void on_read(bufferevent *bev) = 0;
            virtual void on_write(bufferevent */*bev*/) {}
            virtual void on_event(bufferevent *bev, short events) = 0;
        };

        class udp_listener {
        public:
            virtual ~udp_listener() {}
            virtual void on_data(evutil_socket_t s) = 0;
        };

        class http_listener {
        public:
            virtual ~http_listener() {}
            virtual void on_request(struct evhttp_request* req) = 0;
        };

        class timer_listener {
        public:
            virtual ~timer_listener() {}
            virtual void on_timer() = 0;
        };


        class network_loop
            : private noncopyable
        {
            typedef std::set<event*> timer_set;
        public:
            explicit network_loop(bool thread_safe=false);
            ~network_loop();

        public:
            void run();
            void stop(const timeval *tv);
            event* add_timer(uint64_t us, timer_listener * listener);
            void remove_timer(event* timer);
            ::event_base* event_base() { return event_base_; }

            event* udp_bind(uint16_t & port, size_t tries, udp_listener * listener, uint32_t ip=0);
            event* udp_bind_ipv6(const sockaddr_storage& addr, size_t tries, udp_listener * listener);
            evconnlistener* tcp_bind(uint16_t port, bind_listener * listener, uint32_t ip=0);
            evhttp* http_bind(uint16_t port, http_listener* listener, uint32_t ip=0);
            bufferevent * tcp_connect(const sockaddr_in & sin, connect_listener * listener);
            bufferevent * tcp_connect(uint32_t ip, uint16_t port, connect_listener * listener);
            bufferevent * set_tcp_listener(evutil_socket_t fd, tcp_listener * arg);
            void set_tcp_listener(bufferevent * bev, tcp_listener * listener);
            void set_read_timeout(bufferevent *bev, uint32_t second);
            void set_write_timeout(bufferevent *bev, uint32_t second);

            bool sendto(int linkid, const sockaddr_in & addr, const char* data, int length);
            bool sendto(int linkid, const sockaddr_storage& addr, const char* data, int length);
            void send_buffer(bufferevent *bev, const char* data, int length);
            void send_message(bufferevent *bev, const agora::base::packet & p);
            void send_message(bufferevent *bev, const agora::base::unpacker & p);
            void http_reply(evhttp_request* req, const char* data, int length, int code, const char *reason);

            void close(event * e);  // close the event & socket created by udp bind
            void close(bufferevent * bev);
            void http_end(evhttp_request* req);
            static void set_free_bev_on_error(bool free) { free_bev_on_error_ = free; }

        private:
            static void udp_event_callback(evutil_socket_t fd, short event, void* context);
            static void timer_callback(evutil_socket_t fd, short event, void* context);
            static void accept_callback(evconnlistener *listener, evutil_socket_t fd, sockaddr *address, int socklen, void* context);
            static void connect_callback(bufferevent *bev, short events, void *context);
            static void tcp_read_callback(bufferevent *bev, void* context);
            static void tcp_write_callback(bufferevent *bev, void* context);
            static void tcp_event_callback(bufferevent *bev, short events, void* context);
            static void http_request_callback(evhttp_request *req, void* context);

        private:
            ::event_base* event_base_;
            timer_set timers_;
            uint32_t options_;
            static bool free_bev_on_error_;
        };
    }
}
