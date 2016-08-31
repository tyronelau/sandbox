#ifndef __CHE_UI_WIN_TEST_AUDIO_OBSERVER_H__
#define __CHE_UI_WIN_TEST_AUDIO_OBSERVER_H__

#include "common_types.h"
#include "chat_engine_i.h"

namespace win_test
{
struct aframe
{
    aframe();
    ~aframe();
    void InitFrameBuffer(int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec);
    void SaveFrame(void* audioSample);
    void CopyFrame(void* audioSample,
                   int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec);
    char *audioSample_;
    int nSamples_;
    int nBytesPerSample_;
    int nChannels_;
    int samplesPerSec_;
};

class MosaicAudioObserver : public agora::media::IAudioFrameObserver
{
public:
    virtual bool onRecordFrame(void *audioSample, int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec);
    virtual bool onPlaybackFrame(void *audioSample, int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec);
    virtual bool onPlaybackFrameUid(unsigned int uid, void *audioSample, int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec);
private:
    aframe audio_frame;
};

}

#endif //__CHE_UI_WIN_TEST_AUDIO_OBSERVER_H__
