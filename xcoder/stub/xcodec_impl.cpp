#include "stub/xcodec_impl.h"

#include <cassert>
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

using std::string;
using base::async_pipe_reader;
using base::async_pipe_writer;
using base::unpacker;

AudioFrame::AudioFrame(uint_t frame_ms, uint_t sample_rates, uint_t samples) {
  frame_ms_ = frame_ms;
  channels_ = 1;
  sample_bits_ = 16;
  sample_rates_ = sample_rates;
  samples_ = samples;
}

AudioFrame::~AudioFrame() {
}

VideoYuvFrame::VideoYuvFrame(uint_t frame_ms, uint_t width, uint_t height,
    uint_t ystride, uint_t ustride, uint_t vstride) {
  frame_ms_ = frame_ms;
  width_ = width;
  height_ = height;
  ystride_ = ystride;
  ustride_ = ustride;
  vstride_ = vstride;

  ybuf_ = NULL;
  ubuf_ = NULL;
  vbuf_ = NULL;
}

VideoYuvFrame::~VideoYuvFrame() {
}

Recorder* Recorder::CreateRecorder(RecorderCallback *callback) {
  signal(SIGPIPE, SIG_IGN);
  return new RecorderImpl(callback);
}

RecorderImpl::RecorderImpl(RecorderCallback *callback) {
  joined_ = false;
  stopped_ = false;
  callback_ = callback;

  reader_ = NULL;
  writer_ = NULL;
  timer_ = NULL;
}

RecorderImpl::~RecorderImpl() {
  if (reader_) {
    delete reader_;
  }

  if (writer_) {
    delete writer_;
  }

  if (timer_) {
    loop_.remove_timer(timer_);
  }

  process_.stop();
  process_.wait();
}

int RecorderImpl::Destroy() {
  delete this;
  return 0;
}

int RecorderImpl::JoinChannel(const char *vendor_key, const char *cname,
    bool is_dual, uint_t uid, const char *path_prefix) {
  int reader_fds[2];
  if (pipe(reader_fds) != 0) {
    SAFE_LOG(ERROR) << "Failed to create a pipe for read: "
        << strerror(errno);
    return -1;
  }

  int writer_fds[2];
  if (pipe(writer_fds) != 0) {
    SAFE_LOG(ERROR) << "Failed to create a pipe for write: "
        << strerror(errno);
    close(reader_fds[0]);
    close(reader_fds[1]);
    return -1;
  }

  std::string program("xcoder");
  if (path_prefix != NULL && strcmp(path_prefix, "")) {
    program = std::string(path_prefix) + "/" + program;
  }

  std::vector<const char *> args;
  args.push_back(program.c_str());
  args.push_back("--key");
  args.push_back(vendor_key);
  args.push_back("--name");
  args.push_back(cname);

  args.push_back("--write");
  char write_str[16];
  snprintf(write_str, 16, "%d", reader_fds[1]);
  args.push_back(write_str);

  args.push_back("--read");
  char read_str[16];
  snprintf(read_str, 16, "%d", writer_fds[0]);
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
  int skipped[2] = {writer_fds[0], reader_fds[1]};
  if (!p.start(&args[0], false, skipped, 2)) {
    close(reader_fds[0]);
    close(reader_fds[1]);
    close(writer_fds[0]);
    close(writer_fds[1]);
    return -1;
  }

  close(writer_fds[0]);
  close(reader_fds[1]);

  process_.swap(p);

  SAFE_LOG(INFO) << "Reading pipe: " << reader_fds[0];
  SAFE_LOG(INFO) << "Writing pipe: " << writer_fds[1];

  reader_ = new (std::nothrow)async_pipe_reader(&loop_, reader_fds[0], this);
  writer_ = new (std::nothrow)async_pipe_writer(&loop_, writer_fds[1], this);

  thread_ = std::thread(&RecorderImpl::run_internal, this);
  return 0;
}

void RecorderImpl::timer_callback(int fd, void *context) {
  (void)fd;

  RecorderImpl *p = reinterpret_cast<RecorderImpl *>(context);
  p->on_timer();
}

