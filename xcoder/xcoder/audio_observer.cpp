#include "audio_observer.h"

#include "base/log.h"

using namespace win_test;

aframe::aframe() :
    audioSample_(NULL),
    nSamples_(0),
    nBytesPerSample_(0),
    nChannels_(0),
    samplesPerSec_(0)
{}

aframe::~aframe()
{
    if (audioSample_) delete []audioSample_;
}

void aframe::InitFrameBuffer(int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec)
{
    if (nSamples_ != nSamples || nBytesPerSample_ != nBytesPerSample || nChannels_ != nChannels || samplesPerSec_ != samplesPerSec)
    {
        nSamples_ = nSamples;
        nBytesPerSample_ = nBytesPerSample;
        nChannels_ = nChannels;
        samplesPerSec_ = samplesPerSec;

        if (audioSample_) delete []audioSample_;
        audioSample_ = new char[nSamples_*nBytesPerSample_*nChannels_];
    }
}

void aframe::SaveFrame(void* audioSample)
{
    if (audioSample_)
        memcpy(audioSample_, audioSample, nSamples_*nBytesPerSample_*nChannels_);
}

void aframe::CopyFrame(void* audioSample,
                       int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec)
{
    memcpy(audioSample, audioSample_, nSamples_*nBytesPerSample_*nChannels_);
}

bool MosaicAudioObserver::onRecordFrame(void *audioSample, int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec)
{
    audio_frame.CopyFrame(audioSample, nSamples, nBytesPerSample, nChannels, samplesPerSec);
    return true;
}

bool MosaicAudioObserver::onPlaybackFrame(void *audioSample, int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec)
{
    audio_frame.InitFrameBuffer(nSamples, nBytesPerSample, nChannels, samplesPerSec);
    audio_frame.SaveFrame(audioSample);
    return true;
}

bool MosaicAudioObserver::onPlaybackFrameUid(unsigned int uid, void *audioSample, int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec)
{
    return true;
}

