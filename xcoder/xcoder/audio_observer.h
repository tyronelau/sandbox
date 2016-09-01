#pragma once

#include "internal/chat_engine_i.h"

namespace agora {
namespace recording {

class audio_observer : public media::IAudioFrameObserver {
 public:
  virtual bool onRecordFrame(void *audioSample, int nSamples,
      int nBytesPerSample, int nChannels, int samplesPerSec);

  // audio mixed PCM data received from the Engine.
  virtual bool onPlaybackFrame(void *audioSample, int nSamples,
      int nBytesPerSample, int nChannels, int samplesPerSec);

  // Decoded unmixed PCM data from User |uid|
  virtual bool onPlaybackFrameUid(unsigned int uid, void *audioSample,
      int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec);
 private:
  // STORE YOUR DATA HERE
};

}
}
