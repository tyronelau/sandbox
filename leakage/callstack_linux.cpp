#include "callstack_info.h"

void get_callstack(size_t n, callstack *pbt) {
  callstack &bt = *pbt;
  unw_cursor_t cursor;
  unw_context_t uc;
  unw_word_t ip;

  unw_getcontext(&uc);
  unw_init_local(&cursor, &uc);

  bt.count = 0;
  bt.alloc_size = n;

  while (unw_step(&cursor) > 0 && bt.count < kMaxTraceCount) {
    unw_get_reg(&cursor, UNW_REG_IP, &ip);
    // unw_get_reg(&cursor, UNW_REG_SP, &sp);

    bt.stacks[bt.count++] = ip;
    // (void)sp;
  }
}

