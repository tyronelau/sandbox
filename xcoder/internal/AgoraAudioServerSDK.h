//
//  AgoraAudioServerSDK.h
//  agorasdk
//
//  Created by sting feng on 2014-9-2.
//  Copyright (c) 2014 Agora Voice. All rights reserved.
//

#ifndef AGORA_AUDIO_SERVER_SDK_H
#define AGORA_AUDIO_SERVER_SDK_H
#include "AgoraAudioSDK.h"

class IAgoraAudioServer : public IAgoraAudio
{
public:
	// pull decoded data, in PCM format
	/*
	 * PARAMS
	 * nSamples: 80
	 * nBytesPerSample: 2
	 * nChannels: 1
	 * samplesPerSec: 8000
	 * audioSamples: [OUTPUT] buffer to receive audio data
	 * nSamplesOut: [OUTPUT] actual returned sample count
	 *
	 * RETURN
	 * zero indicates succeeded, non-zero indicates failure.
     */
    virtual int NeedMorePlayData(const unsigned int nSamples,
                                     const unsigned char nBytesPerSample,
                                     const unsigned char nChannels,
                                     const unsigned int samplesPerSec,
                                     void* audioSamples,
                                     unsigned int& nSamplesOut) = 0;

	// send raw audio data for encoding
	/*
	 * PARAMS
	 * audioSamples: audio buffer
	 * nSamples: 80
	 * nBytesPerSample: 2
	 * nChannels: 1
	 * samplesPerSec: 8000
	 *
	 * RETURN
	 * zero indicates succeeded, non-zero indicates failure.
	 */
    virtual int32_t RecordedDataIsAvailable(const void* audioSamples,
									uint32_t nSamples,
									uint8_t nBytesPerSample,
									uint8_t nChannels,
									uint32_t samplesPerSec) = 0;

    // NOTE(liuyong): the size of array |uidList| MUST BE NOT LESS THAN 4
    virtual int NeedMorePlayDataEx(unsigned int nSamples,
                                 unsigned char nBytesPerSample,
                                 unsigned char nChannels,
                                 unsigned int samplesPerSec,
                                 unsigned int *pActiveUsers,
                                 unsigned int uidList[],
                                 void *audioSamples) = 0;
    // Query Channel Status
    virtual uint32_t GetLastActiveTs() = 0;
};

class AgoraAudioServerParameters
{
public:
	AgoraAudioServerParameters(IAgoraAudio* engine)
		:m_engine(engine){}
	void setAllowedUdpPortRange(unsigned short minAllowedPort, unsigned short maxAllowedPort);
private:
	IAgoraAudio* m_engine;
};

extern "C" AGORA_API IAgoraAudioServer* createAgoraAudioServerInstance(IAgoraAudioEventHandler* pHandler);

#endif
