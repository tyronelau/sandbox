#pragma once
#include <cstdint>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

namespace agora {
namespace base {
class tcp_connection;
class unpacker;
class packet;

struct tcp_connection_listener {
  // This connection was closed by the remote peer.
  virtual void on_error(tcp_connection *conn, short events) = 0;
  virtual void on_receive_packet(tcp_connection *conn, unpacker &p,
      uint16_t server_type, uint16_t uri) = 0;
};

class tcp_connection {
  friend class tcp_listen_socket;
  friend class client_connection;
 private:
  tcp_connection(event_base *base, evutil_socket_t fd);
  tcp_connection(event_base *base, bufferevent *bev);
  ~tcp_connection();
 public:
  sockaddr_in get_remote_addr() const;
  sockaddr_in get_local_addr() const;

  bool is_closed() const { return bev_ == NULL; }
  event_base* get_event_base() const { return base_; }

  void set_connection_listener(tcp_connection_listener *listener);

  // close the socket and destroy this connection object
  void close_and_destroy();
  void close();
  void destroy();
  void send_buffer(const char *data, uint32_t length);
  void send_packet(const packet &p);
 private:
  void set_socket_fd(evutil_socket_t fd);
  void fill_sock_addr(evutil_socket_t fd);
  void on_data_ready();
  void on_event(short events);
 private:
  static void read_callback(bufferevent *bev, void *context);
  static void write_callback(bufferevent *bev, void *context);
  static void event_callback(bufferevent *bev, short events, void *context);
 private:
  bool is_in_processing_;

  event_base *base_;
  bufferevent *bev_;
  tcp_connection_listener *listener_;

  sockaddr_in remote_;
  sockaddr_in local_;
};

}
}

