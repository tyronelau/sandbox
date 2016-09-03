#pragma once

#if __cplusplus >= 201101L && !defined(NULL)
#define NULL nullptr
#elif !defined(NULL)
#define NULL 0
#endif

namespace agora {
namespace commons {

// exit_state records exit status of the invoked process
// kExitCode means that process calls exit() or return from main(),
// otherwise, that process terminated with a signal No. |signal_no|.
struct exit_state {
  enum exit_kind {kInvalid, kExitCode, kSignal};
  exit_kind kind;
  union {
    int signal_no;
    int exit_code;
  };
};

// |exec_cmd|:
//   the full command line, like "./a.exe -n 1 -c 2"
// |inherit_fd|:
//   true, means that child process will inherit the fds of parent process,
//   otherwise, not.
// |wait|:
//   true means create_process will not return until the child process returns.
//     Once returns, |*ret_code| contains the exit state of child process.
//   false: create_process will return immediately.
// Return value:
//   true means the fork() call successes. Note: If you want to know whether
//   a child process starts up, you need to read the log.
// Note:
//   If you want to invoke a process like this "./a.exe \"seperated arguments\" -n 1",
//   just use the second form of create_process:
//      const char *args[] = {"./a.exe", "seperated arguments", "-n", "1", NULL};
//      create_process(args, ...);
//   Donnot forget the last argument NULL!!

class process {
 public:
  process();
  ~process();

  bool start(const char *cmd, bool inherit_fd=false,
      const int *skipped_fds=NULL, int len=0);

  bool start(const char * const args[], bool inherit_fd=false,
      const int *skipped_fds=NULL, int len=0);

  bool stop();
  bool terminate();

  void swap(process &rhs);

  // timely calls this function
  // this function is mutable because we need to update the internal state.
  bool is_stopped(exit_state *state);
 private:
  process(const process &);
  process& operator=(const process &);

  bool is_started() const;
  bool stop_internal(int sig);
 private:
  int process_fd_;
  exit_state state_;
};

}
}
