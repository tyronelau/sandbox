#ifndef __AGORAVOICE_CHAT_ENGINE_I_H__
#define __AGORAVOICE_CHAT_ENGINE_I_H__
#include <string>


// Usage example, omitting error checking:
//
//  using namespace AgoraRTC;
//  IChatEngine* chatEngine = createChatEngine();
//  chatEngine->init();
//  chatEngine->registerTransport(...);
//  chatEngine->numOfCodecs();
//  chatEngine->setCodec(...);
//  chatEngine->setAecType(...);
//  chatEngine->setAgcStatus(...);
//  chatEngine->setNsStatus(...);
//  chatEngine->startCall(...);
//  then ITransport will receive payload,and you can packet is yourself
//  and send it to net;at the same time, you will receive packet from net,
//  and you should unpacket it and push it to 'chatEngine': chatEngine->receiveNetPacket(...);
//  when you finish call,you should call in the order:
//  chatEngine->stopCall();
//  chatEngine->deRegisterTransport();
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
namespace util
{
template<class T>
class AutoPtr {
  typedef T value_type;
  typedef T* pointer_type;
public:
  AutoPtr(pointer_type p=0)
    :ptr_(p)
  {}
  ~AutoPtr() {
    if (ptr_)
      ptr_->release();
  }
  operator bool() const { return ptr_ != (pointer_type)0; }
  value_type& operator*() const {
    return *get();
  }

  pointer_type operator->() const {
    return get();
  }

  pointer_type get() const {
    return ptr_;
  }

  pointer_type release() {
    pointer_type tmp = ptr_;
    ptr_ = 0;
    return tmp;
  }

  void reset(pointer_type ptr = 0) {
    if (ptr != ptr_ && ptr_)
      ptr_->release();
    ptr_ = ptr;
  }
private:
  AutoPtr(const AutoPtr&);
  AutoPtr& operator=(const AutoPtr&);
private:
  pointer_type ptr_;
};
class IString {
public:
  virtual bool empty() const = 0;
  virtual const char* c_str() = 0;
  virtual const char* data() = 0;
  virtual size_t length() = 0;
  virtual void release() = 0;
};
typedef AutoPtr<IString> AString;
}

class IAudioTransport
{
public:
    virtual int sendAudioPacket(const unsigned char*  payloadData,unsigned short payloadSize,
                                int payload_type, int vad_type, unsigned int timeStamp, unsigned short seqNumber) = 0;
};

enum VideoStreamType {
  MASTER_VIDEO_STREAM = 0,
  LOW_BITRATE_VIDEO_STREAM = 1,
  MEDIUM_BITRATE_VIDEO_STREAM = 2,
  LIVE_VIDEO_STREAM = 3,
};

class IVideoListener
{
public:
    struct LocalVideoStreamStat
    {
      int width;
      int height;
      unsigned int prevFrameNumber;
      int sentBytes;
      int sentFrames;
      int sentQP;
      int sentPkgNumber;

      void reset() {
        sentBytes = 0;
        sentFrames = 0;
        sentQP = 0;
        sentPkgNumber = 0;
      }

      LocalVideoStreamStat() {
        prevFrameNumber = 0;
        width = 640;
        height = 360;
        reset();
      }

    };
    struct LocalVideoStat
    {
      // local stat
      LocalVideoStreamStat highStream;
      LocalVideoStreamStat lowStream;
      int sentRtt;
      int sentLoss;
      int sentTargetBitRate;
      unsigned int encodeTimeMs;
      int captureWidth;
      int captureHeight;
      int captureFrames;
      int encoderRecvFrames;
      int encodedFrames;
      int encodeFailFrames;
      int encoderSkipFrames;
      int duration;
      int fecLevel;
      int estimateBandwidth;
      unsigned int maxFrameOutInterval;

      void reset() {
        highStream.reset();
        lowStream.reset();
        sentRtt = 0;
        sentLoss = 0;
        sentTargetBitRate = 0;
        encodeTimeMs = 0;
        captureWidth = 0;
        captureHeight = 0;
        captureFrames = 0;
        encoderRecvFrames = 0;
        encodedFrames = 0;
        encodeFailFrames = 0;
        encoderSkipFrames = 0;
        duration = 0;
        fecLevel = 0;
        estimateBandwidth = 0;
        maxFrameOutInterval = 0;
      }
    };

