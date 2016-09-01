//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <rtc/IAgoraRtcEngine.h>
#include <rtc/rtc_event.h>
#include <string>

#if defined(_WIN32)
extern HINSTANCE GetCurrentModuleInstance();
#elif defined(__ANDROID__) || defined(__linux__)
#include <dlfcn.h>
#endif

namespace agora {
  namespace commons {
    namespace network {
      struct network_info_t;
    }
    namespace cjson {
      class JsonWrapper;
    }
  }

namespace rtc {
    typedef agora::commons::cjson::JsonWrapper any_document_t;
    struct VideoNetOptions;

enum INTERFACE_ID_EX_TYPE
{
    AGORA_IID_RTC_ENGINE_EX = 0xacbd,
};

class IRtcEngineEventHandlerEx : public IRtcEngineEventHandler
{
public:
    virtual void onMediaEngineLoadSuccess() {}
    virtual void onMediaEngineStartCallSuccess() {}
    virtual void onAudioTransportQuality(uid_t uid, unsigned short delay, unsigned short lost) {
        (void)uid;
        (void)delay;
        (void)lost;
    }

    virtual void onVideoTransportQuality(uid_t uid, unsigned short delay, unsigned short lost) {
        (void)uid;
        (void)delay;
        (void)lost;
    }

    virtual void onRecap(const char* recapData, int length) {
        (void)recapData;
        (void)length;
    }

    virtual bool onEvent(RTC_EVENT evt, std::string* payload) {
        (void)evt;
        (void)payload;

        /* return false to indicate this event is not handled */
        return false;
    }

    virtual void onLogEvent(int level, const char* message, int length) {
        (void)level;
        (void)message;
        (void)length;
    }
	/**
	* when vendor message received, the function will be called
	* @param [in] uid
	*        UID of the remote user
	* @param [in] data
	*        the message data
	* @param [in] length
	*        the message length, in bytes
	*        frame rate
	*/
	virtual void onVendorMessage(uid_t uid, const char* data, size_t length) {
		(void)uid;
		(void)data;
		(void)length;
	}
};

struct RtcEngineContextEx {
    IRtcEngineEventHandler* eventHandler;
    bool isExHandler;
    const char* vendorKey;
	void* context;
	APPLICATION_CATEGORY_TYPE applicationCategory;
	std::string deviceId;
    std::string configDir;
    std::string dataDir;
	RtcEngineContextEx()
		:eventHandler(NULL)
		, isExHandler(false)
		, vendorKey(NULL)
		, context(NULL)
		, applicationCategory(APPLICATION_CATEGORY_COMMUNICATION)
	{}
};

struct WebAgentVideoStats {
	uid_t uid;
	int delay;
	int sentFrameRate;
	int renderedFrameRate;
	int skippedFrames;
};
class RtcContext;

//full feature definition of rtc engine interface
class IRtcEngineEx : public IRtcEngine
{
public:
    static const char* getSdkVersion(int* build);
public:
    virtual ~IRtcEngineEx() {}
    virtual void release2() = 0;
    virtual int initializeEx(const RtcEngineContextEx& context) = 0;
    virtual int setVideoCanvas(const VideoCanvas& canvas) = 0;
    virtual int setParameters(const char* parameters) = 0;
	/**
	* get multiple parameters.
	*/
	virtual int getParameters(const char* key, any_document_t& result) = 0;
    virtual int setProfile(const char* profile, bool merge) = 0;
    virtual int getProfile(any_document_t& result) = 0;
    virtual int notifyNetworkChange(agora::commons::network::network_info_t&& networkInfo) = 0;
	/**
	* enable the vendor message, sending and receiving
	* @return return 0 if success or an error code
	*/
	virtual int enableVendorMessage() = 0;

	/**
	* disable the vendor message, sending and receiving
	* @return return 0 if success or an error code
	*/
	virtual int disableVendorMessage() = 0;

	/**
	* broadcast the vendor message in the channel
	* @param [in] data
	*        the message data
	* @param [in] length
	*        the message length, in bytes
	* @return return 0 if success or an error code
	*/
	virtual int sendVendorMessage(const char* data, size_t length) = 0;
  virtual int sendReportMessage(const char* data, size_t length, int type) = 0;
  virtual int getOptionsByVideoProfile(int profile, VideoNetOptions& options) = 0;
  virtual RtcContext* getRtcContext() = 0;
  virtual int setLogCallback(bool enabled) = 0;
  virtual int makeQualityReportUrl(const char* channel, uid_t listenerUid, uid_t speakerUid, QUALITY_REPORT_FORMAT_TYPE format, agora::util::AString& url) = 0;

  /**
  * get SHA1 values of source files for building the binaries being used, for bug tracking.
  */
	virtual const char* getSourceVersion() const = 0;

	virtual int reportWebAgentVideoStats(const WebAgentVideoStats& stats) = 0;

