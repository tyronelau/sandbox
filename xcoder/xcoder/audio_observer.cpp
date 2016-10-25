#include "xcoder/audio_observer.h"

#include <cassert>
#include <climits>
#include <new>

#include "base/safe_log.h"
#include "base/time_util.h"
#include "protocol/ipc_protocol.h"

namespace agora {
namespace xcodec {

audio_observer::audio_observer(base::event_queue<frame_ptr_t> *frame_queue) {
  queue_ = frame_queue;
}

bool audio_observer::onRecordAudioFrame(AudioFrame &frame) {
  (void)frame;

  assert(false);

  // This function will not be called for receiving-only mode.
  return true;
}

bool audio_observer::onPlaybackAudioFrame(AudioFrame &frame) {
  const void *buffer = frame.buffer;
  int nSamples = frame.samples;
  int nBytesPerSample = frame.bytesPerSample;
  int nChannels = frame.channels;
  int samplesPerSec = frame.samplesPerSec;

  // static FILE *fp = fopen("temp2.pcm", "wb");

  protocol::pcm_frame *f = new protocol::pcm_frame;
  f->uid = 0;
  f->frame_ms = static_cast<uint32_t>(base::now_ms());
  f->channels = static_cast<uint8_t>(nChannels);
  f->bits = static_cast<uint8_t>(nBytesPerSample * CHAR_BIT);
  f->sample_rates = samplesPerSec;

  size_t size = nSamples * nBytesPerSample * nChannels;
  f->data.assign(reinterpret_cast<const char *>(buffer), size);

  // fwrite(buffer, 1, size, fp);

  queue_->push(frame_ptr_t(f));
  return true;
}

bool audio_observer::onPlaybackAudioFrameBeforeMixing(unsigned int uid,
    AudioFrame &frame) {
  const void *buffer = frame.buffer;
  int nSamples = frame.samples;
  int nBytesPerSample = frame.bytesPerSample;
  int nChannels = frame.channels;
  int samplesPerSec = frame.samplesPerSec;

  protocol::pcm_frame *f = new protocol::pcm_frame;
  f->uid = uid;
  f->frame_ms = static_cast<uint32_t>(base::now_ms());
  f->channels = static_cast<uint8_t>(nChannels);
  f->bits = static_cast<uint8_t>(nBytesPerSample * CHAR_BIT);
  f->sample_rates = samplesPerSec;

  size_t size = nSamples * nBytesPerSample * nChannels;
  f->data.assign(reinterpret_cast<const char *>(buffer), size);

  queue_->push(frame_ptr_t(f));
  return true;
}

}
}
