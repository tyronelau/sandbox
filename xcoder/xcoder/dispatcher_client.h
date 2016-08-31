#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "base/event_queue.h"
#include "base/client_connection.h"
#include "xcoder/rtmp_event.h"

struct event_base;

namespace agora {
namespace recording {

class StreamObserver;

class rtmp_event {
 public:
  enum event_kind {kErrorReport=0, kBwReport = 1,};

  rtmp_event();
  explicit rtmp_event(const std::string &url, int bw_kbps);
  rtmp_event(const std::string &url, event_kind kind, int data);

  event_kind get_event_kind() const;
  const std::string& get_rtmp_url() const;
  int get_data() const;

  rtmp_event(const rtmp_event &) = default;
  rtmp_event& operator=(const rtmp_event &) = default;

  rtmp_event(rtmp_event &&);
  rtmp_event& operator=(rtmp_event &&);
 private:
  event_kind kind_;
  std::string rtmp_url_;
  union {
    int error_code;
    int bw_kbps;
  } data_;
};

class dispatcher_client : private base::tcp_client_handler,
    public rtmp_event_handler, private base::async_event_handler<rtmp_event> {
 public:
  dispatcher_client(event_base *base, uint32_t vendor_id,
      const std::string &cname, uint16_t port,
      dispatcher_event_handler *callback=NULL);

  ~dispatcher_client();

  bool connect();

  virtual void on_rtmp_error(const std::string &url, int error_code);
  virtual void on_bandwidth_report(const std::string &url, int bw_kbps);
 private:
  virtual void on_event(const rtmp_event &e);

  virtual void on_connect(base::client_connection *conn, bool connected);
  virtual void on_packet(base::client_connection *conn, base::unpacker &pkr,
      uint16_t service_type, uint16_t uri);

  virtual void on_ping_cycle(base::client_connection *conn);
  virtual void on_socket_error(base::client_connection *conn);
 private:
  void on_recorder_joined(int code);
  void on_recorder_quit(int code);

  void on_add_publisher(const std::string &rtmp_url);
  void on_remove_publisher(const std::string &rtmp_url);
  void on_replace_publisher(const std::vector<std::string> &urls);

  void send_join_request();
  void handle_rtmp_error(const std::string &url, int error_code);
  void handle_bandwidth_report(const std::string &url, int bw_kbps);
 private:
  const uint32_t vendor_id_;
  event_base *loop_;
  std::string channel_name_;
  uint16_t port_;
  dispatcher_event_handler *callback_;

  base::client_connection client_;

  bool joined_;
  base::event_queue<rtmp_event> queue_;
};

}
}
