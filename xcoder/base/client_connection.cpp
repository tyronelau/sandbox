#include "base/client_connection.h"

#include <cassert>
#include "base/log.h"
#include "base/packet.h"
#include "base/utils.h"

namespace agora {
namespace base {

namespace {
enum {
  MICRO_PER_SEC = 1000 * 1000,
  KEEP_ALIVE_INTERVAL = 2 * MICRO_PER_SEC,
  RECONNECT_TIMEOUT = 10,
};

}

client_connection::client_connection(event_base *base, uint32_t ip,
    uint16_t port, tcp_client_handler *handler, bool keep_alive) {
  (void)keep_alive;

  base_ = base;
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = ip;
  addr_.sin_port = htons(port);

  handler_ = handler;

  last_active_ts_ = now_seconds();

  timer_ = event_new(base_, -1, EV_READ | EV_PERSIST, timer_callback, this);

  uint32_t micro_seconds = KEEP_ALIVE_INTERVAL;

  timeval tm;
  tm.tv_sec = micro_seconds / MICRO_PER_SEC;
  tm.tv_usec = micro_seconds % MICRO_PER_SEC;
  evtimer_add(timer_, &tm);

  conn_ = NULL;
  connecting_ = false;
  stopped_ = false;
}

client_connection::~client_connection() {
  close();

  evtimer_del(timer_);
  event_free(timer_);
}

bool client_connection::is_connected() const {
  return !connecting_ && conn_;
}

sockaddr_in client_connection::get_remote_addr() const {
  return addr_;
}

sockaddr_in client_connection::get_local_addr() const {
  assert(conn_ != NULL);
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  if (conn_) {
    return conn_->get_local_addr();
  }

  return addr;
}

void client_connection::set_timeout(uint32_t micro_seconds) {
  if (timer_) {
    evtimer_del(timer_);
    event_free(timer_);
    timer_ = NULL;
  }

  timer_ = event_new(base_, -1, EV_READ | EV_PERSIST, timer_callback, this);

  timeval tm;
  tm.tv_sec = micro_seconds / MICRO_PER_SEC;
  tm.tv_usec = micro_seconds % MICRO_PER_SEC;
  evtimer_add(timer_, &tm);
}

void client_connection::set_connection_listener(tcp_client_handler *handler) {
  handler_ = handler;
}

void client_connection::timer_callback(int fd, short event, void *context) {
  (void)fd;
  (void)event;

  client_connection *conn = reinterpret_cast<client_connection*>(context);
  conn->on_timer();
}

bool client_connection::connect() {
  if (connecting_)
    return true;

  if (conn_) {
    conn_->close_and_destroy();
    conn_ = NULL;
  }

  connecting_ = true;
  pending_ = ::bufferevent_socket_new(base_, -1, BEV_OPT_CLOSE_ON_FREE);
  ::bufferevent_setcb(pending_, NULL, NULL, connect_callback, this);

  if (::bufferevent_socket_connect(pending_, (sockaddr *)&addr_, sizeof(addr_)) < 0) {
    // NOTE(liuyong): If there is no route to the destination, |connect|
    // will return immediately, and |connect_callback| will get invoked and
    // |bufferevent_free| will be called, so don't attempt to free |bev| here.
    // ::bufferevent_free(bev);
    connecting_ = false;
    return false;
  }

  return true;
}

void client_connection::connect_callback(bufferevent *bev, short events,
    void *context) {
  client_connection *p = reinterpret_cast<client_connection*>(context);
  assert(bev == p->pending_);

  p->on_connect(events);
}

void client_connection::on_connect(short events) {
  assert(pending_ != NULL);
  assert(conn_ == NULL);
  assert(connecting_);

  connecting_ = false;

  if (events & BEV_EVENT_CONNECTED) {
    int s = bufferevent_getfd(pending_);
    LOG(INFO, "Connected to %d, %s", s, address_to_string(addr_).c_str());

    conn_ = new tcp_connection(base_, pending_);
    conn_->set_connection_listener(this);
    pending_ = NULL;

    handler_->on_connect(this, true);
  } else if (events & BEV_EVENT_ERROR) {
    bufferevent_free(pending_);
    pending_ = NULL;

    handler_->on_connect(this, false);
  } else {
    LOG(FATAL, "Should not fall through here");
    pending_ = NULL;
  }
}

bool client_connection::send_buffer(const char *data, uint32_t length) {
  if (!is_connected()) {
    return false;
  }

  conn_->send_buffer(data, length);
  return true;
}

bool client_connection::send_message(const packet &p) {
  if (!is_connected()) {
    LOG(ERROR, "cannot send message %u %u to %s, not connected",
        p.server_type, p.uri, address_to_string(addr_).c_str());
    return false;
  }

  conn_->send_packet(p);
  return true;
}

void client_connection::on_error(tcp_connection *conn, short events) {
  assert(conn == conn_);
  (void)conn;
  (void)events;

  close();

  handler_->on_socket_error(this);
}

void client_connection::on_receive_packet(tcp_connection *conn, unpacker &p,
    uint16_t server_type, uint16_t uri) {
  assert(conn == conn_);
  (void)conn;

  last_active_ts_ = now_seconds();
  handler_->on_packet(this, p, server_type, uri);
}

void client_connection::on_timer() {
  // check if we need to reconnect the server
  if (last_active_ts_ + RECONNECT_TIMEOUT < now_seconds()) {
    close();

    last_active_ts_ = now_seconds();

    connect();
    return;
  }

  if (!is_connected())
    return;

  handler_->on_ping_cycle(this);
}

void client_connection::close() {
  // A connection is pending
  if (pending_) {
    assert(conn_ == NULL);
    connecting_ = false;
    bufferevent_free(pending_);
    pending_ = NULL;

    return;
  }

  assert(connecting_ == false);

  if (conn_) {
    conn_->close_and_destroy();
    conn_ = NULL;
  }
}

}
}
