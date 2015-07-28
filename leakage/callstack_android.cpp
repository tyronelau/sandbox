#include "callstack_info.h"
#include <dlfcn.h>
#include <unwind.h>

typedef struct {
  uintptr_t absolute_pc;     /* absolute PC offset */
  uintptr_t stack_top;       /* top of stack for this frame */
  size_t stack_size;         /* size of this stack frame */
} backtrace_frame_t;

typedef ssize_t (*unwindFn)(backtrace_frame_t* backtrace, size_t ignore_depth, size_t max_depth);

static _Unwind_Reason_Code unwind_callback(struct _Unwind_Context *context,
    void *arg) {
  callstack *pbt = reinterpret_cast<callstack*>(arg);
  uintptr_t pc = _Unwind_GetIP(context);

  if (pc) {
    if (pbt->count == kMaxTraceCount) {
      return _URC_END_OF_STACK;
    } else {
      pbt->stacks[pbt->count++] = pc;
    }
  }

  return _URC_NO_REASON;
}

void get_callstack(size_t n, callstack *pbt) {
  pbt->alloc_size = n;
  pbt->count = 0;

  // static void *gHandle = NULL;
  // static unwindFn unwind_backtrace = NULL;
  // const char so_path[] = "/system/lib/libcorkscrew.so";

  // if(gHandle == NULL)
  //   gHandle = dlopen(so_path, RTLD_NOW);

  // if (gHandle != NULL && !unwind_backtrace)
  //   unwind_backtrace = (unwindFn)dlsym(gHandle, "unwind_backtrace");

  // if (unwind_backtrace == NULL)
  //   return;

  // backtrace_frame_t stacks[20];
  // int count = unwind_backtrace(stacks, 1, 20);
  // if (count <= 0)
  //   return;

  // for (int i = 0; i < size; ++i) {
  //   pbt->stacks[i] = stacks[i]; // stacks[i].absolute_pc;
  // }

  // pbt->count = size;
  _Unwind_Backtrace(unwind_callback, pbt);
}