    struct RemoteVideoStat
    {
      // remote stat
      int uid;
      int delay;
      int renderedFrames;
      int receivedBytes;
      int recvPkgNumber;
      int width;
      int height;
      int lossAfterFec;
      int decodeFailedFrames;
      int streamType;
      int decodedFrames;
      int rendererRecvFrames;
      int decodeTimeMs;
      int duration;
      unsigned int maxRenderInterval;
      unsigned int lastRenderMs;

      void reset() {
        uid = 0;
        delay = 0;
        width = 0;
        height = 0;
        renderedFrames = 0;
        receivedBytes = 0;
        lossAfterFec = 0;
        decodeFailedFrames = 0;
        streamType = 0;
        decodedFrames = 0;
        rendererRecvFrames = 0;
        decodeTimeMs = 0;
        recvPkgNumber = 0;
        duration = 0;
        maxRenderInterval = 0;
      }
    };

    //    #define VIDEO_ENGINE_FLAG_KEY_FRAME 0x00000080
    //    #define VIDEO_ENGINE_FLAG_FEC 0x00000040
    //    #define VIDEO_ENGINE_FLAG_LIVE 0x00000020
    //    #define VIDEO_ENGINE_FLAG_B_FRAME 0x00000010
    //    #define VIDEO_ENGINE_FLAG_STD_CODEC 0x00000008
    #define VIDEO_ENGINE_FLAG_HARDWARE_ENCODE 0x00000004
    #define VIDEO_ENGINE_FLAG_MULTI_STREAM 0x00000001

    enum VideoFrameType {
      KEY_FRAME = 0,
      DELTA_FRAME = 1,
      B_FRAME = 2,
    };
    enum VideoCodecType {
      VIDEO_CODEC_VP8 = 1,
      VIDEO_CODEC_H264 = 2,
      VIDEO_CODEC_EVP = 3,
      VIDEO_CODEC_E264 = 4,
    };
    struct PacketInfo
    {
      const void* packet;
      unsigned short packetLen;
      unsigned int frame_num;
      unsigned char frame_pkg_num;
      unsigned char pkg_seq_in_frame;
      int codec_type;
      VideoFrameType frame_type;
      VideoStreamType stream_type;
      unsigned int flag;
      int version;
    };

    virtual int sendVideoPacket(const PacketInfo& info) = 0;
    virtual int sendVideoRtcpPacket(unsigned int uid, const void* packet, unsigned short packetLen) = 0;
    virtual void onRemoteFirstFrameDrawed(int viewIndex, unsigned int uid, int width, int height) = 0;
    virtual void onLocalFirstFrameDrawed(int width, int height) = 0;
    virtual void onRemoteFirstFrameDecoded(unsigned int uid, int width, int height) = 0;
    virtual void onRemoteVideoInterrupted(unsigned int uid, unsigned int elapse_time) = 0;
    virtual void onVideoStat(const LocalVideoStat& localStat, RemoteVideoStat* remoteStat, int remoteCount) = 0;
    virtual void switchVideoStream(unsigned int uid, VideoStreamType stream) = 0;
    virtual void onBandWidthLevelChanged(int level) = 0;
    virtual void onVideoViewSizeChanged(int userID, int newWidth, int newHeight){};
};

class IVideoFrameObserver
{
public:
    virtual bool onCaptureVideoFrame(unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                                    unsigned int width, unsigned int height,
                                    unsigned int yStride, unsigned int uStride, unsigned int vStride) = 0;
    virtual bool onRenderVideoFrame(unsigned int uid, int rotation,
                                    unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                                    unsigned int width, unsigned int height,
                                    unsigned int yStride, unsigned int uStride, unsigned int vStride) = 0;
    virtual bool onExternalVideoFrame(unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                                      unsigned int width, unsigned int height,
                                      unsigned int yStride, unsigned int uStride, unsigned int vStride) = 0;
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
        unsigned short fecSavedLossRate;
        unsigned short duration;

