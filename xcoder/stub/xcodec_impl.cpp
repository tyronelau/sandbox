#include "stub/xcodec_impl.h"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <vector>

#include "base/async_pipe.h"
#include "base/packer.h"
#include "base/safe_log.h"
#include "protocol/ipc_protocol.h"

namespace agora {
namespace xcodec {

using base::async_pipe_reader;
using base::async_pipe_writer;
using base::unpacker;

Recorder* Recorder::CreateRecorder(RecorderCallback *callback) {
  signal(SIGPIPE, SIG_IGN);
  return new RecorderImpl(callback);
}

RecorderImpl::RecorderImpl(RecorderCallback *callback) {
  joined_ = false;
  callback_ = callback;
}

RecorderImpl::~RecorderImpl() {
  if (reader_) {
    delete reader_;
  }

  if (writer_) {
    delete writer_;
  }

  process_.stop();
  process_.wait();
}

int RecorderImpl::JoinChannel(const char *vendor_key,
    const char *channel_name, bool is_dual, uint_t uid) {
  int reader_fds[2];
  if (pipe(reader_fds) != 0) {
    SAFE_LOG(ERROR) << "Failed to create a pipe: "
        << strerror(errno);
    return -1;
  }

  std::vector<const char *> args;
  args.push_back("xcoder");
  args.push_back("--key");
  args.push_back(vendor_key);
  args.push_back("--name");
  args.push_back(channel_name);

  args.push_back("--write");
  char write_str[16];
  snprintf(write_str, 16, "%d", reader_fds[0]);
  args.push_back(write_str);

  args.push_back("--read");
  char read_str[16];
  snprintf(read_str, 16, "%d", reader_fds[1]);
  args.push_back(read_str);

  if (is_dual) {
    args.push_back("--dual");
  }

  char uid_str[16];
  if (uid != 0) {
    args.push_back("--uid");
    snprintf(uid_str, 16, "%u", uid);
    args.push_back(uid_str);
  }

  args.push_back(NULL);

  base::process p;
  if (!p.start(&args[0], false, NULL, 0)) {
    close(reader_fds[0]);
    close(reader_fds[1]);
    return -1;
  }

  process_.swap(p);

  reader_ = new (std::nothrow)async_pipe_reader(&loop_, reader_fds[0], this);
  writer_ = new (std::nothrow)async_pipe_writer(&loop_, reader_fds[1], this);

  thread_ = std::thread(&RecorderImpl::run_internal, this);
  return 0;
}

int RecorderImpl::run_internal() {
  return loop_.run();
}

int RecorderImpl::LeaveChannel() {
  return leave_channel();
}

int RecorderImpl::leave_channel() {
  if (writer_) {
    protocol::leave_packet pkt;
    writer_->write_packet(pkt);
  }

  thread_.join();
  return 0;
}

bool RecorderImpl::on_receive_packet(async_pipe_reader *reader,
    unpacker &pkr, uint16_t uri) {
  switch (uri) {
  case protocol::AUDIO_FRAME_URI: {
    protocol::audio_frame frame;
    frame.unmarshall(pkr);
    on_audio_frame(std::move(frame));
    break;
  }
  case protocol::VIDEO_FRAME_URI: {
    protocol::video_frame frame;
    frame.unmarshall(pkr);
    on_video_frame(std::move(frame));
    break;
  }
  case protocol::USER_JOINED_URI: {
    protocol::user_joined joined;
    joined.unmarshall(pkr);
    on_user_joined(joined.uid);
    break;
  }
  case protocol::USER_DROPPED_URI: {
    protocol::user_dropped dropped;
    dropped.unmarshall(pkr);
    on_user_dropped(dropped.uid);
    break;
  }
  default: return false;
  }

  return true;
}

bool RecorderImpl::on_error(async_pipe_reader *reader, short events) {
  return false;
}

bool RecorderImpl::on_error(async_pipe_writer *writer, short events) {
  return false;
}

void RecorderImpl::on_audio_frame(protocol::audio_frame frame) {
  xcodec::AudioFrame t(frame.frame_ms, frame.rates, frame.samples);
  t.channels_ = 1;
  t.sample_bits_ = 16;
  t.buf_ = std::move(frame.data);

  if (callback_) {
    callback_->AudioFrameReceived(frame.uid, &t);
  }
}

void RecorderImpl::on_video_frame(protocol::video_frame frame) {
  xcodec::VideoFrame t(frame.frame_ms, frame.width, frame.height,
      frame.ystride, frame.ustride, frame.vstride);

  t.data_ = std::move(frame.data);
  t.ybuf_ = reinterpret_cast<uchar_t *>(&t.data_[0]);

  size_t offset = t.height_ * t.ystride_;
  t.ubuf_ = reinterpret_cast<uchar_t *>(&t.data_[offset]);

  offset += t.height_ * t.ustride_ / 2;
  t.vbuf_ = reinterpret_cast<uchar_t *>(&t.data_[offset]);

  if (callback_) {
    callback_->VideoFrameReceived(frame.uid, &t);
  }
}

void RecorderImpl::on_user_joined(uint32_t uid) {
  if (callback_) {
    callback_->RemoteUserJoined(uid);
  }
}

void RecorderImpl::on_user_dropped(uint32_t uid) {
  if (callback_) {
    callback_->RemoteUserDropped(uid);
  }
}

}
}

