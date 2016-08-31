#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "base/log.h"
#include "base/process.h"

namespace agora {
namespace base {
bool create_process(const char *const exec_args[], bool inherit_fd,
    bool wait, ExitState *ret_code, const int skipped_fds[], int len) {
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
    int status;
    waitpid(pid, &status, 0);
    if (wait && ret_code) {
      if (WIFEXITED(status)) {
        ret_code->exit_kind = ExitState::kExitCode;
        ret_code->exit_code = WEXITSTATUS(status);
      } else if (WIFSIGNALED(status)) {
        ret_code->exit_kind = ExitState::kSignal;
        ret_code->signal_no = WTERMSIG(status);
      }
    }
    return true;
  }

  // pid MUST be zero, new process
  if (!wait) {
    // A trick to work around the zombie process.
    pid = fork();
    if (pid < -1) {
      LOG(FATAL, "fork() failed! %s", strerror(errno));
      exit(-1);
    }

    if (pid > 0) _exit(0);
  }

  // close all fds except stdin/out/err
  if (!inherit_fd) {
    std::set<int> skipped{0, 1, 2};
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
  LOG(FATAL, "Failed to call execvp(%s): %s", exec_args[0], strerror(errno));
  exit(-100);
  return false; // never goes here.
}

bool create_process(const char *exec_cmd, bool inherit_fd, bool wait,
    ExitState *ret_code, const int *skipped, int len)  {
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

  return create_process(&exec_args[0], inherit_fd, wait, ret_code, skipped, len);
}

bool get_executable_path(char exec_path[], int max_len) {
  int r = readlink("/proc/self/exe", exec_path, max_len);
  if (r >= max_len) {
    LOG(ERROR, "Insufficient memory: %d", max_len);
    return false;
  }

  exec_path[r] = '\0';
  return true;
}

}
}

