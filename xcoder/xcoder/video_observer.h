#pragma once

#include <memory>

#include "internal/chat_engine_i.h"
#include "base/event_queue.h"

namespace agora {
namespace base {
class packet;
};

namespace xcodec {

class video_observer : public media::IVideoFrameObserver {
  typedef std::unique_ptr<base::packet> frame_ptr_t;
 public:
  explicit video_observer(base::event_queue<frame_ptr_t> *frame_queue);
 public:
  typedef unsigned char uchar_t;
  typedef unsigned int uint_t;

  virtual bool onCaptureVideoFrame(VideoFrame &frame);
  virtual bool onRenderVideoFrame(unsigned int uid, VideoFrame &frame);
 private:
  base::event_queue<frame_ptr_t> *queue_;
};

}
}
