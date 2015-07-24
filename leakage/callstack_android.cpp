#include "callstack_info.h"
#include <unwind.h>

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

  _Unwind_Backtrace(unwind_callback, pbt);
}

