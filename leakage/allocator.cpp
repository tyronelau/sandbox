#include <cstdlib>
#include <new>

void* operator new(size_t size) {
  return malloc(size);
}

void operator delete(void *ptr) {
  free(ptr);
}

void* operator new(size_t size, const std::nothrow_t &nothrow) {
  (void)nothrow;
  return malloc(size);
}

void operator delete(void *ptr, const std::nothrow_t &nothrow) {
  (void)nothrow;
  free(ptr);
}

void* operator new[](size_t size) {
  return malloc(size);
}

void operator delete[](void *ptr) {
  free(ptr);
}

void* operator new[](size_t size, const std::nothrow_t &nothrow) {
  return malloc(size);
}

void operator delete[](void *ptr, const std::nothrow_t &nothrow) {
  (void)nothrow;
  free(ptr);
}