        unsigned short recvPacketNumber;
        unsigned short recvFrameNumber;
        unsigned short recvExpiredFrameNumber;
        unsigned short toDecodeFrameNumber;
        unsigned short decodedFrameNumber;
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
        AUDIO_PLAYOUT_DEVICE = 0,
        AUDIO_RECORDING_DEVICE = 1,
        VIDEO_RENDER_DEVICE = 2,
        VIDEO_CAPTURE_DEVICE = 3
    };
public:
    virtual void OnChatEngineEvent(int evtCode) = 0;
    virtual void OnChatEngineError(int err) = 0;
    virtual void OnChatEngineWarning(int warn) = 0;
    virtual void OnChatEngineStatus(int statusCode) = 0; // use it to record running status (act as a simplified log system)
    virtual void OnAudioDeviceStateChange(const char deviceId[128], int deviceType, int newState) = 0;
    virtual void OnVideoDeviceStateChange(const char deviceId[128], int deviceType, int newState) = 0;
};

class IAudioFrameObserver
{
public:
  virtual bool onRecordFrame(void *audioSample, int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec) = 0;
  virtual bool onPlaybackFrame(void *audioSample, int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec) = 0;
  virtual bool onPlaybackFrameUid(unsigned int uid, void *audioSample, int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec) = 0;
};

#define EVENT_JITTER_COUNT 5
struct ChatEngineEventData {
    int overruns;
    int underruns;
    unsigned int input_jitters[EVENT_JITTER_COUNT];
    unsigned int output_jitters[EVENT_JITTER_COUNT];
    int lastError;
    int apm_buffer;
    int aec_delay;
    int large_echo_prob;
    int saturation_prob;
    int signal_level;
    int howling_state;
    int audio_engine_stat[3];
    int audio_route;
    int magic_id;

  int audio_record_rate;
  int audio_playout_rate;
  int audio_read_in_size;
  int audio_read_out_size;
  int audio_send_frame_rate;
  int audio_send_packet_rate;
  int audio_recv_packet_rate;
  unsigned int encoding_time;
  unsigned int decoding_time;

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
        DTX_OFF = 0,    //no dtx
        DTX_ON_ZERO_PAYLOAD = 1,      //dtx enabled, zero payload package sent when silience
        DTX_ON_PACK_NOT_SEND = 2  //dtx enabled, not sending package when silience
    };

    enum ReturnCodes {
        kOK = 0,
        kFail = -1,
        KPacketExpire = 1,
    };

  // mode 0: host interactive mode, 1: host solo mode, 2: receive mode
  enum SPECIFIC_MODE
  {
    kModeHostInteractive = 0,
    kModeHostSolo = 1,
    kModeAudience = 2,
  };

    virtual ~IAudioEngine(){}
    virtual int init() = 0;
    virtual int terminate() = 0;
    virtual int registerTransport(IAudioTransport* transport) = 0;
    virtual int deRegisterTransport() = 0;
    virtual int receiveNetPacket(unsigned int uid, const unsigned char*  payloadData,unsigned short payloadSize,
                            int payload_type, unsigned int timeStamp, unsigned short seqNumber) = 0;

    virtual int registerNetEQStatistics(IJitterStatistics* neteq_statistics) = 0;
    virtual int deRegisterNetEQStatistics() = 0;

    virtual int registerChatEngineObserver(IChatEngineObserver* observer) = 0;
    virtual int deRegisterChatEngineObserver() = 0;

    virtual int startCall() = 0;
    virtual int stopCall() = 0;
    virtual bool isCalling() = 0;
    virtual bool isCallInterrupted() = 0;

#ifdef _WIN32
    // mic and spk testing before making the call
    virtual int startSpeakerTest(const char* filename) = 0;
    virtual int stopSpeakerTest() = 0;
    virtual int startMicrophoneTest(int reportIntervalMs) = 0;
    virtual int stopMicrophoneTest() = 0;
