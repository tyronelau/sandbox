#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/declare_struct.h"

namespace agora {
namespace base {

class packer {
  enum {
    kDefaultSize = 1024,
    kMaxSize  = 64 * 1024 * 1024,
  };
 public:
  packer();
  ~packer();

  size_t length() const;
  std::string body() const;
  const char* buffer() const;

  packer& pack();
  void reset();

  void write(uint16_t val, uint32_t position);
  void write(uint32_t val, uint32_t position);

  void push(uint64_t val);
  void push(int64_t val);
  void push(uint32_t val);
  void push(int32_t val);
  void push(uint16_t val);
  void push(int16_t val);
  void push(uint8_t val);
  void push(int8_t val);
  void push(const std::string &val);

  void check_size(size_t more, uint32_t position);
 private:
  packer(const packer &) = delete;
  packer(packer &&) = delete;

  packer& operator=(const packer &) = delete;
  packer& operator=(packer &&) = delete;
 private:
  std::vector<char> buffer_;
  uint32_t length_;
  uint32_t position_;
};

packer& operator<<(packer &pkr, uint64_t v);
packer& operator<<(packer &pkr, int64_t v);
packer& operator<<(packer &pkr, uint32_t v);
packer& operator<<(packer &pkr, int32_t v);
packer& operator<<(packer &pkr, uint16_t v);
packer& operator<<(packer &pkr, int16_t v);
packer& operator<<(packer &pkr, uint8_t v);
packer& operator<<(packer &pkr, int8_t v);
packer& operator<<(packer &pkr, const std::string &v);

template<typename T>
packer& operator<<(packer &pkr, const std::vector<T> &v) {
  uint32_t count = static_cast<uint32_t>(v.size());
  pkr << count;

  typedef typename std::vector<T>::const_iterator iter_t;
  for (iter_t it = v.begin(); it != v.end(); ++it) {
    pkr << *it;
  }

  return pkr;
}

template<typename K, typename V>
packer& operator<<(packer &pkr, const std::map<K, V> &v) {
  uint32_t count = static_cast<uint32_t>(v.size());
  pkr << count;

  typedef typename std::map<K, V>::const_iterator iter_t;
  for (iter_t it = v.begin(); it != v.end(); ++it) {
    pkr << it->first << it->second;
  }

  return pkr;
}

class unpacker {
 public:
  explicit unpacker(const char *buf=NULL, size_t len=0, bool copy=false);
  unpacker(unpacker &&rhs);

  unpacker& operator=(unpacker &&rhs);

  ~unpacker();
 public:
  const char*  buffer() const;
  size_t length() const;
  void check_size(size_t more, uint32_t position) const;

  void rewind();

  void write(uint16_t val, uint32_t position);

  uint64_t pop_uint64();
  uint32_t pop_uint32();
  uint16_t pop_uint16();
  uint8_t pop_uint8();

