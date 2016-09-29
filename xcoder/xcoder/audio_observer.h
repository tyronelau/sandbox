#pragma once

#include <memory>

#include "internal/chat_engine_i.h"
#include "base/event_queue.h"

namespace agora {
namespace base {
class packet;
};

namespace recording {

class audio_observer : public media::IAudioFrameObserver {
  typedef std::unique_ptr<base::packet> frame_ptr_t;
 public:
  explicit audio_observer(base::event_queue<frame_ptr_t> *frame_queue);

  virtual bool onRecordFrame(void *audioSample, int nSamples,
      int nBytesPerSample, int nChannels, int samplesPerSec);

  // audio mixed PCM data received from the Engine.
  virtual bool onPlaybackFrame(void *audioSample, int nSamples,
      int nBytesPerSample, int nChannels, int samplesPerSec);

  // Decoded unmixed PCM data from User |uid|
  virtual bool onPlaybackFrameUid(unsigned int uid, void *audioSample,
      int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec);
 private:
  base::event_queue<frame_ptr_t> *queue_;
};

}
}
