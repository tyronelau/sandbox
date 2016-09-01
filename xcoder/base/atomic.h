#pragma once

#ifdef __GNUC__
#if __GNUC_PREREQ(4, 6)
#include <atomic>
typedef std::atomic<bool> atomic_bool_t;

#else
namespace agora {
namespace pstn {
template <typename T> class atomic;

template <>
class atomic<bool> {
 public:
  explicit atomic(bool value=false);
  ~atomic();

  operator bool() const;
  atomic<bool>& operator=(bool value);
  bool load();
  void store(bool value);
 private:
  bool value_;
};

inline
atomic<bool>::atomic(bool value) {
 value_ = value;
 __sync_synchronize();
}

inline
atomic<bool>::~atomic() {
}

inline
atomic<bool>::operator bool() const {
  return value_;
}

inline
atomic<bool>& atomic<bool>::operator=(bool value) {
  value_ = value;
  __sync_synchronize();
  return *this;
}

inline bool atomic<bool>::load() {
  __sync_synchronize();
  return value_;
}

inline void atomic<bool>::store(bool value) {
  value_ = value;
  __sync_synchronize();
}

}
}

typedef agora::pstn::atomic<bool> atomic_bool_t;

#endif
#endif

