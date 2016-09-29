#include "xcoder/video_observer.h"

#include <cassert>

#include "base/safe_log.h"

namespace agora {
namespace recording {

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
