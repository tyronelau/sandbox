#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <event2/thread.h>
#include "network_loop.h"
#include "network_helper.h"
#include "log.h"
#include "utils.h"
#include "packet.h"
#include "packer.h"
#include <cerrno>
using namespace agora::base;

bool network_loop::free_bev_on_error_ = true;

network_loop::network_loop(bool thread_safe)
{
#ifdef WIN32
  WSADATA WSAData;
  WSAStartup(0x101, &WSAData);
#else
  if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
    log(ERROR_LOG, "ignore SIGHUP failed.");
  }
#endif
  ::evthread_use_pthreads();
  event_base_ = ::event_base_new();

  options_ = thread_safe ? BEV_OPT_THREADSAFE : 0;
}

network_loop::~network_loop()
{
    for (const auto & t : timers_) {
        ::evtimer_del(t);
    }

    if (event_base_) {
        ::event_base_free(event_base_);
        event_base_ = NULL;
    }
}

void network_loop::run()
{
  event_base_dispatch(event_base_);
}

void network_loop::stop(const timeval *tv)
{
  ::event_base_loopexit(event_base_, tv);
}

event* network_loop::add_timer(uint64_t us, timer_listener * listener)
{
    timeval tm;
    tm.tv_sec = us / 1000000;
    tm.tv_usec = us - us / 1000000 * 1000000;
    event * timer = ::event_new(event_base_, -1, EV_READ | EV_PERSIST, network_loop::timer_callback, listener);
    evtimer_add(timer, &tm);
    timers_.insert(timer);

    return timer;
}

void network_loop::remove_timer(event* timer)
{
    timers_.erase(timer);
    evtimer_del(timer);

    // NOTE(liuyong): We should call |event_free| to free the timer. Otherwise, memory leak
    ::event_free(timer);
}

event* network_loop::udp_bind_ipv6(const sockaddr_storage& addr, size_t tries, udp_listener * listener)
{
    //sockaddr_in6 sin = network_helper::empty_ipv6_address();
    //memset((char *) &sin, 0, sizeof(sin));
    //sin.sin6_flowinfo = 0;
    //sin.sin6_family = AF_INET6;
    //memcpy(&sin.sin6_addr.s6_addr, ip.data(), sizeof(in6_addr));

    auto &sin = *(sockaddr_in6*)&addr;
  evutil_socket_t s = ::socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

  size_t times = tries;
    auto port=ntohs(sin.sin6_port);
  while (times > 0) {
        sin.sin6_port = htons(port);
        if (::bind(s, (const sockaddr*)&sin, sizeof(sin)) == 0) {
            port = ntohs(sin.sin6_port);
            event* ev = ::event_new(event_base_, s, EV_READ | EV_PERSIST, udp_event_callback, listener);
            ::event_add(ev, NULL);
            return ev;
    }

    log(WARN_LOG, "try %u to bind on port %u failed, error %u", times, port, errno);
    ++ port;
    --times;
  }
  ::evutil_closesocket(s);
  return NULL;
}


event* network_loop::udp_bind(uint16_t & port, size_t tries, udp_listener * listener, uint32_t ip)
{
    sockaddr_in sin = {0, 0, {0}, {0}};
  sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip;
  evutil_socket_t s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  size_t times = tries;
  while (times > 0) {
        sin.sin_port = htons(port);
        if (::bind(s, (const sockaddr*)&sin, sizeof(sin)) == 0) {
            port = ntohs(sin.sin_port);
            event* ev = ::event_new(event_base_, s, EV_READ | EV_PERSIST, udp_event_callback, listener);
            ::event_add(ev, NULL);
            return ev;
    }

    log(WARN_LOG, "try %u to bind on port %u failed, error %u", times, port, errno);
    ++ port;
    --times;
  }
  ::evutil_closesocket(s);
  return NULL;
}

void network_loop::close(event * e)
{
  evutil_socket_t s = ::event_get_fd(e);
  ::event_free(e);
  ::evutil_closesocket(s);
}

void network_loop::close(bufferevent * bev)
{
    if (bev == nullptr) {
        return ;
    }
    ::bufferevent_free(bev);
}

void network_loop::http_end(evhttp_request *req)
{
    if (evhttp_request_is_owned(req)) {
        evhttp_request_free(req);
    }
}

evconnlistener* network_loop::tcp_bind(uint16_t port, bind_listener * listener, uint32_t ip)
{
    sockaddr_in sin = {0, 0, {0}, {0}};
  sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip;
    sin.sin_port = htons(port);
  return ::evconnlistener_new_bind(event_base_,
    network_loop::accept_callback, listener,
    LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
        (struct sockaddr*)&sin, sizeof(sin));
}

evhttp *network_loop::http_bind(uint16_t port, http_listener *listener, uint32_t ip)
{
    evhttp* http = ::evhttp_new(event_base_);
    ::evhttp_set_gencb(http, network_loop::http_request_callback, listener);

    std::string host("0.0.0.0");
    if (ip != 0) {
        host = ip_to_string(ip);
    }

    if (::evhttp_bind_socket(http, host.c_str(), port) != 0) {
        evhttp_free(http);
        http = nullptr;
    }
    return http;
}

