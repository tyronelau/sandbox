#ifndef __AGORAVOICE_CHAT_ENGINE_I_H__
#define __AGORAVOICE_CHAT_ENGINE_I_H__
#include <string>


// Usage example, omitting error checking:
//
//  using namespace webrtc;
//  IChatEngine* chatEngine = createChatEngine();
//	chatEngine->init();
//	chatEngine->registerTransport(...);
//  chatEngine->numOfCodecs();
//  chatEngine->setCodec(...);
//	chatEngine->setAecType(...);
//  chatEngine->setAgcStatus(...);
//	chatEngine->setNsStatus(...);
//	chatEngine->startCall(...);
//	then ITransport will receive payload,and you can packet is yourself
//  and send it to net;at the same time, you will receive packet from net,
//  and you should unpacket it and push it to 'chatEngine': chatEngine->receiveNetPacket(...);
//  when you finish call,you should call in the order:
//	chatEngine->stopCall();
//	chatEngine->deRegisterTransport();
//  chatEngine->terminate();
//  destoryChatEngine(chatEngine);	

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#define HELPER_DLL_IMPORT __declspec(dllimport)
#define HELPER_DLL_EXPORT __declspec(dllexport)
#define HELPER_DLL_LOCAL
#define CHAT_ENGINE_API __cdecl
#else
#include <stdint.h>
#if __GNUC__ >= 4
#define HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
#define HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
#define HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#define CHAT_ENGINE_API
#else
#define HELPER_DLL_IMPORT
#define HELPER_DLL_EXPORT
#define HELPER_DLL_LOCAL
#define CHAT_ENGINE_API
#endif
#endif


#ifdef AGORAVOICE_EXPORT
#define AGORAVOICE_DLLEXPORT HELPER_DLL_EXPORT
#elif AGORAVOICE_DLL
#define AGORAVOICE_DLLIMPORT HELPER_DLL_IMPORT
#else
#define AGORAVOICE_DLLEXPORT
#endif

namespace agora
{
namespace media
{

class IAudioTransport
{
public:
    virtual int sendAudioPacket(const unsigned char*  payloadData,unsigned short payloadSize, 
							int payload_type, int vad_type, unsigned int timeStamp, unsigned short seqNumber) = 0;
};

class IVideoTransport
{
public:
	virtual int sendVideoPacket(const void* packet, unsigned short packetLen) = 0;
	virtual int sendVideoRtcpPacket(unsigned int uid, const void* packet, unsigned short packetLen) = 0;
};

class IJitterStatistics
{
public:
	 virtual ~IJitterStatistics(){}
	 struct JitterStatistics{
		unsigned short delay;
		unsigned short currentExpandRate;
		unsigned int   maxJitterInMs;
		unsigned int consecutive2LostCnt;
		unsigned int   start_seq;
		unsigned int   end_seq;
		unsigned short duration;
	};
	struct SpeakerInfo {
		unsigned int uid;
		unsigned int volume; // [0,255]
	};
	virtual int GetJitterStatistics(int uid,JitterStatistics *p) = 0;
	virtual int GetRecapStatistics(unsigned char* recap_state, int length) = 0;
	virtual int GetJitterStatisticsByPkt(int uid, int64_t *val, unsigned  short *val1) = 0;
	virtual int GetSpeakersReport(SpeakerInfo* speakers, int speakerNumber, int mixVolume) = 0;
};
class IChatEngineObserver
{
public:
    enum EVENT_CODE
    {
        RECORDING_ERROR = 0,
        PLAYOUT_ERROR = 1,
        RECORDING_WARNING = 2,
        PLAYOUT_WARNING = 3
    };
	enum DEVICE_STATE_TYPE {
		DEVICE_STATE_ACTIVE = 1,
		DEVICE_STATE_DISABLED = 2,
		DEVICE_STATE_NOT_PRESENT = 4,
		DEVICE_STATE_UNPLUGGED = 8
	};
	enum DEVICE_TYPE {
		UNKNOWN_DEVICE = -1,
		PLAYOUT_DEVICE = 0,
		RECORDING_DEVICE = 1
	};
public:
	virtual void OnChatEngineEvent(int evtCode) = 0;
	virtual void OnAudioDeviceStateChange(const char deviceId[128], int deviceType, int newState) = 0;
};

class ICaptureStatistics
{
public:
	virtual ~ICaptureStatistics(){}