#endif

    // Use meida file as input
    virtual int startPlayoutNearendAudioFile(const char* filename, int loopCount) = 0;

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

    // Sets the NetEQ max and min playout delay for a specific UID.
    virtual int setNetEQMaxPlayoutDelayWithUid(unsigned int uid, int delayMs) = 0;
    virtual int setNetEQMinPlayoutDelayWithUid(unsigned int uid, int delayMs) = 0;

    virtual int setSpeakerStatus(bool enable) = 0;
    virtual int getPlayoutTS(unsigned int uid,int *playoutTS) = 0;

    // enable/disable volume/level report
    // reportIntervalMs: designates the callback interval in milliseconds, 0 means disable
    // smoothFactor: decides the smoothness of the returned volume, [0, 10], default is 10
    virtual int setSpeakersReportStatus(int reportIntervalMs, int smoothFactor) = 0;

    // enable/disable recording during the call
    virtual int startCallRecording(const char* filename) = 0;
    virtual int stopCallRecording() = 0;
    virtual int startDecodeRecording(const char* filename, int bitrate = 24000) = 0;
    virtual int stopDecodeRecording() = 0;

    // mute or unmute
    virtual int setMuteStatus(bool mute) = 0;

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
    enum RawVideoType {
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
    enum VideoCodecType {
        kVideoCodecVP8 = 0,
        kVideoCodecH264 = 1,
        kVideoCodecI420 = 2,
        kVideoCodecRED = 3,
        kVideoCodecULPFEC = 4,
        kVideoCodecGeneric = 5,
        kVideoCodecEvp = 6,
        kVideoCodecE264 = 7,
        kVideoCodecUnknown = 99
    };

    enum RotateCapturedFrame {
        RotateCapturedFrame_0 = 0,
        RotateCapturedFrame_90 = 90,
        RotateCapturedFrame_180 = 180,
        RotateCapturedFrame_270 = 270
    };

    enum VideoFecMode {
      FEC_NORM = 0,
      FEC_KEY = 1,
    };

    typedef enum tagVideoViewLayoutType {
        LAYOUT_UNKNOWN = 0,
        LAYOUT_MIXED = 1, // large/small windows, request master/slave streams
        LAYOUT_FOUR_VIEWS = 2, // divided into 4 views, request third stream
        LAYOUT_N_SMALL_VIEWS = 3, // multiple views with same size, request master stream
    } VideoViewLayoutType;

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
    virtual int receiveNetPacket(unsigned int uid, const IVideoListener::PacketInfo& info) = 0;
    virtual int receiveRtcpPacket(unsigned int uid, const void* packet, unsigned int packetSize) = 0;
    virtual int setListener(IVideoListener* listener) = 0;
    virtual int deregisterTransport() = 0;
    virtual void checkUserOffline() = 0;

    // Controls
    virtual int startCapture(CaptureCapability* ability = NULL) = 0;
    virtual int stopCapture() = 0;
    virtual bool isCapturing() = 0;
    virtual int setRotateCapturedFrames(int rotation, int devRotate) = 0;
    virtual int startRemoteRender(unsigned int uid) = 0;
    virtual int stopRemoteRender() = 0;
    virtual bool isRemoteRendering() = 0;

    virtual int startLocalRender() = 0;
    virtual int stopLocalRender() = 0;

    // Video codecs
    virtual int numOfCodecs() = 0;
    virtual int getCodecInfo(int index, char* pInfoBuffer, int infoBufferLen) = 0;
    virtual int numOfCodecSizes() = 0;
    virtual int getCodecSizeInfo(int index, char* pInfoBuffer, int infoBufferLen) = 0;
    virtual int setCodec(int index, unsigned short width, unsigned short height) = 0;
    virtual int setCodec(VideoCodecType type) = 0;
    virtual int setCodecResolution(unsigned short width, unsigned short height) = 0;

    // Set the maximum bitrate of video encoder
    virtual int setMaxVideoBitrate(unsigned int max_bitrate_bps) = 0;
    virtual unsigned int getSendTargetBitRate() = 0;
    virtual int RequestEncodeKeyFrame() = 0;

    // Set the target bitrate and frame rate of the video codec
    virtual int setCodecRates(unsigned int target_bitrate_bps, unsigned int frame_rate) = 0;

    // Request the sender with 'uid' to encode and send a key frame
    virtual int RequestSendKeyFrame(unsigned int uid) = 0;

    // Gets the number of available capture devices.
    virtual int numberOfCaptureDevices() = 0;

    // Gets the name and unique id of a capture device.
    virtual int getCaptureDevice(unsigned int list_number,
      char* device_nameUTF8,
      const unsigned int device_nameUTF8Length,
      char* unique_idUTF8,
      const unsigned int unique_idUTF8Length) = 0;

    virtual int setCaptureDevice(int index) = 0;

    virtual int setCaptureDevice(const char *deviceId) = 0;
    virtual int setChannelProfileType(int type) = 0;
    virtual int requestEncodeStreamByUser(unsigned int uid, VideoStreamType type) = 0;

    // NOTE(liuyong): for live video only
    virtual int setVideoMinimumPlayout(unsigned int uid, int playoutMs) = 0;
};