bufferevent * network_loop::tcp_connect(const sockaddr_in & sin, connect_listener * listener)
{
    bufferevent *bev = ::bufferevent_socket_new(event_base_, -1, BEV_OPT_CLOSE_ON_FREE|options_);
    ::bufferevent_setcb(bev, NULL, NULL, network_loop::connect_callback, listener);

    if (::bufferevent_socket_connect(bev, (sockaddr *)&sin, sizeof(sin)) < 0) {
        // NOTE(liuyong): If there is no route to the destination, |connect|
        // will return immediately, and |connect_callback| will get invoked and
        // |bufferevent_free| will be called, so don't attempt to free |bev| here.
        // ::bufferevent_free(bev);
        return NULL;
    }

    return bev;
}

bufferevent * network_loop::tcp_connect(uint32_t ip, uint16_t port, connect_listener * listener)
{
    sockaddr_in sin = {0, 0, {0}, {0}};
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip;
    sin.sin_port = htons(port);
    return tcp_connect(sin, listener);
}

bufferevent * network_loop::set_tcp_listener(evutil_socket_t fd, tcp_listener * listener)
{
  bufferevent *bev = ::bufferevent_socket_new(event_base_, fd, BEV_OPT_CLOSE_ON_FREE|options_);
  set_tcp_listener(bev, listener);
    return bev;
}

void network_loop::set_tcp_listener(bufferevent *bev, tcp_listener * listener)
{
  bufferevent_setcb(bev, network_loop::tcp_read_callback, network_loop::tcp_write_callback, network_loop::tcp_event_callback, listener);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
}

void network_loop::set_read_timeout(bufferevent *bev, uint32_t second)
{
    timeval read_timeval = {0, 0};
    read_timeval.tv_sec = second;
    bufferevent_set_timeouts(bev, &read_timeval, NULL);
}

void network_loop::set_write_timeout(bufferevent *bev, uint32_t second)
{
    timeval write_timeval = {0, 0};
    write_timeval.tv_sec = second;
    bufferevent_set_timeouts(bev, NULL, &write_timeval);
}

void network_loop::send_buffer(bufferevent *bev, const char *data, int length)
{
    bufferevent_write(bev, data, length);
}

bool network_loop::sendto(evutil_socket_t s, const sockaddr_storage & addr, const char* msg, int len)
{
  if( ::sendto(s, msg, len, 0, (sockaddr*)&addr, sizeof(addr)) == -1 ) {
    log(WARN_LOG, "send datagram failed %u on socket %u to %s", errno, s, address_to_string(addr).c_str());
        return false;
  }
    return true;
}
bool network_loop::sendto(evutil_socket_t s, const sockaddr_in & addr, const char* msg, int len)
{
  if( ::sendto(s, msg, len, 0, (sockaddr*)&addr, sizeof(addr)) == -1 ) {
    log(WARN_LOG, "send datagram failed %u on socket %u to %s", errno, s, address_to_string(addr).c_str());
        return false;
  }
    return true;
}

void network_loop::send_message(bufferevent * bev, const packet & p)
{
  packer pk;
  p.pack(pk);

  bufferevent_write(bev, pk.buffer(), pk.length());
}

void network_loop::send_message(bufferevent * bev, const unpacker & p)
{
    bufferevent_write(bev, p.buffer(), p.length());
}

void network_loop::http_reply(evhttp_request *req,  const char *data, int length, int code, const char* reason)
{
    if (data != nullptr) {
        evbuffer* databuf = evbuffer_new();
        evbuffer_add(databuf, data, length);
        evhttp_send_reply(req, code, reason, databuf);
        evbuffer_free(databuf);
    } else {
        evhttp_send_reply(req, code, reason, nullptr);
    }
}

void network_loop::udp_event_callback(evutil_socket_t s, short event, void* context)
{
    if (event == EV_READ) {
        udp_listener* listener = (udp_listener*)context;
        listener->on_data(s);
    } else {
        log(WARN_LOG, "udp_event_callback, unexpected event %x", event);
    }
}

void network_loop::timer_callback(int fd, short event, void* context ) {
    (void)fd;
    (void)event;
  ((timer_listener*)context)->on_timer();
}

void network_loop::accept_callback(evconnlistener *listener, evutil_socket_t fd, sockaddr *address, int socklen, void* context)
{
  ((bind_listener*)context)->on_accept(listener, fd, address, socklen);
}

void network_loop::connect_callback(bufferevent *bev, short events, void *context)
{
  ((connect_listener*)context)->on_connect(bev, events);
    if (events & (BEV_EVENT_ERROR|BEV_EVENT_EOF|BEV_EVENT_TIMEOUT)) {
    ::bufferevent_free(bev);
  }
}

void network_loop::tcp_read_callback(bufferevent *bev, void * context)
{
  ((tcp_listener*)context)->on_read(bev);
}

void network_loop::tcp_write_callback(bufferevent* bev, void * context)
{
  ((tcp_listener*)context)->on_write(bev);
}

void network_loop::tcp_event_callback(bufferevent *bev, short events, void * context)
{
  ((tcp_listener*)context)->on_event(bev, events);
    if (events & (BEV_EVENT_ERROR|BEV_EVENT_EOF|BEV_EVENT_TIMEOUT)) {
        if (free_bev_on_error_) {
            ::bufferevent_free(bev);
        }
    }
}

void network_loop::http_request_callback(evhttp_request *req, void *context)
{
    ((http_listener*)context)->on_request(req);
}
