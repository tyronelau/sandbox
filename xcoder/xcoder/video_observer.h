
#ifndef __CHE_UI_WIN_TEST_VIDEO_OBSERVER_H__
#define __CHE_UI_WIN_TEST_VIDEO_OBSERVER_H__


#include "common_types.h"
#include "chat_engine_i.h"
#include "pthread.h"
#include <map>


namespace win_test {


struct vframe
{
    vframe();
    ~vframe();
    void InitFrameBuffer(unsigned int width, unsigned int height,
                         unsigned int yStride, unsigned int uStride, unsigned int vStride);
    void SaveFrame(unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer);
    void CopyFrame(unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                   unsigned int yStride, unsigned int uStride, unsigned int vStride,
                   unsigned int yOffset, unsigned int uOffset, unsigned int vOffset,
                   unsigned int width, unsigned int height);
    void CopyFrameScaled(unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                         unsigned int yStride, unsigned int uStride, unsigned int vStride,
                         unsigned int yOffset, unsigned int uOffset, unsigned int vOffset,
                         unsigned int newWidth, unsigned int newHeight);
    unsigned char* yBuffer_;
    unsigned char* uBuffer_;
    unsigned char* vBuffer_;
    unsigned int width_;
    unsigned int height_;
    unsigned int yStride_;
    unsigned int uStride_;
    unsigned int vStride_;
    bool active_;
};


class MosaicObserver : public agora::media::IVideoFrameObserver
{
public:
    void setActiveRender(unsigned int uid, bool active);


    void setMode(uint32_t mode) { mode_ = mode; }


    /**
     * set master(host) user id.
     */
    void setMaster(uint32_t uid) { master_uid_ = uid; }


    virtual bool onCaptureVideoFrame(unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                                     unsigned int width, unsigned int height,
                                     unsigned int yStride, unsigned int uStride, unsigned int vStride);
    virtual bool onRenderVideoFrame(unsigned int uid,
                                    unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                                    unsigned int width, unsigned int height,
                                    unsigned int yStride, unsigned int uStride, unsigned int vStride);
    virtual bool onExternalVideoFrame(unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                                      unsigned int width, unsigned int height,
                                      unsigned int yStride, unsigned int uStride, unsigned int vStride);
private:
    uint32_t mode_;
    uint32_t master_uid_;
    std::map<uint32_t, vframe> frame_map;
    pthread_mutex_t cs_mutex =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
};

}

#endif //__CHE_UI_WIN_TEST_VIDEO_OBSERVER_H__
