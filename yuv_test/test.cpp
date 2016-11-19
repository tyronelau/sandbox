#include <string>

#include "libyuv/planar_functions.h"

struct YuvImage {
 public:
  explicit YuvImage(unsigned width=640, unsigned height=360);
  ~YuvImage();

  bool LoadImage(const std::string &file);
  bool BlendImage(const YuvImage &src, float alpha=1.0);
 private:
  unsigned int width_;
  unsigned int height_;

  uint8_t *real_buf_;

  uint8_t *ybuf_;
  uint8_t *ubuf_;
  uint8_t *vbuf_;
};

int main() {
  return 0;
}

