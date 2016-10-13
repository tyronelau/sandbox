#include "xcoder/video_observer.h"

#include <cassert>

#include "base/safe_log.h"
#include "base/time_util.h"
#include "protocol/ipc_protocol.h"

namespace agora {
namespace xcodec {

video_observer::video_observer(base::event_queue<frame_ptr_t> *frame_queue) {
  queue_ = frame_queue;
}

bool video_observer::onCaptureVideoFrame(VideoFrame &frame) {
  (void)frame;

  assert(false);
  SAFE_LOG(WARN) << "This function should not be called";

  return true;
}

bool video_observer::onRenderVideoFrame(uint_t uid, VideoFrame &frame) {
  // TODO: This function gets called each time the engine decoded a frame
  // coming from User |uid|. Save the yuv buffer for your own processing.
  const void *yBuffer = frame.yBuffer;
  const void *uBuffer = frame.uBuffer;
  const void *vBuffer = frame.vBuffer;
  int width = frame.width;
  int height = frame.height;
  int yStride = frame.yStride;
  int uStride = frame.uStride;
  int vStride = frame.vStride;

  SAFE_LOG(DEBUG) << "Frame received: " << uid << ", width: " << width
      << ", height: " << height;

  // FIXME
  protocol::video_frame *f = new protocol::video_frame;
  f->uid = uid;
  f->frame_ms = static_cast<uint32_t>(base::now_ms());
  f->width = static_cast<uint16_t>(width);
  f->height = static_cast<uint16_t>(height);
  f->ystride = static_cast<uint16_t>(yStride);
  f->ustride = static_cast<uint16_t>(uStride);
  f->vstride = static_cast<uint16_t>(vStride);

  size_t y_size = height * yStride;
  size_t u_size = height * uStride / 2;
  size_t v_size = height * vStride / 2;

  std::string &data = f->data;
  data.reserve(y_size + u_size + v_size);

  const char *start = reinterpret_cast<const char *>(yBuffer);
  data.insert(data.end(), start, start + y_size);

  start = reinterpret_cast<const char *>(uBuffer);
  data.insert(data.end(), start, start + u_size);

  start = reinterpret_cast<const char *>(vBuffer);
  data.insert(data.end(), start, start + v_size);

  queue_->push(frame_ptr_t(f));
  return true;
}

}
}
