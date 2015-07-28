#pragma once

#include <cstdint>
#include <cstring>

#ifdef __ANDROID__

typedef uintptr_t pointer_type_t; 

#ifdef __arm__
#define CALLID "u"
#endif

#elif defined(__linux__)

#define UNW_LOCAL_ONLY
#include <libunwind.h>

typedef unw_word_t pointer_type_t;

#ifdef __i386__
#define CALLID "u"
#elif defined __x86_64__ 
#define CALLID "lu"
#endif

#endif

enum {kMaxTraceCount = 20};

struct address_info {
  pointer_type_t handle;
  pointer_type_t offset;
};

struct callstack_detail {
  pointer_type_t callid;
  address_info addresses[kMaxTraceCount];
};

struct callstack {
  uint32_t alloc_size;
  uint32_t count;
  pointer_type_t stacks[kMaxTraceCount];

  const char* begin() const {
    return reinterpret_cast<const char*>(this);
  }

  size_t size() const {
    const char *end = reinterpret_cast<const char*>(&stacks[count]);
    return static_cast<size_t>(end - begin());
  }
};

inline bool operator==(const callstack &a, const callstack &b) {
  if (a.count != b.count)
    return false;

  return !memcmp(a.begin(), b.begin(), a.size());
}

void get_callstack(size_t n, callstack *pbt);

