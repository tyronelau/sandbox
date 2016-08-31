#include "base/safe_log.h"
#include "libyuv/scale.h"
#include "xcoder/video_observer.h"

using namespace win_test;

vframe::vframe() :
    yBuffer_(NULL),
    uBuffer_(NULL),
    vBuffer_(NULL),
    width_(0),
    height_(0),
    yStride_(0),
    uStride_(0),
    vStride_(0)
{}


vframe::~vframe()
{
    if (yBuffer_) delete []yBuffer_;
    if (uBuffer_) delete []uBuffer_;
    if (vBuffer_) delete []vBuffer_;
}


void vframe::InitFrameBuffer(unsigned int width, unsigned int height,
                             unsigned int yStride, unsigned int uStride, unsigned int vStride)
{
    if (width_ != width || height_ != height)
    {
        width_ = width;
        height_ = height;
        yStride_ = yStride;
        uStride_ = uStride;
        vStride_ = vStride;

        if (yBuffer_) delete []yBuffer_;
        if (uBuffer_) delete []uBuffer_;
        if (vBuffer_) delete []vBuffer_;
        yBuffer_ = new unsigned char[yStride_*height_];
        uBuffer_ = new unsigned char[uStride_*height_ / 2];
        vBuffer_ = new unsigned char[vStride_*height_ / 2];
    }
}

void vframe::SaveFrame(unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer)
{
    libyuv::I420Scale(yBuffer, yStride_, uBuffer, uStride_, vBuffer, vStride_, width_, height_,
                      yBuffer_, yStride_, uBuffer_, uStride_, vBuffer_, vStride_,
                      width_, height_, libyuv::kFilterBox);
    //if (yBuffer_)
    //  memcpy(yBuffer_, yBuffer, yStride_*height_);
    //if (uBuffer_)
    //  memcpy(uBuffer_, uBuffer, uStride_*height_/2);
    //if (vBuffer_)
    //  memcpy(vBuffer_, vBuffer, vStride_*height_/2);
}

void vframe::CopyFrame(unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                       unsigned int yStride, unsigned int uStride, unsigned int vStride,
                       unsigned int yOffset, unsigned int uOffset, unsigned int vOffset,
                       unsigned int width, unsigned int height)
{
    for (int i = 0; i < height_; i++)
        memcpy(yBuffer + i*yStride + yOffset, yBuffer_ + i*yStride_, width_);
    for (int i = 0; i < height_/2; i++)
        memcpy(uBuffer + i*uStride + uOffset, uBuffer_ + i*uStride_, width_/2);
    for (int i = 0; i < height_/2; i++)
        memcpy(vBuffer + i*vStride + vOffset, vBuffer_ + i*vStride_, width_/2);

    //fill black
    for (int i = height_; i < height; i++)
        memset(yBuffer + i*yStride + yOffset, 0, width_);
    for (int i = height_/2; i < height/2; i++)
        memset(uBuffer + i*uStride + uOffset, 0x80, width_/2);
    for (int i = height_/2; i < height/2; i++)
        memset(vBuffer + i*vStride + vOffset, 0x80, width_/2);

}

void vframe::CopyFrameScaled(unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                             unsigned int yStride, unsigned int uStride, unsigned int vStride,
                             unsigned int yOffset, unsigned int uOffset, unsigned int vOffset,
                             unsigned int newWidth, unsigned int newHeight)
{
    libyuv::I420Scale(yBuffer_, yStride_, uBuffer_, uStride_, vBuffer_, vStride_, width_, height_,
                      yBuffer+yOffset, yStride, uBuffer+uOffset, uStride, vBuffer+vOffset, vStride,
                      newWidth, newHeight, libyuv::kFilterBox);
}

bool MosaicObserver::onCaptureVideoFrame(unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                                         unsigned int width, unsigned int height,
                                         unsigned int yStride, unsigned int uStride, unsigned int vStride)
{
    return true;
}

bool MosaicObserver::onRenderVideoFrame(unsigned int uid,
                                        unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                                        unsigned int width, unsigned int height,
                                        unsigned int yStride, unsigned int uStride, unsigned int vStride)
{
    std::map<uint32_t, vframe>::iterator it = frame_map.find(uid);
    if (it == frame_map.end()) {
        vframe frame;
        frame_map.insert(std::pair<uint32_t, vframe>(uid, frame));
    }

    it = frame_map.find(uid);
    if (it == frame_map.end()) {
        return false;
    }

    SAFE_LOG(DEBUG) << "Received user " << uid << ", width: " << width << ", height: " << height;

    pthread_mutex_lock( &cs_mutex );
    it->second.InitFrameBuffer(width, height, yStride, uStride, vStride);
    it->second.SaveFrame(yBuffer, uBuffer, vBuffer);
    it->second.active_ = true;

    pthread_mutex_unlock( &cs_mutex );
    return true;
}