	virtual void printLog(LOG_FILTER_TYPE level, const char* message) = 0;
};

class RtcEngineParametersEx : public RtcEngineParameters
{
public:
    RtcEngineParametersEx(IRtcEngine& engine)
        :RtcEngineParameters(engine)
    {}
    int setVideoResolution(int width, int height) {
        return setObject("che.video.local.resolution", "{\"width\":%d,\"height\":%d}", width, height);
    }
    int setVideoMaxBitrate(int bitrate) {
        return parameter()->setInt("che.video.local.max_bitrate", bitrate);
    }
    //call before joining channel
    int setVideoMaxFrameRate(int frameRate) {
        return setProfile("{\"audioEngine\":{\"maxVideoFrameRate\":%d}}", frameRate);
    }
    int enableAudioQualityIndication(bool enabled) {
        return parameter()->setBool("rtc.audio_quality_indication", enabled);
    }
    int enableTransportQualityIndication(bool enabled) {
        return parameter()->setBool("rtc.transport_quality_indication", enabled);
    }
    int enableRecap(int interval) {
        return parameter()->setInt("che.audio.recap.interval", interval);
    }
    int playRecap() {
        return parameter()->setBool("che.audio.recap.start_play", true);
    }
    int playAudioFile(const char* filePath) {
        if (!filePath || *filePath == '\0')
            return -ERR_INVALID_ARGUMENT;
        return parameter()->setString("che.audio.start_audio_file", filePath);
    }
    int playVideoFile(const char* filePath) {
        if (!filePath || *filePath == '\0')
            return -ERR_INVALID_ARGUMENT;
        return parameter()->setString("che.video.start_video_file", filePath);
    }
    // many players (up to 25): special settings in underlying engine
    int enableManyPlayers(bool enabled) {
        return parameter()->setBool("che.video.enableManyParties", enabled);
    }
    // request video engine to play specified stream.
    // streamType: 0 - large resolution, 1 - 1/4 resolution and 1/2 fps of large resolution
    int requestBitstreamType(unsigned int uid, unsigned int streamType) {
        if (streamType != 0 && streamType != 1)
            return -1;
        return setObject("che.video.setstream", "{\"uid\":%u,\"stream\":%d}", uid, streamType);
    }
protected:
    int setProfile(const char* format, ...) {
        char buf[512];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, sizeof(buf)-1, format, args);
        va_end(args);
        return parameter()->setProfile(buf, true);
    }
};

class RtcEngineLibHelper
{
	typedef const char* (AGORA_CALL *PFN_GetAgoraRtcEngineVersion)(int* build);
	typedef IRtcEngine* (AGORA_CALL *PFN_CreateAgoraRtcEngine)();
    typedef void (AGORA_CALL *PFN_ShutdownAgoraRtcEngineService)();
#if defined(_WIN32)
    typedef HINSTANCE lib_handle_t;
    static HINSTANCE MyLoadLibrary(const char* dllname)
    {
        char path[MAX_PATH];
        GetModuleFileNameA(GetCurrentModuleInstance(), path, MAX_PATH);
        auto p = strrchr(path, '\\');
        strcpy(p + 1, dllname);
        HINSTANCE hDll = LoadLibraryA(path);
        if (hDll)
            return hDll;

        hDll = LoadLibraryA(dllname);
        return hDll;
    }
#else
    typedef void* lib_handle_t;
#endif
public:
    RtcEngineLibHelper(const char* dllname)
        : m_firstInit(true)
        , m_lib(NULL)
        , m_dllName(dllname)
        , m_pfnCreateAgoraRtcEngine(nullptr)
		, m_pfnGetAgoraRtcEngineVersion(nullptr)
    {
    }

    bool initialize()
    {
        if (!m_firstInit)
            return isValid();
        m_firstInit = false;
#if defined(_WIN32)
        m_lib = MyLoadLibrary(m_dllName.c_str());
        if (m_lib)
        {
            m_pfnCreateAgoraRtcEngine = (PFN_CreateAgoraRtcEngine)GetProcAddress(m_lib, "createAgoraRtcEngine");
			m_pfnGetAgoraRtcEngineVersion = (PFN_GetAgoraRtcEngineVersion)GetProcAddress(m_lib, "getAgoraRtcEngineVersion");
        }
#elif defined(__ANDROID__) || defined(__linux__)
        m_lib = dlopen(m_dllName.c_str(), RTLD_LAZY);
        if (m_lib)
        {
            m_pfnCreateAgoraRtcEngine = (PFN_CreateAgoraRtcEngine)dlsym(m_lib, "createAgoraRtcEngine");
			m_pfnGetAgoraRtcEngineVersion = (PFN_GetAgoraRtcEngineVersion)dlsym(m_lib, "getAgoraRtcEngineVersion");
		}
#endif
        return isValid();
    }

    void deinitialize()
    {
        if (m_lib)
        {
#if defined(_WIN32)
            FreeLibrary(m_lib);
#elif defined(__ANDROID__) || defined(__linux__)
            dlclose(m_lib);
#endif
            m_lib = NULL;
        }
        m_pfnCreateAgoraRtcEngine = nullptr;
		m_pfnGetAgoraRtcEngineVersion = nullptr;
    }

    ~RtcEngineLibHelper()
    {
        deinitialize();
    }

    bool isValid()
    {
        return m_pfnCreateAgoraRtcEngine != NULL;
    }

    agora::rtc::IRtcEngine* createEngine()
    {
        return m_pfnCreateAgoraRtcEngine ? m_pfnCreateAgoraRtcEngine() : NULL;
    }
	const char* getVersion(int* build)
	{
		return m_pfnGetAgoraRtcEngineVersion ? m_pfnGetAgoraRtcEngineVersion(build) : nullptr;
	}
private:
    bool m_firstInit;
    lib_handle_t m_lib;
    std::string m_dllName;
    PFN_CreateAgoraRtcEngine m_pfnCreateAgoraRtcEngine;
	PFN_GetAgoraRtcEngineVersion m_pfnGetAgoraRtcEngineVersion;
};


}}
