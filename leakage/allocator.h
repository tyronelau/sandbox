#pragma once
#include <new>
#include "memory_func.h"

template <typename T>
struct native_allocator {
  typedef T value_type;
  typedef T *pointer;
  typedef const T *const_pointer;
  typedef T &reference;
  typedef const T &const_reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  
  template <typename U>
  struct rebind {
    typedef native_allocator<U> other;
  };

  native_allocator() = default;
  ~native_allocator() = default;

  template <typename U>
  native_allocator(const native_allocator<U> &other __attribute__((unused))) {}

  pointer address(reference x) const {
    return reinterpret_cast<T*>(&const_cast<char&>(
      reinterpret_cast<const volatile char &>(x)));
  }

  pointer allocate(size_type n, const_pointer hint = nullptr) {
    (void)hint;
    void *p = g_real_malloc(n * sizeof(value_type));
    return reinterpret_cast<pointer>(p);
  }

  void deallocate(pointer p, size_type n) {
    (void)n;
    g_real_free(p);
  }

  size_type max_size() const noexcept {
    return 256 * 1024 * 1024;
  }

  void construct(pointer p, const_reference val) {
    new (p) T(val);
  }

  template <typename U, typename ...Args>
  void construct(U *p, Args&&... args) {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }

  void destroy(pointer p) {
    p->~T();
  }

  template <typename U> void destroy(U *p) {
    p->~U();
  }
};

template <typename T1, typename T2>
inline bool operator==(const native_allocator<T1> &a, const native_allocator<T2> &b) {
  return true;
}

template <typename T1, typename T2>
inline bool operator!=(const native_allocator<T1> &a, const native_allocator<T2> &b) {
  return !(a == b);
}