class IChatEngine
{
public:
    enum CHANNEL_PROFILE_TYPE
    {
      CHANNEL_PROFILE_FREE = 0,
      CHANNEL_PROFILE_BROADCASTER = 1,
      CHANNEL_PROFILE_AUDIENCE = 2,
    };

    virtual void release() = 0;
    virtual int init(IChatEngineObserver* observer) = 0;
    virtual int initVideo() = 0;
    virtual int terminate() = 0;
    virtual int startCall() = 0;
    virtual int stopCall() = 0;
    virtual bool isCalling() = 0;
    virtual int setParameters(const char* parameters) = 0;
    virtual int getParameters(const char* parameters, char* buffer, size_t* length) = 0;
    virtual IAudioEngine* getAudioEngine() = 0;
    virtual IVideoEngine* getVideoEngine() = 0;
    virtual int getEngineEvents(ChatEngineEventData& eventData) = 0;
    virtual int setBoolParameter(const char* key, bool value) = 0;
    virtual int setIntParameter(const char* key, int value) = 0;
    virtual int setNumberParameter(const char* key, double value) = 0;
    virtual int setStringParameter(const char* key, const char* value) = 0;
    virtual int setObjectParameter(const char* key, const char* value) = 0;
    virtual int getParameter(const char* key, const char* args, util::AString& result) = 0;
    virtual int setChannelProfileType(CHANNEL_PROFILE_TYPE type) = 0;
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
    virtual void flushTrace() = 0;
};

class IVideoFrame
{
public:
  enum PLANE_TYPE {
    Y_PLANE = 0,
    U_PLANE = 1,
    V_PLANE = 2,
    NUM_OF_PLANES = 3
  };
  enum VIDEO_TYPE {
    VIDEO_TYPE_UNKNOWN = 0,
    VIDEO_TYPE_I420 = 1,
    VIDEO_TYPE_IYUV = 2,
    VIDEO_TYPE_RGB24 = 3,
    VIDEO_TYPE_ABGR = 4,
    VIDEO_TYPE_ARGB = 5,
    VIDEO_TYPE_ARGB4444 = 6,
    VIDEO_TYPE_RGB565 = 7,
    VIDEO_TYPE_ARGB1555 = 8,
    VIDEO_TYPE_YUY2 = 9,
    VIDEO_TYPE_YV12 = 10,
    VIDEO_TYPE_UYVY = 11,
    VIDEO_TYPE_MJPG = 12,
    VIDEO_TYPE_NV21 = 13,
    VIDEO_TYPE_NV12 = 14,
    VIDEO_TYPE_BGRA = 15,
    VIDEO_TYPE_RGBA = 16,
  };
  virtual void release() = 0;
  virtual const unsigned char* buffer(PLANE_TYPE type) const = 0;

  // Copy frame: If required size is bigger than allocated one, new buffers of
  // adequate size will be allocated.
  // Return value: 0 on success ,-1 on error.
  virtual int copyFrame(IVideoFrame** dest_frame) const = 0;

  // Convert frame
  // Input:
  //   - src_frame        : Reference to a source frame.
  //   - dst_video_type   : Type of output video.
  //   - dst_sample_size  : Required only for the parsing of MJPG.
  //   - dst_frame        : Pointer to a destination frame.
  // Return value: 0 if OK, < 0 otherwise.
  // It is assumed that source and destination have equal height.
  virtual int convertFrame(VIDEO_TYPE dst_video_type, int dst_sample_size, unsigned char* dst_frame) const = 0;

  // Get allocated size per plane.
  virtual int allocated_size(PLANE_TYPE type) const = 0;

