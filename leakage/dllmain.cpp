#include <dlfcn.h>
#include <cstdio>
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
  dump_memory_snapshot();
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
  if (g_real_free != NULL) {
    g_real_free(ptr);
    record_free(ptr);
  }
}

void* realloc(void *ptr, size_t size) {
  if (g_real_realloc != NULL) {
    return g_real_realloc(ptr, size);
  }

  return NULL;
}

void* calloc(size_t nmem, size_t size) {
  if (g_real_calloc != NULL) {
    return g_real_calloc(nmem, size);
  }

  return NULL;
}

int posix_memalign(void **memptr, size_t alignment, size_t size) {
  if (g_real_posix_memalign != NULL) {
    return g_real_posix_memalign(memptr, alignment ,size);
  }

  return -1;
}

void* valloc(size_t size) {
  if ((g_real_valloc != NULL)) {
    return g_real_valloc(size);
  }

  return NULL;
}

void* memalign(size_t boundary, size_t size) {
  if ((g_real_memalign != NULL)) {
    return g_real_memalign(boundary, size);
  }

  return NULL;
}

}