	// Return the microphone capture waveform
	virtual int GetCaptureWaveform(short *wave, int length) = 0;
};

#define EVENT_JITTER_COUNT 5
struct ChatEngineEventData {
    int overruns;
    int underruns;
    unsigned int input_jitters[EVENT_JITTER_COUNT];
    unsigned int output_jitters[EVENT_JITTER_COUNT];
    int lastError;
};
ChatEngineEventData& GetEngineEventData();

class IAudioEngine
{
public:
    enum AEC_TYPE
    {
        UNCHANGED_AEC_TYPE = 0,
        DEFAULT_AEC_TYPE,
        CONFERENCE_AEC_TYPE,
        AEC_AEC_TYPE,
        AECM_AEC_TYPE,
        NONE_AEC_TYPE
    };

    enum AUDIO_OUTPUT_ROUTING
    {
        HEADSET = 0,
        EARPIECE,
        HEADSETWITHOUTMIC,
        SPEAKERPHONE,
        LOUDSPEAKER
    };

    enum AUDIO_LAYERS
    {
        kAudioPlatformDefault = 0,
        kAudioWindowsWave = 1,
        kAudioWindowsCore = 2,
        kAudioLinuxAlsa = 3,
        kAudioLinuxPulse = 4
    };

    enum DTX_MODE
    {
        DTX_OFF = 0,		//no dtx
        DTX_ON_ZERO_PAYLOAD = 1,			//dtx enabled, zero payload package sent when silience
        DTX_ON_PACK_NOT_SEND = 2	//dtx enabled, not sending package when silience
    };

	virtual ~IAudioEngine(){}
  // 
	virtual int init() = 0;
	virtual int terminate() = 0;
    virtual int registerTransport(IAudioTransport* transport) = 0;
    virtual int deRegisterTransport() = 0;
    virtual int receiveNetPacket(unsigned int uid, const unsigned char*  payloadData,unsigned short payloadSize,
                            int payload_type, unsigned int timeStamp, unsigned short seqNumber) = 0;

    // ?? trivial
    virtual int registerNetEQStatistics(IJitterStatistics* neteq_statistics) = 0;
    virtual int deRegisterNetEQStatistics() = 0;

    // ?? trivial
    virtual int registerChatEngineObserver(IChatEngineObserver* observer) = 0;
    virtual int deRegisterChatEngineObserver() = 0;

    // ?? trivial
    virtual int registerCaptureStatistics(ICaptureStatistics* capture_statistics) = 0;
    virtual int deRegisterCaptureStatistics() = 0;

    // |sid| is the channel id to join
    // NOTE(liuyong): Actually, for the real audio chat engine,
    // the real prototype of |startCall| takes no arguments, but my fake chat engine
    // requires the sid to make a unique recording file name for serializing
    // received network packets.
    //
    // For C calling conventions, it is safe to pass more arguments
    // than expected, to a funtion.
    virtual int startCall(unsigned int sid) = 0;
    // virtual int startCall() = 0;

    virtual int stopCall() = 0;
    virtual bool isCalling() = 0;

	// expose for Linux
    virtual int RecordedDataIsAvailable(const void* audioSamples,
                                        unsigned int nSamples,
                                        unsigned char nBytesPerSample,
                                        unsigned char nChannels,
                                        unsigned int samplesPerSec) = 0;

    virtual int NeedMorePlayData(unsigned int nSamples,
                                 unsigned char nBytesPerSample,
                                 unsigned char nChannels,
                                 unsigned int samplesPerSec,
                                 void* audioSamples,
                                 unsigned int& nSamplesOut) = 0;

    virtual int NeedMorePlayDataEx(unsigned int nSamples,
                                 unsigned char nBytesPerSample,
                                 unsigned char nChannels,
                                 unsigned int samplesPerSec,
                                 unsigned int *pActiveUsers,
                                 unsigned int uidList[],
                                 void* audioSamples) = 0;
#ifdef _WIN32
    // mic and spk testing before making the call
    virtual int startSpeakerTest(const char* filename) = 0;
    virtual int stopSpeakerTest() = 0;
    virtual int startMicrophoneTest(int reportIntervalMs) = 0;
    virtual int stopMicrophoneTest() = 0;
#endif

    // Use wave file as input
    virtual int startFileAsInput(const char* filename) = 0;

    //codec-setting functions
    virtual int numOfCodecs() = 0;
    virtual int getCodecInfo(int index, char* pInfoBuffer, int infoBufferLen) = 0;
    virtual int setCodec(int index) = 0;
    virtual int setMultiFrameInterleave(int num_frame, int num_interleave) = 0;
    virtual int setREDStatus(bool enable) = 0;
    virtual int setDTXStatus(DTX_MODE dtx_mode) = 0;

