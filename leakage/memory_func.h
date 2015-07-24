#pragma once

typedef void* (*alloc_func_t)(size_t);
typedef void (*free_func_t)(void *);
typedef void* (*realloc_func_t)(void *, size_t);
typedef void* (*calloc_func_t)(size_t, size_t);
typedef int (*posix_memalign_func_t)(void **, size_t, size_t);

extern alloc_func_t g_real_malloc;
extern free_func_t g_real_free;
extern realloc_func_t g_real_realloc;
extern calloc_func_t g_real_calloc;
extern posix_memalign_func_t g_real_posix_memalign;
extern alloc_func_t g_real_valloc;
extern calloc_func_t g_real_memalign;

