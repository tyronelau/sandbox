#include "base/process.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <sstream>
#include <unordered_set>
#include <utility>
#include <vector>

#include "base/safe_log.h"

namespace agora {
namespace base {

process::process() {
  process_fd_ = -1;
  state_.kind = exit_state::kExitCode;
  state_.exit_code = -1;
}

process::~process() {
  if (process_fd_ > 0) {
    int status = -1;

    // hangs until the process exits
    waitpid(process_fd_, &status, 0);
    (void)status;
  }
}

bool process::is_stopped(exit_state *state) {
  if (process_fd_ <= 0) {
    if (state) {
      state->kind = exit_state::kInvalid;
    }
    return true;
  }

  int ret = -1;
  int status = -1;
  if ((ret = waitpid(process_fd_, &status, WNOHANG)) > 0) {
    process_fd_ = -1;
    if (WIFEXITED(status)) {
      state_.kind = exit_state::kExitCode;
      state_.exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
      state_.kind = exit_state::kSignal;
      state_.signal_no = WTERMSIG(status);
    }

    if (state) {
      *state = state_;
    }

    return true;
  }

  if (ret < 0) {
    LOG(ERROR, "waitpid failed: %s", strerror(errno));
  }

  return false;
}

bool process::stop() {
  return stop_internal(SIGABRT);
}

bool process::terminate() {
  return stop_internal(SIGKILL);
}

bool process::stop_internal(int sig) {
  if (process_fd_ <= 0) {
    SAFE_LOG(WARN) << "Stop an non-existed process: " << process_fd_;
    return true;
  }

  if (!kill(process_fd_, sig)) {
    SAFE_LOG(ERROR) << "Failed to stop the process: " << process_fd_
        << ", signal: " << sig;
    return false;
  }

  return true;
}

void process::swap(process &rhs) {
  std::swap(process_fd_, rhs.process_fd_);
  std::swap(state_, rhs.state_);
}

bool process::start(const char *const exec_args[], bool inherit_fd,
    const int *skipped_fds, int len, void (*error)(int, void *),
    void *context) {
  (void)error;
  (void)context;

  if (exec_args[0] == NULL) {
    LOG(ERROR, "No arugments!");
    return false;
  }

  int pid = fork();
  if (pid < 0) {
    LOG(FATAL, "fork() failed! %s", strerror(errno));
    return false;
  }

  if (pid > 0) {
    process_fd_ = pid;
    return true;
  }

  // close all fds except stdin/out/err
  if (!inherit_fd) {
    // char filename[256];
    // snprintf(filename, 256, "/tmp/%d-%s.log", getpid(), exec_args[0]);

    // int fd = -1;
    // if ((fd = open(filename, O_CREAT | O_WRONLY, 0666)) != -1) {
    //   dup2(fd, STDOUT_FILENO);
    //   dup2(fd, STDERR_FILENO);
    //   if (fd > STDERR_FILENO) close(fd);
    // } else {
    //   LOG(ERROR, "Failed to create the file: %s, %s", filename, strerror(errno));
    // }

    int fd = -1;
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
      dup2(fd, STDIN_FILENO);
      dup2(fd, STDOUT_FILENO);
      dup2(fd, STDERR_FILENO);
      if (fd > STDERR_FILENO) close(fd);
    }

    std::unordered_set<int> skipped{STDOUT_FILENO, STDERR_FILENO};
    for (int i = 0; i < len; ++i)
      skipped.insert(skipped_fds[i]);

    DIR *dir = opendir("/proc/self/fd");
    if (dir != NULL) {
      int ret = 0;
      struct dirent *result = NULL;
      struct dirent entry;
      std::vector<int> fds;
      while ((ret = readdir_r(dir, &entry, &result)) == 0) {
        if (result == NULL) break;
        int fd = atoi(entry.d_name);
        if (fd > 2 && skipped.find(fd) == skipped.end()) {
          fds.push_back(fd);
        }
      }

      if (ret != 0)
        LOG(ERROR, "Error occurs during enumerating file descriptors, %s",\
            strerror(errno));
      for (size_t i = 0; i < fds.size(); ++i)
        LOG_IF(ERROR, close(fds[i]) == -1, "Failed to close fd: %d", fds[i]);
    } else {
      LOG(ERROR, "Failed to open the self fd dir, %s", strerror(errno));
    }
  }

  execvp(const_cast<char*>(exec_args[0]), const_cast<char *const *>(&exec_args[0]));
  int err_code = errno;

  LOG(FATAL, "Failed to call execvp(%s): %s", exec_args[0], strerror(err_code));

  if (error) {
    (*error)(err_code, context);
  }

  usleep(100 * 1000);
  _Exit(-100);
  return false; // never goes here.
}

bool process::start(const char *exec_cmd, bool inherit_fd,
    const int *skipped, int len, void (*error)(int, void *),
    void *context)  {
  std::istringstream sin(exec_cmd);
  std::string arg;
  std::vector<std::string> args;
  while (sin >> arg) {
    args.push_back(arg);
  }

  if (args.empty()) {
    LOG(ERROR, "No arguments for executing! %s", exec_cmd);
    return false;
  }

  std::vector<char *> exec_args(args.size() + 1, NULL);
  for (size_t i = 0; i < args.size(); ++i) {
    exec_args[i] = &args[i][0];
  }

  return start(&exec_args[0], inherit_fd, skipped, len, error, context);
}

bool process::wait() {
  if (process_fd_ <= 0) {
    return true;
  }

  int ret = -1;
  int status = -1;
  if ((ret = waitpid(process_fd_, &status, 0)) > 0) {
    process_fd_ = -1;
    if (WIFEXITED(status)) {
      state_.kind = exit_state::kExitCode;
      state_.exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
      state_.kind = exit_state::kSignal;
      state_.signal_no = WTERMSIG(status);
    }

    return true;
  }

  if (ret < 0) {
    LOG(ERROR, "waitpid failed: %s", strerror(errno));
  }

  return false;
}

}
}
