#include "base/tcp_connection.h"

#include <cassert>

#include "base/log.h"
#include "base/packer.h"
#include "base/packet.h"
#include "base/utils.h"

namespace agora {
namespace base {
tcp_connection::tcp_connection(event_base *base, evutil_socket_t fd) {
  is_in_processing_ = false;

  base_ = base;
  listener_ = NULL;
  bev_ = NULL;

  if (fd != -1) {
    set_socket_fd(fd);
  }
}

tcp_connection::tcp_connection(event_base *base, bufferevent *bev) {
  is_in_processing_ = false;

  base_ = base;
  listener_ = NULL;

  bev_ = bev;
  bufferevent_setcb(bev_, read_callback, write_callback, event_callback, this);
  bufferevent_enable(bev_, EV_READ|EV_WRITE);

  assert(bev_ != NULL);
  evutil_socket_t s = bufferevent_getfd(bev_);
  fill_sock_addr(s);
}

tcp_connection::~tcp_connection() {
  close();
}

sockaddr_in tcp_connection::get_remote_addr() const {
  return remote_;
}

sockaddr_in tcp_connection::get_local_addr() const {
  return local_;
}

void tcp_connection::fill_sock_addr(evutil_socket_t s) {
  socklen_t len = sizeof(remote_);

  if (getpeername(s, reinterpret_cast<sockaddr *>(&remote_), &len) == -1) {
    LOG(ERROR, "Failed to get the remote addr: %d", errno);
  }

  assert(len ==  sizeof(remote_));

  if (getsockname(s, reinterpret_cast<sockaddr *>(&local_), &len) == -1) {
    LOG(ERROR, "Failed to get the local addr: %d", errno);
  }

  assert(len == sizeof(local_));
}

void tcp_connection::set_connection_listener(tcp_connection_listener *listener) {
  listener_ = listener;
}

void tcp_connection::set_socket_fd(evutil_socket_t fd) {
  assert(bev_ == NULL);
  bev_ = bufferevent_socket_new(base_, fd, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev_, read_callback, write_callback, event_callback, this);
  bufferevent_enable(bev_, EV_READ|EV_WRITE);

  fill_sock_addr(fd);
}

void tcp_connection::close() {
  if (bev_) {
    bufferevent_free(bev_);
    bev_ = NULL;
  }
}

void tcp_connection::destroy() {
  delete this;
}

void tcp_connection::close_and_destroy() {
  close();

  // delay this destruction
  if (!is_in_processing_)
    destroy();
}

void tcp_connection::send_packet(const packet &p) {
  assert(!is_closed());

  packer pk;
  p.pack(pk);
  bufferevent_write(bev_, pk.buffer(), pk.length());
}

void tcp_connection::send_buffer(const char *buff, uint32_t length) {
  assert(!is_closed());

  bufferevent_write(bev_, buff, length);
}

void tcp_connection::on_data_ready() {
  evbuffer *input = bufferevent_get_input(bev_);
  uint16_t packet_length = 0;  // packet length 2 bytes
  // int s = bufferevent_getfd(bev_);

  enum {kPacketSize = 64 * 1024};
  char buffer[kPacketSize];

  is_in_processing_ = true;

  while (!is_closed()) {
    size_t length = evbuffer_get_length(input);
    if (length <= 2)
      break;

    evbuffer_copyout(input, &packet_length, sizeof(packet_length));
    if (packet_length <= 2) {
      if (listener_)
        listener_->on_error(this, BEV_EVENT_ERROR|BEV_EVENT_READING);
      break;
    }

    if (length < packet_length)
      break;

    size_t read = bufferevent_read(bev_, buffer, packet_length);
    if (read == (size_t)(-1)) {
      sockaddr_in remote = get_remote_addr();
      sockaddr_in local = get_local_addr();

      LOG(ERROR, "read data from socket %d(%s<->%s) error",
          address_to_string(remote).c_str(),
          address_to_string(local).c_str());
      break;
    }

    unpacker p(buffer, packet_length);
    p.pop_uint16(); // uint16_t packet_length

    uint16_t server_type = p.pop_uint16();
    uint16_t uri = p.pop_uint16();
    p.rewind();

    try {
      if (listener_)
        listener_->on_receive_packet(this, p, server_type, uri);
    } catch (const std::overflow_error &e) {
      LOG(ERROR, "error on packet %u %u: %s", server_type, uri, e.what());
      close();
      break;
    }
  }

  is_in_processing_ = false;

  if (is_closed())
    destroy();
}

void tcp_connection::on_event(short events) {
  int s = bufferevent_getfd(bev_);
  if (events & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) {
    LOG(NOTICE, "socket %x %u %s error 0x%x", bev_, s, address_to_string(
        get_remote_addr()).c_str(), events);
    if (listener_)
      listener_->on_error(this, events);
  }

  // close();
  // close_and_destroy();
}

void tcp_connection::read_callback(bufferevent *bev, void *context) {
  (void)bev;

  tcp_connection *c = reinterpret_cast<tcp_connection *>(context);
  c->on_data_ready();
}

void tcp_connection::write_callback(bufferevent *bev, void *context) {
  (void)bev;
  (void)context;
  // do nothing
}

void tcp_connection::event_callback(bufferevent *bev, short events, void *context) {
  (void)bev;

  tcp_connection *c = reinterpret_cast<tcp_connection *>(context);
  c->on_event(events);
}

}
}

