#pragma once

#include <memory>

#include "internal/chat_engine_i.h"
#include "base/event_queue.h"

namespace agora {
namespace base {
class packet;
};

namespace xcodec {

class audio_observer : public media::IAudioFrameObserver {
  typedef std::unique_ptr<base::packet> frame_ptr_t;
 public:
  explicit audio_observer(base::event_queue<frame_ptr_t> *frame_queue);

  virtual bool onRecordAudioFrame(AudioFrame &audioFrame);
  virtual bool onPlaybackAudioFrame(AudioFrame &audioFrame);
  virtual bool onPlaybackAudioFrameBeforeMixing(unsigned int uid,
      AudioFrame &audioFrame);
 private:
  base::event_queue<frame_ptr_t> *queue_;
};

}
}
