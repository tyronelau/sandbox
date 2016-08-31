#include "xcoder/dispatcher_client.h"

#include "base/log.h"
#include "base/packet.h"
#include "base/safe_log.h"
#include "base/utils.h"
#include "xcoder/engine_callback.h"

namespace agora {
namespace recording {

using std::string;
using std::vector;

using base::client_connection;
using base::unpacker;

enum CallbackType {
  CB_BANDWIDTH    = 0,
  CB_ERROR_CODE   = 1,
};

enum recorder_protocol_uri {
  CDN_JOIN_DISPATCHER_REQ_URI = 1,
  CDN_JOIN_DISPATCHER_RES_URI = 2,
  CDN_QUIT_DISPATCHER_REQ_URI = 3,
  CDN_QUIT_DISPATCHER_RES_URI = 4,
  CDN_PING_DISPATCHER_URI = 5,
  CDN_PONG_DISPATCHER_URI = 6,
  CDN_PUBLISHER_BW_URI = 8,
  CDN_PUBLISHER_ERROR_URI = 9,
  CDN_REPLACE_PUBLISHER_REQ_URI = 10,
  CDN_REPLACE_PUBLISHER_RES_URI = 11,
  CDN_ADD_PUBLISHER_REQ_URI = 12,
  CDN_ADD_PUBLISHER_RES_URI = 13,
  CDN_REMOVE_PUBLISHER_REQ_URI = 14,
  CDN_REMOVE_PUBLISHER_RES_URI = 15,
};

DECLARE_PACKET_4(PPublisherBwReport, CDN_PUBLISHER_BW_URI, uint32_t, vendor_id,
    std::string, cname, std::string, rtmp_url, uint32_t, bandwidth)

DECLARE_PACKET_4(PPublisherErrorReport, CDN_PUBLISHER_ERROR_URI,
    uint32_t, vendor_id, std::string, cname, std::string, rtmp_url,
    uint16_t, error)

DECLARE_PACKET_3(PRecorderJoinReq, CDN_JOIN_DISPATCHER_REQ_URI,
    uint32_t, vendor_id, std::string, cname,
    std::vector<std::string>, rtmp_urls)

DECLARE_PACKET_1(PRecorderJoinRes, CDN_JOIN_DISPATCHER_RES_URI, int32_t, code)

DECLARE_PACKET_1(PRecorderQuitReq, CDN_QUIT_DISPATCHER_REQ_URI, int32_t, reason)
DECLARE_PACKET_1(PRecorderQuitRes, CDN_QUIT_DISPATCHER_RES_URI, int32_t, code)

DECLARE_PACKET_1(PPingDispatcher, CDN_PING_DISPATCHER_URI, int32_t, from)
DECLARE_PACKET_1(PPongDispatcher, CDN_PONG_DISPATCHER_URI, int32_t, to)

DECLARE_PACKET_1(PReplacePublisherReq, CDN_REPLACE_PUBLISHER_REQ_URI,
    std::vector<std::string>, rtmp_urls)

DECLARE_PACKET_2(PReplacePublisherRes, CDN_REPLACE_PUBLISHER_RES_URI,
    std::vector<std::string>, rtmp_urls, int32_t, code)

DECLARE_PACKET_1(PAddPublisherReq, CDN_ADD_PUBLISHER_REQ_URI,
    std::string, rtmp_url)

DECLARE_PACKET_2(PAddPublisherRes, CDN_ADD_PUBLISHER_RES_URI,
    std::string, rtmp_url, int32_t, result)

DECLARE_PACKET_1(PRemovePublisherReq, CDN_REMOVE_PUBLISHER_REQ_URI,
    std::string, rtmp_url)

DECLARE_PACKET_2(PRemovePublisherRes, CDN_REMOVE_PUBLISHER_RES_URI,
    std::string, rtmp_url, int32_t, result)

inline rtmp_event::rtmp_event() {
  kind_ = kErrorReport;
  data_.error_code = 0;
}

inline rtmp_event::rtmp_event(const string &url, int bw_kbps) {
  kind_ = kBwReport;
  rtmp_url_ = url;

  data_.bw_kbps = bw_kbps;
}

inline rtmp_event::rtmp_event(const string &url, event_kind kind, int data) {
  kind_ = kind;
  rtmp_url_ = url;
  data_.error_code = data;
}

inline rtmp_event::event_kind rtmp_event::get_event_kind() const {
  return kind_;
}

inline const string& rtmp_event::get_rtmp_url() const {
  return rtmp_url_;
}

inline int rtmp_event::get_data() const {
  return data_.error_code;
}

inline rtmp_event::rtmp_event(rtmp_event &&rhs) :kind_(rhs.kind_),
    rtmp_url_(std::move(rhs.rtmp_url_)) {
  data_.bw_kbps = rhs.data_.bw_kbps;
}

inline rtmp_event& rtmp_event::operator=(rtmp_event &&rhs) {
  if (this != &rhs) {
    std::swap(kind_, rhs.kind_);
    std::swap(rtmp_url_, rhs.rtmp_url_);
    std::swap(data_.bw_kbps, rhs.data_.bw_kbps);
  }

  return *this;
}

dispatcher_client::dispatcher_client(event_base *base, uint32_t vendor_id,
    const string &cname, uint16_t port, dispatcher_event_handler *callback)
    : vendor_id_(vendor_id), loop_(base), channel_name_(cname),
    port_(port), callback_(callback),
    client_(base, htonl(INADDR_LOOPBACK), port, this, false),
    queue_(base, this, 256) {
  joined_ = false;
}

dispatcher_client::~dispatcher_client() {
}

bool dispatcher_client::connect() {
  return client_.connect();
}

void dispatcher_client::on_rtmp_error(const string &url, int error_code) {
  LOG(DEBUG, "error occured: %s, code: %d", url.c_str(), error_code);

  queue_.push(rtmp_event(url, rtmp_event::kErrorReport, error_code));
}

void dispatcher_client::handle_rtmp_error(const string &url, int error_code) {
  PPublisherErrorReport errorReport;

  errorReport.vendor_id = vendor_id_;
  errorReport.cname = channel_name_;
  errorReport.rtmp_url = url;
  errorReport.error = static_cast<uint16_t>(error_code);

  LOG(INFO, "rtmp %s error type %d", url.c_str(), error_code);

  if (joined_ && client_.is_connected()) {
    client_.send_message(errorReport);
  }
}

void dispatcher_client::on_bandwidth_report(const string &url, int bw_kbps) {
  LOG(INFO, "bandwidth report : %s, bw: %d kbps", url.c_str(), bw_kbps);

  queue_.push(rtmp_event(url, rtmp_event::kBwReport, bw_kbps));
}

void dispatcher_client::handle_bandwidth_report(const string &url, int bw_kbps) {
  PPublisherBwReport bwReport;
  bwReport.vendor_id = vendor_id_;
  bwReport.cname = channel_name_;
  bwReport.rtmp_url = url;
  bwReport.bandwidth = bw_kbps;

  LOG(INFO, "rtmp %s bandwidth %d", url.c_str(), bw_kbps);

  if (joined_ && client_.is_connected()) {
    client_.send_message(bwReport);
  }
}

void dispatcher_client::on_connect(client_connection *c, bool connected) {
  joined_ = false;

  if (connected) {
    SAFE_LOG(INFO) << "Connected to " << base::address_to_string(
        c->get_remote_addr()) << ", try to join";

    send_join_request();
  }
}

void dispatcher_client::send_join_request() {
  PRecorderJoinReq req;
  req.vendor_id = vendor_id_;
  req.cname = channel_name_;
  req.rtmp_urls = callback_->get_rtmp_urls();

  client_.send_message(req);
}

void dispatcher_client::on_packet(client_connection *c, unpacker &p,
    uint16_t server_type, uint16_t uri) {
  (void)c;
  (void)p;
  (void)server_type;
  (void)uri;

  switch (uri) {
  case CDN_JOIN_DISPATCHER_RES_URI: {
    PRecorderJoinRes res;
    res.unmarshall(p);
    on_recorder_joined(res.code);
    break;
  }
  case CDN_QUIT_DISPATCHER_RES_URI: {
    PRecorderQuitRes res;
    res.unmarshall(p);
    on_recorder_quit(res.code);
    break;
  }
  case CDN_REPLACE_PUBLISHER_REQ_URI: {
    PReplacePublisherReq req;
    req.unmarshall(p);
    on_replace_publisher(req.rtmp_urls);
    break;
  }
  case CDN_ADD_PUBLISHER_REQ_URI: {
    PAddPublisherReq req;
    req.unmarshall(p);
    on_add_publisher(req.rtmp_url);
    break;
  }
  case CDN_REMOVE_PUBLISHER_REQ_URI: {
    PRemovePublisherReq req;
    req.unmarshall(p);
    on_remove_publisher(req.rtmp_url);
    break;
  }
  default: break;
  }
}


void dispatcher_client::on_recorder_joined(int code) {
  SAFE_LOG(INFO) << "Recorder join to the dispatcher: " << code;

  if (code == 0)
    joined_ = true;
}

void dispatcher_client::on_recorder_quit(int code) {
  SAFE_LOG(INFO) << "Recorder quits the dispatcher: " << code;
  joined_ = false;
}

void dispatcher_client::on_replace_publisher(const vector<string> &urls) {
}

void dispatcher_client::on_add_publisher(const std::string &rtmp_url) {
  callback_->on_add_publisher(rtmp_url);

  PAddPublisherRes res;
  res.rtmp_url = rtmp_url;
  res.result = 0;

  client_.send_message(res);
}

void dispatcher_client::on_remove_publisher(const std::string &rtmp_url) {
  callback_->on_remove_publisher(rtmp_url);

  PRemovePublisherRes res;
  res.rtmp_url = rtmp_url;
  res.result = 0;

  client_.send_message(res);
}

void dispatcher_client::on_ping_cycle(client_connection *c) {
  (void)c;

  PPingDispatcher ping;
  ping.from = 0; // unused
  client_.send_message(ping);
}

void dispatcher_client::on_socket_error(client_connection *client) {
  (void)client;
  joined_ = false;
}

void dispatcher_client::on_event(const rtmp_event &e) {
  switch (e.get_event_kind()) {
  case rtmp_event::kErrorReport: {
    handle_rtmp_error(e.get_rtmp_url(), e.get_data());
    break;
  }
  case rtmp_event::kBwReport: {
    handle_bandwidth_report(e.get_rtmp_url(), e.get_data());
    break;
  }
  default: break;
  }
}

}
}