  std::string pop_string();
 private:
  const char *buffer_;
  uint32_t length_;
  uint32_t position_;
  bool copy_;
};

unpacker& operator>>(unpacker &unpkr, uint64_t &v);
unpacker& operator>>(unpacker &unpkr, uint32_t &v);
unpacker& operator>>(unpacker &unpkr, uint16_t &v);
unpacker& operator>>(unpacker &unpkr, uint8_t &v);
unpacker& operator>>(unpacker &unpkr, int64_t &v);
unpacker& operator>>(unpacker &unpkr, int32_t &v);
unpacker& operator>>(unpacker &unpkr, int16_t &v);
unpacker& operator>>(unpacker &unpkr, int8_t &v);
unpacker& operator>>(unpacker &unpkr, std::string &v);

template<typename T>
unpacker& operator>>(unpacker &unpkr, std::vector<T> &v) {
  uint32_t count = unpkr.pop_uint32();
  v.reserve(count);

  for (uint32_t i = 0; i < count; ++i) {
    T t;
    unpkr >> t;
    v.push_back(std::move(t));
  }

  return unpkr;
}

template<typename K, typename V>
unpacker& operator>>(unpacker &unpkr, std::map<K, V> &x) {
  uint32_t count = unpkr.pop_uint32();

  for (uint32_t i = 0; i < count; ++i) {
    K k;
    V v;
    unpkr >> k >> v;
    x.insert(std::make_pair(std::move(k), std::move(v)));
  }

  return unpkr;
}

#define DECLARE_PACKABLE_1_START(name, type1, name1) \
  DECLARE_STRUCT_1_START(name, type1, name1) \
  friend packer& operator<<(packer &p, const name &x) { \
    p << x.name1; \
    return p; \
  } \
  friend unpacker& operator>>(unpacker &p, name &x) { \
    p >> x.name1; \
    return p; \
  }

#define DECLARE_PACKABLE_1(name, type1, name1) \
  DECLARE_PACKABLE_1_START(name, type1, name1) \
  DECLARE_STRUCT_END

#define DECLARE_PACKABLE_2_START(name, type1, name1, type2, name2) \
  DECLARE_STRUCT_2_START(name, type1, name1, type2, name2) \
  friend packer& operator<<(packer &p, const name &x) { \
    p << x.name1 << x.name2; \
    return p; \
  } \
  friend unpacker& operator>>(unpacker &p, name &x) { \
    p >> x.name1 >> x.name2; \
    return p; \
  }

#define DECLARE_PACKABLE_2(name, type1, name1, type2, name2) \
  DECLARE_PACKABLE_2_START(name, type1, name1, type2, name2) \
  DECLARE_STRUCT_END

#define DECLARE_PACKABLE_3_START(name,type1,name1,type2,name2,type3,name3) DECLARE_STRUCT_3_START(name,type1,name1,type2,name2,type3,name3) \
    friend packer & operator<< (packer& p, const name & x) \
    {  \
        p << x.name1 << x.name2 << x.name3; \
        return p;\
    }\
    friend unpacker & operator>> (unpacker & p, name & x) \
    {      \
      p >> x.name1 >> x.name2 >> x.name3; \
        return p;\
    }
#define DECLARE_PACKABLE_3(name,type1,name1,type2,name2,type3,name3) DECLARE_PACKABLE_3_START(name,type1,name1,type2,name2,type3,name3) \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_4_START(name,type1,name1,type2,name2,type3,name3,type4,name4) DECLARE_STRUCT_4_START(name,type1,name1,type2,name2,type3,name3,type4,name4) \
    friend packer & operator<< (packer& p, const name & x) \
    {  \
        p << x.name1 << x.name2 << x.name3 << x.name4; \
        return p;\
    }\
    friend unpacker & operator>> (unpacker & p, name & x) \
    {      \
      p >> x.name1 >> x.name2 >> x.name3 >> x.name4; \
        return p;\
    }
#define DECLARE_PACKABLE_4(name,type1,name1,type2,name2,type3,name3,type4,name4) DECLARE_PACKABLE_4_START(name,type1,name1,type2,name2,type3,name3,type4,name4) \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_5_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5) DECLARE_STRUCT_5_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5) \
    friend packer & operator<< (packer& p, const name & x) \
    {  \
        p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5; \
        return p;\
    }\
    friend unpacker & operator>> (unpacker & p, name & x) \
    {      \
      p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5; \
        return p;\
    }
#define DECLARE_PACKABLE_5(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5) DECLARE_PACKABLE_5_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5) \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_6_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6) DECLARE_STRUCT_6_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6) \
    friend packer & operator<< (packer& p, const name & x) \
    {  \
        p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6; \
        return p;\
    }\
    friend unpacker & operator>> (unpacker & p, name & x) \
    {      \
      p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6; \
        return p;\
    }
#define DECLARE_PACKABLE_6(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6) DECLARE_PACKABLE_6_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6) \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_7_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7) DECLARE_STRUCT_7_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7) \
    friend packer & operator<< (packer& p, const name & x) \
    {  \
        p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7; \
        return p;\
    }\
    friend unpacker & operator>> (unpacker & p, name & x) \
    {      \
      p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7; \
        return p;\
    }
#define DECLARE_PACKABLE_7(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7) DECLARE_PACKABLE_7_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7) \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_8_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8) DECLARE_STRUCT_8_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8) \
    friend packer & operator<< (packer& p, const name & x) \
    {  \
        p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8; \
        return p;\
    }\
    friend unpacker & operator>> (unpacker & p, name & x) \
    {      \
      p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8; \
        return p;\
    }
#define DECLARE_PACKABLE_8(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8) DECLARE_PACKABLE_8_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8) \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_9_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8,type9,name9) DECLARE_STRUCT_9_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8,type9,name9) \
    friend packer & operator<< (packer& p, const name & x) \
    {  \
        p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8 << x.name9; \
        return p;\
    }\
    friend unpacker & operator>> (unpacker & p, name & x) \
    {      \
      p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8 >> x.name9; \
        return p;\
    }
#define DECLARE_PACKABLE_9(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8,type9,name9) DECLARE_PACKABLE_9_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8,type9,name9) \
  DECLARE_STRUCT_END

} }
