#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>

#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include "allocator.h"
#include "CityHash.h"

enum {kMaxTraceCount = 20};

struct address_info {
  unw_word_t handle;
  unw_word_t offset;
};

struct callstack_detail {
  unw_word_t callid;
  address_info addresses[kMaxTraceCount];
};

struct callstack {
  uint32_t alloc_size;
  uint32_t count;
  unw_word_t stacks[kMaxTraceCount];

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

struct callstack_hasher {
  size_t operator()(const callstack &bt) const {
    return CityHash64(bt.begin(), bt.size());
  }
};

typedef uint32_t callid_t;

typedef std::pair<const callstack, callstack_detail> callid_elem_t;
typedef std::unordered_map<callstack, callstack_detail, callstack_hasher,
    std::equal_to<callstack>, native_allocator<callid_elem_t> > callstack_map_t;

typedef std::unordered_map<callid_t, uint32_t, std::hash<callid_t>,
    std::equal_to<callid_t>,
    native_allocator<std::pair<const callid_t, uint32_t> > > memusage_map_t;

typedef std::pair<const unw_word_t, callid_t> malloc_record_t;
typedef std::unordered_map<unw_word_t, callid_t, std::hash<unw_word_t>,
    std::equal_to<unw_word_t>, native_allocator<malloc_record_t> > mem_map_t;

typedef std::basic_string<char, std::char_traits<char>,
    native_allocator<char> > native_string_t; 

struct sopath_hasher {
  size_t operator()(const native_string_t &s) const {
    return CityHash64(&s[0], s.size());
  }
};

typedef std::pair<const native_string_t, unw_word_t> so_item_t;
typedef std::unordered_map<native_string_t, unw_word_t, sopath_hasher,
    std::equal_to<native_string_t>, native_allocator<so_item_t> > so_db_t;

static std::mutex g_backtrace_lock;
static bool g_init = false;
static uint32_t g_next_callid = 0;

static int g_next_so_id = 0;
static char g_library_db_buf[sizeof(so_db_t)];

static char g_callstack_buf[sizeof(callstack_map_t)];
static char g_memusage_buf[sizeof(memusage_map_t)];
static char g_malloc_buf[sizeof(mem_map_t)];

static so_db_t *g_so_db;
static callstack_map_t *g_callstack_db;
static memusage_map_t *g_memusage_db;
static mem_map_t *g_malloc_db;

static void init_backtrace() {
  g_so_db = ::new (g_library_db_buf) so_db_t();
  g_callstack_db = ::new (g_callstack_buf) callstack_map_t();
  g_memusage_db = ::new (g_memusage_buf) memusage_map_t();
  g_malloc_db = ::new (g_malloc_buf) mem_map_t();

  g_init = true;
}

static void get_callstack(size_t n, callstack *pbt) {
  callstack &bt = *pbt;
  unw_cursor_t cursor;
  unw_context_t uc;
  unw_word_t ip, sp;

  unw_getcontext(&uc);
  unw_init_local(&cursor, &uc);

  bt.count = 0;
  bt.alloc_size = n;

  while (unw_step(&cursor) > 0 && bt.count < kMaxTraceCount) {
    unw_get_reg(&cursor, UNW_REG_IP, &ip);
    unw_get_reg(&cursor, UNW_REG_SP, &sp);

    bt.stacks[bt.count++] = ip;
    (void)sp;
  }
}

static int simple_snprintf(char *p, int len, const char *fmt, ...);

static callstack_detail create_callstack_info(const callstack &bt) {
  callstack_detail detail;
  detail.callid = g_next_callid++;
  for (int i = 0; i < bt.count && i < kMaxTraceCount; ++i) {
    Dl_info info;
    dladdr(reinterpret_cast<const void*>(bt.stacks[i]), &info);
    // if (info.dli_sname) {
    //   char buf[1024];
    //   simple_snprintf(buf, 1024, "%x: base(%x) symbol %s\n", bt.stacks[i], info.dli_fbase, info.dli_sname);
    //   write(2, buf, strlen(buf));
    // } else {
    //   char buf[1024];
    //   simple_snprintf(buf, 1024, "%x: \n", bt.stacks[i]);
    //   write(2, buf, strlen(buf));
    // }

    native_string_t sopath(info.dli_fname);
    if (g_so_db->find(sopath) == g_so_db->end()) {
      (*g_so_db)[sopath] = g_next_so_id++;
    }

    auto &frame = detail.addresses[i];
    frame.handle = (*g_so_db)[sopath];
    frame.offset = bt.stacks[i] - (unw_word_t)info.dli_fbase;
  }

  return detail;
}

void dump_backtrace(void *p, size_t n) {
  callstack bt;
  get_callstack(n, &bt);

  if (bt.count == 0)
    return;

  std::lock_guard<std::mutex> lock(g_backtrace_lock);
  if (!g_init) {
    init_backtrace();
  }

  uint32_t callid = 0;
  auto it = g_callstack_db->find(bt);
  if (it == g_callstack_db->end()) {
    callid = g_next_callid;
    (*g_callstack_db)[bt] = create_callstack_info(bt);
  } else {
    callid = it->second.callid;
  }

  ++((*g_memusage_db)[callid]);
  (*g_malloc_db)[reinterpret_cast<unw_word_t>(p)] = callid;
}

void record_free(void *p) {
  std::lock_guard<std::mutex> lock(g_backtrace_lock);
  if (!g_init)
    return;
  auto it = g_malloc_db->find(reinterpret_cast<unw_word_t>(p));
  if (it == g_malloc_db->end())
    return;

  uint32_t callid = it->second;
  g_malloc_db->erase(it);

  if (--((*g_memusage_db)[callid]) == 0) {
    g_memusage_db->erase(callid);
    // FIXME(liuyong): erase callstack from g_callstack_db
  }
}

static inline int format_hex_word(char *p, unw_word_t pc) {
  char buf[32];
  int i = 0;
  const static char hex[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F'
  };

  while (pc != 0) {
    int r = pc & 0xF;
    pc >>= 4;
    buf[i++] = hex[r];
  }
  
  if (i == 0) {
    buf[0] = '0';
    i = 1;
  }

  for (int j = i - 1, k = 0; j >= 0; --j, ++k) {
    p[j] = buf[k];
  }
  
  return i;
}

static inline int format_word(char *p, long num) {
  char buf[32];
  int i = 0;
  const static char digits[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
  };

  while (num != 0) {
    long r = num % 10;
    num /= 10;
    buf[i++] = digits[r];
  }
  
  if (i == 0) {
    buf[0] = '0';
    i = 1;
  }

  for (int j = i - 1, k = 0; j >= 0; --j, ++k) {
    p[j] = buf[k];
  }
  
  return i;
}

static int simple_strncpy(char *dst, const char *src, int n) {
  int r = n;
  while (*src && r >= 0) {
    *dst++ = *src++;
    --r;
  }

  return n - r;
}

// A simple but not-safe snprintf to work around the recursive malloc-calling.
static int simple_snprintf(char *p, int len, const char *fmt, ...) {
  int r = len;
  int i = 0;
  int state = 0;
  va_list va;

  va_start(va, fmt);

  while (*fmt && r > 0) {
    if (state == 0) {
      if (*fmt == '%') {
        state = 1;
      } else {
        p[i++] = *fmt;
        --r;
      }
    } else {
      state = 0;
      switch (*fmt) {
      case 'd':  {
        int val = va_arg(va, int);
        int n = format_word(p + i, val);
        i += n;
        r -= n;
        break;
      }
      case 'x': {
        unw_word_t val = va_arg(va, unw_word_t);
        int n = format_hex_word(p + i, val);
        i += n;
        r -= n;
        break;
      }
      case 's': {
        const char *s = va_arg(va, const char *);
        int n = simple_strncpy(p + i, s, r);
        i += n;
        r -= n;
        break;
      }
      case '%':
        p[i++] = '%';
        --r;
        break;
      default: return -1;
      }
    }

    ++fmt;
  }

  va_end(va);

  if (r > 0) {
    p[i] = '\0';
  }

  return i;
}

static char* dump_single_callstack(char *p, int buf_size, int count,
    int alloc_size, int stack_depth, const callstack &bt,
    const callstack_detail &detail) {
  int n = simple_snprintf(p, buf_size, "Allocated %d bytes in %d calls\n",
      alloc_size * count, count);
  if (buf_size < n + 512)
    return p;

  buf_size -= n;
  p += n;

  for (int i = 0; i < stack_depth; ++i) {
    n = simple_snprintf(p, buf_size, "%x: %d + %x\n", bt.stacks[i],
        (int)detail.addresses[i].handle, detail.addresses[i].offset); 
    buf_size -= n;
    p += n;
  }

  *p++ = '\n';
  return p;
}

static void dump_so_paths() {
  enum {kMinSpace = 512, kBufSize = 8000};
  char buf[kBufSize];
  int n = 0;

  const so_db_t &so_db = *g_so_db;
  for (const auto &so : so_db) {
    if (n > kBufSize - kMinSpace) {
      write(2, buf, n);
      n = 0;
    }
    int cnt = simple_snprintf(buf + n, kBufSize - n, "handle %d: %s\n", int(so.second),
        so.first.c_str());
    n += cnt;
  }

  buf[n++] = '\n';

  write(2, buf, n);
}

static void dump_memory_prelogue() {
  char buf[] = "MEMORY SNAPSHOT IS DUMPPING\n";
  write(2, buf, sizeof(buf));
}

static void dump_memory_epilogue() {
  char buf[] = "MEMORY SNAPSHOT DUMPPING FINISHED\n";
  write(2, buf, sizeof(buf));
}

void dump_memory_snapshot() {
  std::lock_guard<std::mutex> lock(g_backtrace_lock);
  if (!g_init)
    return;

  dump_memory_prelogue();
  dump_so_paths();

  const callstack_map_t &callstack_db = *g_callstack_db;
  const memusage_map_t &memusage_db = *g_memusage_db;
  const mem_map_t &malloc_db = *g_malloc_db;

  enum {kCallstackSize = 512, kCacheSize = 8192};
  char buf[kCacheSize];
  int n = 0;

  for (const auto &bt : callstack_db) {
    const callstack &s = bt.first;
    uint32_t callid = bt.second.callid;
    auto it = memusage_db.find(callid);
    if (it == memusage_db.end())
      continue;

    uint32_t count = it->second;
    if (n > kCacheSize - kCallstackSize) {
      write(2, buf, n);
      n = 0;
    }

    char *pos = dump_single_callstack(buf + n, kCacheSize - n, count,
        s.alloc_size, s.count, s, bt.second);
    n = pos - buf;
  }

  write(2, buf, n);

  dump_memory_epilogue();
  fsync(2);
}

