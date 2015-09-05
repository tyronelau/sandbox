#pragma once
#include <cassert>
#include <cstring>
#include "memory_func.h"

namespace base {
class const_string {
 public:
  explicit const_string(const char *);
  ~const_string();

  const_string(const_string &&rhs);
  const_string& operator=(const_string &&rhs);

  const_string(const const_string &rhs);

  const char* get_cstr() const;
  size_t size() const;
  const char& operator[](size_t i) const;
 private:
  const char *rep_;
  size_t len_;
};

inline bool operator==(const const_string &a, const const_string &b) {
  return !strcmp(a.get_cstr(), b.get_cstr());
}

inline bool operator!=(const const_string &a, const const_string &b) {
  return !(a == b);
}

inline const_string::const_string(const char *p) {
  size_t len = strlen(p); 
  char *data = reinterpret_cast<char *>(__libc_malloc(len + 1));

  assert(data != NULL);
  strcpy(data, p);
  len_ = len;
  rep_ = data;
}

inline const_string::~const_string() {
  __libc_free(const_cast<void *>(reinterpret_cast<const void *>(rep_)));
}

inline const_string::const_string(const_string &&rhs) {
  rep_ = rhs.rep_;
  len_ = rhs.len_;
  rhs.rep_ = NULL;
  rhs.len_ = 0;
}

inline const_string::const_string(const const_string &rhs) {
  rep_ = NULL;
  len_ = 0;

  if (rhs.rep_) {
    char *dst = reinterpret_cast<char *>(__libc_malloc(rhs.len_ + 1));
    strncpy(dst, rhs.rep_, rhs.len_ + 1);
    rep_ = dst;
    len_ = rhs.len_;
  }
}

inline const_string& const_string::operator=(const_string &&rhs) {
  std::swap(rep_, rhs.rep_);
  std::swap(len_, rhs.len_);

  return *this;
}

inline const char* const_string::get_cstr() const {
  return rep_;
}

inline size_t const_string::size() const {
  return len_;
}

inline const char& const_string::operator[](size_t i) const {
  assert(i < len_);
  return rep_[i];
}

}

