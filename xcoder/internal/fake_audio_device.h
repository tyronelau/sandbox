#pragma once
namespace agora {
namespace media {

class IAudioFrameObserver {
 public:
  virtual bool onRecordFrame(void *audioSample, int nSamples,
      int nBytesPerSample, int nChannels, int sampleRates) = 0;

  virtual bool onPlaybackFrame(void *audioSample, int nSamples,
      int nBytesPerSample, int nChannels, int sampleRates) = 0;

  virtual bool onPlaybackFrameUid(unsigned int uid, void *audioSample,
      int nSamples, int nBytesPerSample, int nChannels, int sampleRates) = 0;
};

}
}

