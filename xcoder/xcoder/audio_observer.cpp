#include "xcoder/audio_observer.h"

#include <cassert>

namespace agora {
namespace recording {

bool audio_observer::onRecordFrame(void *audioSample, int nSamples,
    int nBytesPerSample, int nChannels, int samplesPerSec) {
  (void)audioSample;
  (void)nSamples;
  (void)nBytesPerSample;
  (void)nChannels;
  (void)samplesPerSec;

  assert(false);

  // This function will not be called for receiving-only mode.
  return true;
}

bool audio_observer::onPlaybackFrame(void *audioSample, int nSamples,
    int nBytesPerSample, int nChannels, int samplesPerSec) {
  (void)audioSample;
  (void)nSamples;
  (void)nBytesPerSample;
  (void)nChannels;
  (void)samplesPerSec;

  // TODO: add your processing here
  return true;
}

bool audio_observer::onPlaybackFrameUid(unsigned int uid, void *audioSample,
    int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec) {
  (void)uid;
  (void)audioSample;
  (void)nSamples;
  (void)nBytesPerSample;
  (void)nChannels;
  (void)samplesPerSec;

  // TODO: add your processing here
  return true;
}

}
}
