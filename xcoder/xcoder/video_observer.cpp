#include "xcoder/video_observer.h"

#include <cassert>

#include "base/safe_log.h"
#include "base/time_util.h"
#include "protocol/ipc_protocol.h"

namespace agora {
namespace recording {

video_observer::video_observer(base::event_queue<frame_ptr_t> *frame_queue) {
  queue_ = frame_queue;
}

bool video_observer::onCaptureVideoFrame(uchar_t *yBuffer,
    uchar_t *uBuffer, uchar_t *vBuffer, uint_t width,
    uint_t height, uint_t yStride, uint_t uStride, uint_t vStride) {
  (void)yBuffer;
  (void)uBuffer;
  (void)vBuffer;
  (void)width;
  (void)height;
  (void)yStride;
  (void)uStride;
  (void)vStride;

  assert(false);
  SAFE_LOG(WARN) << "This function should not be called";

  return true;
}

bool video_observer::onRenderVideoFrame(uint_t uid, uchar_t *yBuffer,
    uchar_t *uBuffer, uchar_t *vBuffer, uint_t width,
    uint_t height, uint_t yStride, uint_t uStride,
    uint_t vStride) {
  // TODO: This function gets called each time the engine decoded a frame
  // coming from User |uid|. Save the yuv buffer for your own processing.
  (void)uid;
  (void)yBuffer;
  (void)uBuffer;
  (void)vBuffer;
  (void)width;
  (void)height;
  (void)yStride;
  (void)uStride;
  (void)vStride;

  SAFE_LOG(DEBUG) << "Frame received: " << uid << ", width: " << width
      << ", height: " << height;

  // FIXME
  protocol::video_frame *frame = new (std::nothrow)protocol::video_frame;
  frame->uid = uid;
  frame->frame_ms = static_cast<uint32_t>(base::now_ms());
  frame->width = static_cast<uint16_t>(width);
  frame->height = static_cast<uint16_t>(height);
  frame->ystride = static_cast<uint16_t>(yStride);
  frame->ustride = static_cast<uint16_t>(uStride);
  frame->vstride = static_cast<uint16_t>(vStride);

  size_t y_size = height * yStride;
  size_t u_size = height * uStride / 2;
  size_t v_size = height * vStride / 2;

  std::string &data = frame->data;
  data.reserve(y_size + u_size + v_size);

  const char *start = reinterpret_cast<char *>(yBuffer);
  data.insert(data.end(), start, start + y_size);

  start = reinterpret_cast<char *>(uBuffer);
  data.insert(data.end(), start, start + u_size);

  start = reinterpret_cast<char *>(vBuffer);
  data.insert(data.end(), start, start + v_size);

  queue_->push(frame_ptr_t(frame));
  return true;
}

bool video_observer::onExternalVideoFrame(uchar_t *yBuffer, uchar_t *uBuffer,
    uchar_t *vBuffer, uint_t width, uint_t height, uint_t yStride,
    uint_t uStride, uint_t vStride) {
  (void)yBuffer;
  (void)uBuffer;
  (void)vBuffer;
  (void)width;
  (void)height;
  (void)yStride;
  (void)uStride;
  (void)vStride;

  assert(false);
  SAFE_LOG(WARN) << "This function should not be called";

  return true;
}

}
}
