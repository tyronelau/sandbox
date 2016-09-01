#pragma once

#include "internal/chat_engine_i.h"

namespace agora {
namespace recording {

class video_observer : public media::IVideoFrameObserver {
 public:
  typedef unsigned char uchar_t;
  typedef unsigned int uint_t;

  virtual bool onCaptureVideoFrame(uchar_t *yBuffer, uchar_t *uBuffer,
      uchar_t *vBuffer, uint_t width, uint_t height,
      uint_t yStride, uint_t uStride, uint_t vStride);

  virtual bool onRenderVideoFrame(uint_t uid, uchar_t *yBuffer,
      uchar_t *uBuffer, uchar_t *vBuffer, uint_t width,
      uint_t height, uint_t yStride, uint_t uStride,
      uint_t vStride);

  virtual bool onExternalVideoFrame(uchar_t *yBuffer, uchar_t *uBuffer,
      uchar_t *vBuffer, uint_t width, uint_t height,
      uint_t yStride, uint_t uStride, uint_t vStride);
 private:
  // STORE your data
};

}
}
