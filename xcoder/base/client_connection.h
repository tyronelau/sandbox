#pragma once
#include <cstdint>
#include <arpa/inet.h>
#include "base/tcp_connection.h"

namespace agora {
namespace base {
class unpacker;
class client_connection;

struct tcp_client_handler {
  virtual void on_connect(client_connection *conn, bool connected) = 0;
  virtual void on_packet(client_connection *conn, unpacker &pkr,
      uint16_t service_type, uint16_t uri) = 0;

  virtual void on_ping_cycle(client_connection *conn) = 0;
  virtual void on_socket_error(client_connection *conn) = 0;
};

class client_connection : private tcp_connection_listener {
 public:
  client_connection(event_base *base, uint32_t ip, uint16_t port,
      tcp_client_handler *listener, bool keep_alive=true);
  ~client_connection();

  bool is_connected() const;
  sockaddr_in get_remote_addr() const;
  sockaddr_in get_local_addr() const;

  void set_timeout(uint32_t micro_seconds);
  void set_connection_listener(tcp_client_handler *listener);

  bool connect();
  bool stop();
  void close();

  bool send_buffer(const char *data, uint32_t length);
  bool send_message(const packet &p);
 private:
  void on_connect(short events);
  void on_timer();

  virtual void on_error(tcp_connection *conn, short events);
  virtual void on_receive_packet(tcp_connection *conn, unpacker &p,
      uint16_t server_type, uint16_t uri);
 private:
  static void timer_callback(int fd, short events, void *context);
  static void connect_callback(bufferevent *bev, short events, void *context);
 private:
  event_base *base_;
  sockaddr_in addr_;
  tcp_client_handler *handler_;
  event *timer_;

  bufferevent *pending_;
  tcp_connection *conn_;
  bool connecting_;
  bool stopped_;

  uint32_t last_active_ts_;
};

}
}