bool MosaicObserver::onExternalVideoFrame(unsigned char* yBuffer, unsigned char* uBuffer, unsigned char* vBuffer,
                                          unsigned int width, unsigned int height,
                                          unsigned int yStride, unsigned int uStride, unsigned int vStride)
{
    if (width == 0)
        return false;

    if (mode_ == 0) {
        pthread_mutex_lock( &cs_mutex );

        int total_width = 0;
        std::map<uint32_t, vframe>::iterator it;
        for (it = frame_map.begin(); it != frame_map.end(); it++)
        {
            if (!it->second.active_)
                continue;

            if (it->second.height_ > height)
                break;

            if (total_width + it->second.width_ > width)
                break;

            it->second.CopyFrame(yBuffer, uBuffer, vBuffer, yStride, uStride, vStride, total_width, total_width / 2, total_width / 2, width, height);

            total_width += it->second.width_;
        }
        pthread_mutex_unlock( &cs_mutex );
    }

    if (mode_ == 1) {
        int total_width = 0;
        std::map<uint32_t, vframe>::iterator it = frame_map.begin();
        if (it == frame_map.end())
            return false;
        pthread_mutex_lock( &cs_mutex );
        int output_width = it->second.width_;
        int output_height = it->second.height_;
        for (it = frame_map.begin(); it != frame_map.end(); it++)
        {
            if (!it->second.active_)
                continue;

            if (total_width + output_width > width)
                break;

            it->second.CopyFrameScaled(yBuffer, uBuffer, vBuffer, yStride, uStride, vStride, total_width, total_width / 2, total_width / 2, output_width, output_height);

            total_width += output_width;
        }
        pthread_mutex_unlock( &cs_mutex );
    }

    if(mode_ == 2) {
        auto findIter = frame_map.find(master_uid_);
        if (findIter == frame_map.end() || !findIter->second.active_) {
            findIter = frame_map.begin();
            while (findIter != frame_map.end()) {
                if (findIter->second.active_) break;
                ++findIter;
            }
        }

        if (findIter == frame_map.end()) {
            return false;
        }

        pthread_mutex_lock(&cs_mutex);

        int output_width = findIter->second.width_;
        int output_height = findIter->second.height_;

        //copy the first
        findIter->second.CopyFrameScaled(yBuffer, uBuffer, vBuffer, yStride, uStride, vStride, 0, 0, 0, width, height);
        int scaled_width = output_width/3;
        int scaled_height = output_height/3;
        scaled_width = (scaled_width / 2) * 2; // must be an even number
        scaled_height = (scaled_height / 2) * 2; // must be an even number
        int yoffset = 0, uoffset = 0, voffset = 0;

        //copy the other three
        int total = 0;
        for (auto iter = frame_map.begin(); iter != frame_map.end(); ++iter) {
            if (iter == findIter || !iter->second.active_) {
                continue;
            }
            if (width >= height) {
              yoffset = total * scaled_height * yStride;
              uoffset = total * scaled_height / 2 * uStride;
              voffset = total * scaled_height / 2 * vStride;
            }
            else {
              yoffset = (height - scaled_height) * yStride + total * scaled_width;
              uoffset = (height - scaled_height) / 2 * uStride + total * scaled_width / 2;
              voffset = (height - scaled_height) / 2 * vStride + total * scaled_width / 2;
            }
            iter->second.CopyFrameScaled(yBuffer, uBuffer, vBuffer, yStride, uStride, vStride, yoffset, uoffset, voffset, scaled_width, scaled_height);

            ++total;
            if (total >= 3) {
                break;
            }
        }

        pthread_mutex_unlock(&cs_mutex);
    }
    return true;
}

void MosaicObserver::setActiveRender(unsigned int uid, bool active)
{
    std::map<uint32_t, vframe>::iterator it = frame_map.find(uid);
    if (it != frame_map.end()) {
        it->second.active_ = active;
        SAFE_LOG(INFO) << "User " << uid << (active ? " active" : " inactive");
    }
}