  // Get allocated stride per plane.
  virtual int stride(PLANE_TYPE type) const = 0;

  // Get frame width.
  virtual int width() const = 0;

  // Get frame height.
  virtual int height() const = 0;

  // Get frame timestamp (90kHz).
  virtual unsigned int timestamp() const = 0;

  // Get render time in milliseconds.
  virtual int64_t render_time_ms() const = 0;

  // Return true if underlying plane buffers are of zero size, false if not.
  virtual bool IsZeroSize() const = 0;
};

class IExternalVideoRenderCallback
{
public:
  virtual void onViewSizeChanged(int width, int height) = 0;
  virtual void onViewDestroyed() = 0;
};

struct ExternalVideoRenerContext
{
  IExternalVideoRenderCallback* renderCallback;
  void* view;
  int renderMode;
  int zOrder;
  float left;
  float top;
  float right;
  float bottom;
};

class IExternalVideoRender
{
public:
  virtual void release() = 0;
  virtual int initialize() = 0;
  virtual int deliverFrame(const IVideoFrame& videoFrame, int videoRotate, int deviceRotate) = 0;
};

class IExternalVideoRenderFactory
{
public:
  virtual IExternalVideoRender* createRenderInstance(const ExternalVideoRenerContext& context) = 0;
};
} //namespace media
} //namespace agora


#ifdef __cplusplus
extern "C"
{
#endif

// Video source: external
class VideoSource
{
public:
    VideoSource() { mFuncOutputYuv = NULL; mReceiver = NULL; }
    virtual ~VideoSource() {}
    // output
    void (* mFuncOutputYuv)(void *receiver, void *nv21,
    int width, int height,
    int rotation, long long timestamp);
    void *mReceiver;
};
AGORAVOICE_DLLEXPORT void registerVideoSource(VideoSource *src);
// Video preprocessor: process yuv frame before encoder
AGORAVOICE_DLLEXPORT void registerVideoPreProcessor(void *routine, void *param);
AGORAVOICE_DLLEXPORT int registerVideoFrameObserver(agora::media::IVideoFrameObserver *observer);
AGORAVOICE_DLLEXPORT int registerAudioFrameObserver(agora::media::IAudioFrameObserver *observer);
AGORAVOICE_DLLEXPORT const char* getChatEngineVersion();
AGORAVOICE_DLLEXPORT const char* getChatEngineSourceVersion();
AGORAVOICE_DLLEXPORT const char* getChatEngineVersionEx(int* build);
AGORAVOICE_DLLEXPORT agora::media::IChatEngine* CHAT_ENGINE_API createChatEngine(const char* profile, void* context);
AGORAVOICE_DLLEXPORT void CHAT_ENGINE_API destroyChatEngine(agora::media::IChatEngine* chatEngine);
AGORAVOICE_DLLEXPORT agora::media::ITraceService* CHAT_ENGINE_API createTraceService();
AGORAVOICE_DLLEXPORT const char* findChatEngineProfile(const char* deviceId);

// Video renderer view management
// pending a view for user 'uid' (following userManagerBindUserView will bind it)
AGORAVOICE_DLLEXPORT void CHAT_ENGINE_API  userManagerPendingView(void *priv,
                        void *view, unsigned int uid);
// bind 'view' to user 'uid', and set render mode; does not open/close video
AGORAVOICE_DLLEXPORT void CHAT_ENGINE_API userManagerBindUserView(unsigned int uid, void *view, int renderMode, void *priv);
// clean all views from renderers; does not send message to VideoEngine (suppose engine is gone)
AGORAVOICE_DLLEXPORT void CHAT_ENGINE_API userManagerClearUserViews(void *priv);
AGORAVOICE_DLLEXPORT void CHAT_ENGINE_API setVideoRenderFactory(agora::media::IExternalVideoRenderFactory* factory);

typedef void (*RtmpCB)(const char *url, unsigned int type, unsigned int data);
AGORAVOICE_DLLEXPORT void CHAT_ENGINE_API setRtmpCallback(RtmpCB cb);

#ifdef __cplusplus
}
#endif

#endif // __AGORAVOICE_CHAT_ENGINE_I_H__
