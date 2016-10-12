#include "xcoder/audio_observer.h"

#include <cassert>
#include <new>

#include "base/safe_log.h"
#include "base/time_util.h"
#include "protocol/ipc_protocol.h"

namespace agora {
namespace recording {

audio_observer::audio_observer(base::event_queue<frame_ptr_t> *frame_queue) {
  queue_ = frame_queue;
}

bool audio_observer::onRecordFrame(void *audioSample, int nSamples,
    int nBytesPerSample, int nChannels, int samplesPerSec) {
  (void)audioSample;
  (void)nSamples;
  (void)nBytesPerSample;
  (void)nChannels;
  (void)samplesPerSec;

  assert(false);

  // This function will not be called for receiving-only mode.
  return true;
}

bool audio_observer::onPlaybackFrame(void *audioSample, int nSamples,
    int nBytesPerSample, int nChannels, int samplesPerSec) {
  (void)audioSample;
  (void)nSamples;
  (void)nBytesPerSample;
  (void)nChannels;
  (void)samplesPerSec;

  protocol::audio_frame *frame = new protocol::audio_frame;
  frame->uid = 0;
  frame->frame_ms = static_cast<uint32_t>(base::now_ms());
  frame->channels = 1;
  frame->bits = 16;
  frame->sample_rates = samplesPerSec;

  size_t size = nSamples * nBytesPerSample * nChannels;
  frame->data.assign(reinterpret_cast<const char *>(audioSample), size);

  queue_->push(frame_ptr_t(frame));
  return true;
}

bool audio_observer::onPlaybackFrameUid(unsigned int uid, void *audioSample,
    int nSamples, int nBytesPerSample, int nChannels, int samplesPerSec) {
  (void)uid;
  (void)audioSample;
  (void)nSamples;
  (void)nBytesPerSample;
  (void)nChannels;
  (void)samplesPerSec;

  protocol::audio_frame *frame = new protocol::audio_frame;
  frame->uid = uid;
  frame->frame_ms = static_cast<uint32_t>(base::now_ms());
  frame->channels = 1;
  frame->bits = 16;
  frame->sample_rates = samplesPerSec;

  size_t size = nSamples * nBytesPerSample * nChannels;
  frame->data.assign(reinterpret_cast<const char *>(audioSample), size);

  queue_->push(frame_ptr_t(frame));
  return true;
}

}
}