    //apm functions
    virtual int setAecType(AEC_TYPE type) = 0;
    virtual int setAudioOutputRouting(AUDIO_OUTPUT_ROUTING routing) = 0;
    virtual int setAgcStatus(bool enable) = 0;
    virtual int setNsStatus(bool enable) = 0;
    virtual int getJitterBufferMaxMetric() = 0;
    //debug recording
    virtual int startDebugRecording(const char* filename) = 0;
    virtual int stopDebugRecording() = 0;

    // start recap playing
    virtual int startRecapPlaying() = 0;
    // enable/disable recap
    // reportIntervalMs designates the callback interval in milliseconds, 0 means disable
    virtual int setRecapStatus(int reportIntervalMs) = 0;

    // Sets the NetEQ max and min playout delay.
    virtual int setNetEQMaximumPlayoutDelay(int delayMs) = 0;
    virtual int setNetEQMinimumPlayoutDelay(int delayMs) = 0;

    virtual int setSpeakerStatus(bool enable) = 0;
    virtual int getPlayoutTS(unsigned int uid,int *playoutTS) = 0;

    // enable/disable volume/level report
    // reportIntervalMs: designates the callback interval in milliseconds, 0 means disable
    // smoothFactor: decides the smoothness of the returned volume, [0, 10], default is 10
    virtual int setSpeakersReportStatus(int reportIntervalMs, int smoothFactor) = 0;

    // enabel/disable waveform report
    virtual int setCaptureWaveformStatus(int reportIntervalMs, int nSamplesPer10ms) = 0;

    // enable/disable recording during the call
    virtual int startCallRecording(const char* filename) = 0;
    virtual int stopCallRecording() = 0;
    virtual int startDecodeRecording(const char* filename) = 0;
    virtual int stopDecodeRecording() = 0;

    // mute or unmute
    virtual int setMuteStatus(bool enable) = 0;

public:
    // Sets the type of audio device layer to use.
    virtual int setAudioDeviceLayer(AUDIO_LAYERS audioLayer) = 0;

    // Gets the number of audio devices available for recording.
    virtual int getNumOfRecordingDevices(int& devices) = 0;

    // Gets the number of audio devices available for playout.
    virtual int getNumOfPlayoutDevices(int& devices) = 0;

    // Gets the name of a specific recording device given by an |index|.
    // On Windows Vista/7, it also retrieves an additional unique ID
    // (GUID) for the recording device.
    virtual int getRecordingDeviceName(int index, char name[128],
                                       char deviceId[128]) = 0;

    // Gets the name of a specific playout device given by an |index|.
    // On Windows Vista/7, it also retrieves an additional unique ID
    // (GUID) for the playout device.
    virtual int getPlayoutDeviceName(int index, char name[128],
                                     char deviceId[128]) = 0;

    // Checks if the sound card is available to be opened for recording.
    virtual int getRecordingDeviceStatus(bool& isAvailable) = 0;

    // Checks if the sound card is available to be opened for playout.
    virtual int getPlayoutDeviceStatus(bool& isAvailable) = 0;

    // Sets the audio device used for recording.
    virtual int setRecordingDevice(int index) = 0;
    virtual int setRecordingDevice(const char deviceId[128]) = 0;

    // Sets the audio device used for playout.
    virtual int setPlayoutDevice(int index) = 0;
    virtual int setPlayoutDevice(const char deviceId[128]) = 0;

    // Gets current selected audio device used for recording.
    virtual int getCurrentRecordingDevice(char deviceId[128]) = 0;

    // Gets current selected audio device used for playout.
    virtual int getCurrentPlayoutDevice(char deviceId[128]) = 0;
};

class IVideoEngine
{
public:
    // Raw video types
    enum RawVideoType
    {
        kVideoI420     = 0,
        kVideoYV12     = 1,
        kVideoYUY2     = 2,
        kVideoUYVY     = 3,
        kVideoIYUV     = 4,
        kVideoARGB     = 5,
        kVideoRGB24    = 6,
        kVideoRGB565   = 7,
        kVideoARGB4444 = 8,
        kVideoARGB1555 = 9,
        kVideoMJPEG    = 10,
        kVideoNV12     = 11,
        kVideoNV21     = 12,
        kVideoBGRA     = 13,
        kVideoUnknown  = 99
    };

    // Video codec types
    enum VideoCodecType
    {
        kVideoCodecVP8,
        kVideoCodecH264,
        kVideoCodecI420,
        kVideoCodecRED,
        kVideoCodecULPFEC,
        kVideoCodecGeneric,
        kVideoCodecUnknown
    };

    enum RotateCapturedFrame
    {
        RotateCapturedFrame_0 = 0,
        RotateCapturedFrame_90 = 90,
        RotateCapturedFrame_180 = 180,
        RotateCapturedFrame_270 = 270
    };



