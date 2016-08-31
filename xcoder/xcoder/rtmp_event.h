#pragma once

#include <string>
#include <vector>

namespace agora {
namespace recording {

// Registered into |rtmp_stream|, monitoring the status of rtmp streams.
struct rtmp_event_handler {
  virtual void on_rtmp_error(const std::string &url, int error_code) = 0;
  virtual void on_bandwidth_report(const std::string &url, int bw_kbps) = 0;
};

struct dispatcher_event_handler {
  virtual std::vector<std::string> get_rtmp_urls() const = 0;

  virtual void on_add_publisher(const std::string &rtmp_url) = 0;
  virtual void on_remove_publisher(const std::string &rtmp_url) = 0;
  virtual void on_replace_publisher(const std::vector<std::string> &urls) = 0;
};

}
}