void RecorderImpl::on_timer() {
  if (stopped_) {
    loop_.stop();
  }
}

int RecorderImpl::run_internal() {
  timer_ = loop_.add_timer(500, &RecorderImpl::timer_callback, this);
  return loop_.run();
}

int RecorderImpl::LeaveChannel() {
  if (std::this_thread::get_id() == thread_.get_id()) {
    return leave_channel();
  }

  stopped_ = true;
  thread_.join();
  return 0;
}

int RecorderImpl::leave_channel() {
  if (writer_) {
    protocol::leave_packet pkt;
    writer_->write_packet(pkt);
  }

  loop_.stop();
  thread_.detach();
  return 0;
}

bool RecorderImpl::on_receive_packet(async_pipe_reader *reader,
    unpacker &pkr, uint16_t uri) {
  (void)reader;
  assert(reader == reader_);

  switch (uri) {
  case protocol::AUDIO_FRAME_URI: {
    protocol::audio_frame frame;
    frame.unmarshall(pkr);
    on_audio_frame(std::move(frame));
    break;
  }
  case protocol::YUV_FRAME_URI: {
    protocol::yuv_frame frame;
    frame.unmarshall(pkr);
    on_video_frame(std::move(frame));
    break;
  }
  case protocol::H264_FRAME_URI: {
    protocol::h264_frame frame;
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
  case protocol::RECORDER_ERROR_URI: {
    protocol::recorder_error error;
    error.unmarshall(pkr);
    on_recorder_error(error.error_code, error.reason);
    break;
  }
  default: return false;
  }

  return true;
}

bool RecorderImpl::on_error(async_pipe_reader *reader, short events) {
  (void)reader;
  (void)events;

  assert(reader == reader_);

  if (callback_) {
    callback_->RecorderError(-2, "Broken reading pipe");
  }
  return true;
}

bool RecorderImpl::on_error(async_pipe_writer *writer, short events) {
  (void)writer;
  (void)events;

  assert(writer == writer_);

  if (callback_) {
    callback_->RecorderError(-1, "Broken writing pipe");
  }
  return true;
}

void RecorderImpl::on_audio_frame(protocol::audio_frame frame) {
  xcodec::AudioFrame t(frame.frame_ms, frame.sample_rates, 160);
  t.channels_ = 1;
  t.sample_bits_ = 16;
  t.buf_ = std::move(frame.data);
  t.samples_ = static_cast<uint_t>(t.buf_.size() / 2);

  if (callback_) {
    callback_->AudioFrameReceived(frame.uid, &t);
  }
}

void RecorderImpl::on_video_frame(protocol::yuv_frame frame) {
  xcodec::VideoYuvFrame t(frame.frame_ms, frame.width, frame.height,
      frame.ystride, frame.ustride, frame.vstride);

  t.data_ = std::move(frame.data);
  t.ybuf_ = reinterpret_cast<uchar_t *>(&t.data_[0]);

  size_t offset = t.height_ * t.ystride_;
  t.ubuf_ = reinterpret_cast<uchar_t *>(&t.data_[offset]);

  offset += t.height_ * t.ustride_ / 2;
  t.vbuf_ = reinterpret_cast<uchar_t *>(&t.data_[offset]);

  if (callback_) {
    xcodec::VideoFrame frm;
    frm.type = xcodec::kRawYuv;
    frm.frame.yuv = &t;

    callback_->VideoFrameReceived(frame.uid, &frm);
  }
}

void RecorderImpl::on_video_frame(protocol::h264_frame frame) {
  xcodec::VideoH264Frame t;
  t.frame_ms = frame.frame_ms;
  t.frame_num = frame.frame_num;
  t.payload = std::move(frame.data);

  if (callback_) {
    xcodec::VideoFrame frm;
    frm.type = xcodec::kH264;
    frm.frame.h264 = &t;

    callback_->VideoFrameReceived(frame.uid, &frm);
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

void RecorderImpl::on_recorder_error(int32_t error_code, const string &reason) {
  if (callback_) {
    callback_->RecorderError(error_code, reason.c_str());
  }
}

}
}