    // This structure describes one set of the supported capabilities for a capture
    // device.
    struct CaptureCapability {
        unsigned int width;
        unsigned int height;
        unsigned int maxFPS;
        int rawType;
        int codecType;
        unsigned int expectedCaptureDelay;
        bool interlaced;
        CaptureCapability() {
            width = 0;
            height = 0;
            maxFPS = 0;
            rawType = kVideoI420;
            codecType = kVideoCodecUnknown;
            expectedCaptureDelay = 0;
            interlaced = false;
        }
    };

	virtual ~IVideoEngine(){}
	virtual int init() = 0;
	virtual int terminate() = 0;
    virtual int receiveNetPacket(unsigned int uid, const void* packet, unsigned int packetSize) = 0;
    virtual int receiveRtcpPacket(unsigned int uid, const void* packet, unsigned int packetSize) = 0;
    virtual int registerTransport(IVideoTransport* transport) = 0;
    virtual int deregisterTransport() = 0;

    // Controls
    virtual int startCapture(CaptureCapability* ability = NULL) = 0;
    virtual int stopCapture() = 0;
    virtual bool isCapturing() = 0;
    virtual int setRotateCapturedFrames(int rotation) = 0;
    virtual int startRemoteRender(void* window,
        const unsigned int z_order, const float left,
        const float top, const float right,
        const float bottom) = 0;
    virtual int stopRemoteRender() = 0;
    virtual bool isRemoteRendering() = 0;

    virtual int startLocalRender(void* window,
        const unsigned int z_order, const float left,
        const float top, const float right,
        const float bottom) = 0;
    virtual int stopLocalRender() = 0;

    // Video codecs
    virtual int numOfCodecs() = 0;
    virtual int getCodecInfo(int index, char* pInfoBuffer, int infoBufferLen) = 0;
    virtual int numOfCodecSizes() = 0;
    virtual int getCodecSizeInfo(int index, char* pInfoBuffer, int infoBufferLen) = 0;
    virtual int setCodec(int index,
                              unsigned short width = 352,
                              unsigned short height = 288) = 0;
    // Set the target bitrate and frame rate of the video codec
    virtual int setCodecRates(unsigned int target_bitrate_bps, unsigned int frame_rate) = 0;

    // Gets the number of available capture devices.
    virtual int numberOfCaptureDevices() = 0;
    // Gets the name and unique id of a capture device.
    virtual int getCaptureDevice(unsigned int list_number,
        char* device_nameUTF8,
        const unsigned int device_nameUTF8Length,
        char* unique_idUTF8,
        const unsigned int unique_idUTF8Length) = 0;
    virtual int setCaptureDevice(int index) = 0;
};


class IChatEngine
{
public:
	virtual void release() = 0;
	virtual int init() = 0;
	virtual int terminate() = 0;
  virtual int startCall(unsigned int sid) = 0;

  virtual int stopCall() = 0;
  virtual bool isCalling() = 0;
  // important
  virtual int setParameters(const char* parameters) = 0;
  virtual int getParameters(const char* parameters, char* buffer, size_t* length) = 0;
  virtual IAudioEngine* getAudioEngine() = 0;
  virtual IVideoEngine* getVideoEngine() = 0;
  // ?? trivial
  virtual int getEngineEvents(ChatEngineEventData& eventData) = 0;
};

class ITraceCallback
{
public:
  virtual void Print(int level, const char* message, int length) = 0;
};

class ITraceService
{
public:
	virtual void release() = 0;
	virtual bool startTrace(const char* traceFile, int maxFileSize, unsigned int filter) = 0;
	virtual bool startTrace(ITraceCallback* callback, unsigned int filter) = 0;
	virtual void stopTrace() = 0;
	virtual void setTraceFile(const char* traceFile, int maxFileSize) = 0;
	virtual void setTraceCallback(ITraceCallback* callback) = 0;
	virtual void setTraceFilter(unsigned int filter) = 0;
	virtual void addTrace(int level, int module, int id, const char* msg) = 0;
};

} //namespace media
} //namespace agora


#ifdef __cplusplus
extern "C"
{
#endif

AGORAVOICE_DLLEXPORT agora::media::IChatEngine* CHAT_ENGINE_API createChatEngine(const char* profile, void* context);
AGORAVOICE_DLLEXPORT void CHAT_ENGINE_API destroyChatEngine(agora::media::IChatEngine* chatEngine);
AGORAVOICE_DLLEXPORT agora::media::ITraceService* CHAT_ENGINE_API createTraceService();
AGORAVOICE_DLLEXPORT const char* findChatEngineProfile(const char* deviceId);

#ifdef __cplusplus
}
#endif

#endif // __AGORAVOICE_CHAT_ENGINE_I_H__
