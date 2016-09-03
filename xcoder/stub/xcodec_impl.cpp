#include "stub/xcodec_impl.h"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <vector>

#include "base/safe_log.h"

namespace agora {
namespace xcodec {

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
  snprintf(write_str, 16, "%d", fds[0]);
  args.push_back(write_str);

  args.push_back("--read");
  char read_str[16];
  snprintf(read_str, 16, "%d", fds[1]);
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
  if (!p.start(&args[0], false, skipped, 2)) {
    close(fds[0]);
    close(fds[1]);
    return -1;
  }

  process_.swap(p);

  reader_ = new (std::nothrow)async_pipe_reader(fds[0]);
  writer_ = new (std::nothrow)async_pipe_writer(fds[1]);

  thread_ = std::thread(&RecorderImpl::run_internal, this);
  return 0;
}

int RecorderImpl::run_internal() {
  while (!stopped_) {

  }
}

}
}
