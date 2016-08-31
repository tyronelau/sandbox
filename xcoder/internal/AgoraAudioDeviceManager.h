//
//  AgoraAudioDeviceManager.h
//  agorasdk
//
//  Created by sting feng on 2014-10-22.
//  Copyright (c) 2014 Agora Voice. All rights reserved.
//

#ifndef AGORA_AUDIO_DEVICE_MANAGER_H
#define AGORA_AUDIO_DEVICE_MANAGER_H
#include "AgoraAudioSDK.h"
#include <vector>
#include <string>

class AgoraAudioDeviceCollection : public IAgoraAudioDeviceCollection
{
	struct DeviceInfo {
		int index;
		std::string name;
		std::string id;
	};
public:
	AgoraAudioDeviceCollection(IAgoraAudioDeviceManager* pAudioDeviceManager, bool isPlayoutDevice);
	virtual int getCount();
	virtual int getDevice(int index, int& deviceIndex, char deviceName[MAX_AUDIO_DEVICE_NAME_LENGTH], char deviceId[MAX_AUDIO_DEVICE_GUID_LENGTH]);
	virtual int setDevice(const char deviceId[MAX_AUDIO_DEVICE_GUID_LENGTH]);
	virtual int setDevice(int deviceIndex);
	virtual void release();
public:
	void addDevice(int index, const char* name, const char* guid);
private:
	IAgoraAudioDeviceManager* m_pAudioDeviceManager;
	bool m_isPlayoutDevice;
	std::vector<DeviceInfo> m_deviceList;
};

class AgoraAudioDeviceManager : public IAgoraAudioDeviceManager
{
public:
	AgoraAudioDeviceManager();
	virtual IAgoraAudioDeviceCollection* enumeratePlayoutDevices();
	virtual IAgoraAudioDeviceCollection* enumerateRecordingDevices();
	virtual int setPlayoutDevice(int index);
	virtual int setPlayoutDevice(const char deviceId[MAX_AUDIO_DEVICE_GUID_LENGTH]);
	virtual int setRecordingDevice(int index);
	virtual int setRecordingDevice(const char deviceId[MAX_AUDIO_DEVICE_GUID_LENGTH]);
	virtual int startSpeakerTest(const char* audioFileName);
	virtual int stopSpeakerTest();
	virtual int startMicrophoneTest(int reportInterval);
	virtual int stopMicrophoneTest();
private:
	int doGetIntParameter(const char* name, int defValue);
	int doGetNumOfDevice(int& devices, bool isPlayout);
	int doGetDeviceName(int index, char deviceName[MAX_AUDIO_DEVICE_NAME_LENGTH], char deviceId[MAX_AUDIO_DEVICE_GUID_LENGTH], bool isPlayout);
private:
	int setParameters(const char* parameters);
	int setBooleanParameter(const char* module, const char* name, bool value);
	int setIntegerParameter(const char* module, const char* name, int value);
	int setStringParameter(const char* module, const char* name, const char* value);
private:
	IAgoraAudioDeviceCollection* doEnumerateAudioDevices(bool isPlayoutDevice);
};

#endif
