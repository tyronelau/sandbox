#include <dlfcn.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>

#include "memory_func.h"
#include "guard_thread.h"

alloc_func_t g_real_malloc = NULL;
free_func_t g_real_free = NULL;
realloc_func_t g_real_realloc = NULL;
calloc_func_t g_real_calloc = NULL;
posix_memalign_func_t g_real_posix_memalign = NULL;
alloc_func_t g_real_valloc = NULL;
calloc_func_t g_real_memalign = NULL;

void dump_backtrace(void *p, size_t size);
void record_free(void *p);
void dump_memory_snapshot();

extern "C" {
void __attribute__ ((constructor)) dll_load(void);
void __attribute__ ((destructor)) dll_unload(void);

void dll_load(void) {
  // FIXME(liuyong): dlsym will sometimes call |malloc|, but this library
  // is still initializing
  g_real_malloc = (alloc_func_t)dlsym(RTLD_NEXT, "malloc");
  g_real_free = (free_func_t)dlsym(RTLD_NEXT, "free");
  g_real_realloc = (realloc_func_t)dlsym(RTLD_NEXT, "realloc");
  g_real_calloc = (calloc_func_t)dlsym(RTLD_NEXT, "calloc");
  g_real_posix_memalign = (posix_memalign_func_t)dlsym(RTLD_NEXT, "posix_memalign");
  g_real_valloc = (alloc_func_t)dlsym(RTLD_NEXT, "valloc");
  g_real_memalign = (calloc_func_t)dlsym(RTLD_NEXT, "memalign");

  guard_thread::create_and_run();
}

void dll_unload(void) {
  // dump_memory_snapshot();
  guard_thread::terminate();
}

void* malloc(size_t size) {
  if (g_real_malloc != NULL) {
    void *p = g_real_malloc(size);
    if (p != NULL) {
      dump_backtrace(p, size);
      return p;
    }
  }

  return NULL;
}

void free(void *ptr) {
  if (ptr == NULL)
    return;

  if (g_real_free != NULL) {
    g_real_free(ptr);
    record_free(ptr);
  }
}

void* realloc(void *ptr, size_t size) {
  // if (g_real_realloc != NULL) {
  //   return g_real_realloc(ptr, size);
  // }

  if (ptr == NULL) {
    return malloc(size);
  }

  if (size == 0) {
    free(ptr);
    return NULL;
  }

  if (g_real_realloc == NULL)
    return NULL;

  // Release the pointer |ptr| from our db, but not really destroy it
  record_free(ptr);

  ptr = g_real_realloc(ptr, size);

  // Add this pointer into our db, and track it
  dump_backtrace(ptr, size);
  return ptr;
}

void* calloc(size_t nmem, size_t size) {
  size_t total = nmem * size;
  void *p = malloc(total);
  if (p == NULL)
    return NULL;

  memset(p, 0, total);
  return p;
}

__attribute__((visibility("default"))) int posix_memalign(void **memptr, size_t alignment, size_t size) {
  if (g_real_posix_memalign != NULL) {
    if (!g_real_posix_memalign(memptr, alignment ,size)) {
      // char buf[] = "memalign called\n";
      // write(2, buf, sizeof(buf));
      dump_backtrace(*memptr, size);
      return 0;
    }
  }

  return -1;
}

__attribute__((visibility("default"))) void* memalign(size_t boundary, size_t size) {
  if ((g_real_memalign != NULL)) {
    void *p = g_real_memalign(boundary, size);
    if (p == NULL)
      return NULL;
    dump_backtrace(p, size);
    return p;
  }

  return NULL;
}

__attribute__((visibility("default"))) void* valloc(size_t size) {
  return memalign(sysconf(_SC_PAGESIZE), size);
}

}

