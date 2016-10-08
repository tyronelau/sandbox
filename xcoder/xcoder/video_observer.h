#pragma once

#include <memory>

#include "internal/chat_engine_i.h"
#include "base/event_queue.h"

namespace agora {
namespace base {
class packet;
};

namespace recording {

class video_observer : public media::IVideoFrameObserver {
  typedef std::unique_ptr<base::packet> frame_ptr_t;
 public:
  explicit video_observer(base::event_queue<frame_ptr_t> *frame_queue);
 public:
  typedef unsigned char uchar_t;
  typedef unsigned int uint_t;

  virtual bool onCaptureVideoFrame(uchar_t *yBuffer, uchar_t *uBuffer,
      uchar_t *vBuffer, uint_t width, uint_t height,
      uint_t yStride, uint_t uStride, uint_t vStride);

  virtual bool onRenderVideoFrame(uint_t uid, int rotation, uchar_t *yBuffer,
      uchar_t *uBuffer, uchar_t *vBuffer, uint_t width,
      uint_t height, uint_t yStride, uint_t uStride,
      uint_t vStride);

  virtual bool onExternalVideoFrame(uchar_t *yBuffer, uchar_t *uBuffer,
      uchar_t *vBuffer, uint_t width, uint_t height,
      uint_t yStride, uint_t uStride, uint_t vStride);
 private:
  base::event_queue<frame_ptr_t> *queue_;
};

}
}
