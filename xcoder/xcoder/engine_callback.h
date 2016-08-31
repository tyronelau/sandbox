#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "internal/ICodingModuleCallback.h"
#include "xcoder/rtmp_manager.h"

namespace agora {
namespace recording {
struct rtmp_event_handler;

class StreamObserver : public AgoraRTC::ICMFileObserver {
  class PeerStream;
  typedef std::unique_ptr<PeerStream> peer_stream_t;
 public:
  explicit StreamObserver(const std::vector<std::string> &urls,
      rtmp_event_handler *callback=NULL);

  virtual ~StreamObserver();

  void register_rtmp_callabck(rtmp_event_handler *callback);

  bool add_rtmp_url(const std::string &url);
  bool remove_rtmp_url(const std::string &url);
  bool replace_rtmp_urls(const std::vector<std::string> &urls);

  std::vector<std::string> get_rtmp_urls() const;

  bool set_mosaic_mode(bool mosaic);

  virtual AgoraRTC::ICMFile* GetICMFileObject(unsigned int uid);
  virtual int InsertRawAudioPacket(unsigned int uid,
      const unsigned char *payload, unsigned short size, int type,
      unsigned int ts, unsigned short seq);
 private:
  StreamObserver(const StreamObserver &rhs) = delete;
  StreamObserver(StreamObserver &&rhs) = delete;

  StreamObserver& operator=(const StreamObserver &rhs) = delete;
  StreamObserver& operator=(StreamObserver &&rhs) = delete;
 private:
  rtmp_manager rtmp_mgr_;
  std::mutex lock_;
  std::unordered_map<unsigned, peer_stream_t> peers_;
};

}
}
