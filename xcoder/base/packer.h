#pragma once

#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <map>

#include "base/declare_struct.h"

namespace agora { namespace base {

    class packer
    {
            enum
            {
                PACKET_BUFFER_SIZE = 1024,
                PACKET_BUFFER_SIZE_MAX = 64*1024
            }
            ;
    public:
      packer()
                : buffer_(PACKET_BUFFER_SIZE)
                , length_(0)
                , position_(2)
      {
      }
      ~packer()
      {
      }

    public:
      packer & pack()
      {
        length_ = position_;
        position_ = 0;
        *this << length_;
        position_ = length_;
                return *this;
      }

      void reset()
      {
        length_ = 0;
        position_ = 2;
      }

      void write(uint16_t val, uint16_t position)
      {
        check_size(sizeof(val), position);
                ::memcpy(&buffer_[0] + position, &val, sizeof(val));
      }

      void write(uint32_t val, uint16_t position)
      {
        check_size(sizeof(val), position);
                ::memcpy(&buffer_[0] + position, &val, sizeof(val));
      }

      void push(uint64_t val)
      {
        check_size(sizeof(val), position_);
                ::memcpy(&buffer_[0] + position_, &val, sizeof(val));
        position_ += static_cast<uint16_t>(sizeof(val));
      }

      void push(uint32_t val)
      {
        check_size(sizeof(val), position_);
                ::memcpy(&buffer_[0] + position_, &val, sizeof(val));
        // position_ += sizeof(val);
        position_ += static_cast<uint16_t>(sizeof(val));
      }

      void push(uint16_t val)
      {
        check_size(sizeof(val), position_);
                ::memcpy(&buffer_[0] + position_, &val, sizeof(val));
        // position_ += sizeof(val);
        position_ += static_cast<uint16_t>(sizeof(val));
      }

      void push(uint8_t val)
      {
        check_size(sizeof(val), position_);
                ::memcpy(&buffer_[0] + position_, &val, sizeof(val));
        // position_ += sizeof(val);
        position_ += static_cast<uint16_t>(sizeof(val));
      }

      void push(const std::string & val)
      {
        push(static_cast<uint16_t>(val.length()));

        size_t length = val.length();
        check_size(length, position_);
        if (length > 0)
        {
                    ::memcpy(&buffer_[0] + position_, val.data(), length);
          position_ += length;
        }
      }

            const char*  buffer() const { return &buffer_[0]; }
      size_t  length() const { return length_; }
            std::string body() const { return std::string(&buffer_[0] + 2, length_ - 2); }

            void check_size(size_t more, uint16_t position)
      {
                if (buffer_.size() - position < more) {
                    size_t new_size = buffer_.size() * 4;
                    if (new_size > PACKET_BUFFER_SIZE_MAX) {
                        throw std::overflow_error("packer buffer overflow!");
                    }
                    buffer_.resize(new_size);
                }
      }

      packer& operator<< (uint64_t v)
      {
        push(v);
        return *this;
      }

      packer& operator<< (uint32_t v)
      {
        push(v);
        return *this;
      }

      packer& operator<< (uint16_t v)
      {
        push(v);
        return *this;
      }
      packer& operator<< (uint8_t v)
      {
        push(v);
        return *this;
      }

      packer& operator<< (int64_t v)
      {
        push(static_cast<uint64_t>(v));
        return *this;
      }

      packer& operator<< (int32_t v)
      {
        push(static_cast<uint32_t>(v));
        return *this;
      }

      packer& operator<< (int16_t v)
      {
        push(static_cast<uint16_t>(v));
        return *this;
      }
      packer& operator<< (int8_t v)
      {
        push(static_cast<uint8_t>(v));
        return *this;
      }

      packer& operator<< (const std::string & v)
      {
        push(v);
        return *this;
      }

      template<typename T>
      packer& operator<< (const std::vector<T> &v)
      {
        uint16_t count = v.size();
        *this << count;
        for (uint16_t i = 0; i < count; i++)
        {
          *this << v[i];
        }
        return *this;
      }

      template<typename K, typename V>
      packer& operator<< (const std::map<K, V> & v)
      {
        uint16_t count = v.size();
        *this << count;
        for (typename std::map<K, V>::const_iterator it = v.begin(); it != v.end(); ++it) {
          *this << it->first << it->second;
        }
        return *this;
      }

    private:
            std::vector<char> buffer_;
      uint16_t  length_;
      uint16_t  position_;
    };

    class unpacker
    {
    public:
      unpacker() :buffer_(NULL), length_(0), position_(0), copy_(false) {}
      unpacker(unpacker &&rhs) {
        buffer_ = rhs.buffer_;
        length_ = rhs.length_;
        position_ = rhs.position_;
        copy_ = rhs.copy_;

        rhs.buffer_ = NULL;
      }

      unpacker& operator= (unpacker &&rhs) {
        if (copy_) {
          delete []buffer_;
          buffer_ = NULL;
        }

        buffer_ = rhs.buffer_;
        length_ = rhs.length_;
        position_ = rhs.position_;
        copy_ = rhs.copy_;

        rhs.buffer_ = NULL;

        return *this;
      }

      unpacker(const char* buf, size_t len, bool copy = false)
        : buffer_(NULL)
        , length_(len)
        , position_(0)
        , copy_(copy)
      {
        if (copy_) {
          buffer_ = new char[len];
          ::memcpy(buffer_, buf, len);
        }
        else {
          buffer_ = const_cast<char *>(buf);
        }
      }
      ~unpacker()  {
        if (copy_) {
                    delete [] buffer_;
        }
      }

    public:
      void rewind() {
        position_ = 2;
      }

      void write(uint16_t val, uint16_t position) {
        check_size(sizeof(val), position);
        ::memcpy(buffer_ + position, &val, sizeof(val));
      }

      uint64_t pop_uint64() {
        uint64_t v = 0;
        check_size(sizeof(v), position_);
        ::memcpy(&v, buffer_ + position_, sizeof(v));
        position_ += sizeof(v);
        return v;
      }

      uint32_t pop_uint32() {
        uint32_t v = 0;
        check_size(sizeof(v), position_);
        ::memcpy(&v, buffer_ + position_, sizeof(v));
        position_ += sizeof(v);
        return v;
      }

      uint16_t pop_uint16() {
        uint16_t v = 0;
        check_size(sizeof(v), position_);
        ::memcpy(&v, buffer_ + position_, sizeof(v));
        position_ += sizeof(v);
        return v;
      }

      uint8_t  pop_uint8() {
        uint8_t v = 0;
        check_size(sizeof(v), position_);
        ::memcpy(&v, buffer_ + position_, sizeof(v));
        position_ += sizeof(v);
        return v;
      }

      std::string  pop_string() {
        uint16_t length = pop_uint16();
        check_size(length, position_);
        std::string s = std::string(buffer_ + position_, length);
        position_ += length;

        return s;
      }

      const char*  buffer() const {
        return buffer_;
      }

      size_t length() const {
        return length_;
      }

      void check_size(size_t more, uint16_t position) const {
        if (static_cast<size_t>(length_ - position) < more) {
          throw std::overflow_error("unpacker buffer overflow!");
        }
      }

      unpacker& operator>> (uint64_t & v)
      {
        v = pop_uint64();
        return *this;
      }

      unpacker& operator>> (uint32_t & v)
      {
        v = pop_uint32();
        return *this;
      }
      unpacker& operator>> (uint16_t & v)
      {
        v = pop_uint16();
        return *this;
      }
      unpacker& operator>> (uint8_t & v)
      {
        v = pop_uint8();
        return *this;
      }

      unpacker& operator>> (int64_t & v)
      {
        v = static_cast<int64_t>(pop_uint64());
        return *this;
      }
      unpacker& operator>> (int32_t & v)
      {
        v = static_cast<int32_t>(pop_uint32());
        return *this;
      }
      unpacker& operator>> (int16_t & v)
      {
        v = static_cast<int16_t>(pop_uint16());
        return *this;
      }
      unpacker& operator>> (int8_t & v)
      {
        v = static_cast<int8_t>(pop_uint8());
        return *this;
      }
      unpacker& operator>> (std::string & v)
      {
        v = pop_string();
        return *this;
      }
      template<typename T>
      unpacker& operator>> (std::vector<T> &v)
      {
        uint16_t count = pop_uint16();
        for (uint16_t i = 0; i < count; i++)
        {
          T t;
          *this >> t;
          v.push_back(t);
        }
        return *this;
      }

      template<typename K, typename V>
      unpacker& operator>> (std::map<K, V> & x)
      {
        uint16_t count = pop_uint16();
        for (uint16_t i = 0; i < count; i++)
        {
          K k;
          V v;
          *this >> k >> v;
          x.insert(std::make_pair(k, v));
        }
        return *this;
      }
    private:
      char *      buffer_;
      uint16_t    length_;
      uint16_t    position_;
      bool      copy_;
    };


#define DECLARE_PACKABLE_1_START(name,type1,name1) DECLARE_STRUCT_1_START(name,type1,name1) \
    friend packer & operator<< (packer& p, const name & x) \
    {  \
        p << x.name1; \
        return p;\
    }\
    friend unpacker & operator>> (unpacker & p, name & x) \
    {      \
      p >> x.name1; \
        return p;\
    }
#define DECLARE_PACKABLE_1(name,type1,name1) DECLARE_PACKABLE_1_START(name,type1,name1) \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_2_START(name,type1,name1,type2,name2) DECLARE_STRUCT_2_START(name,type1,name1,type2,name2) \
    friend packer & operator<< (packer& p, const name & x) \
    {  \
        p << x.name1 << x.name2; \
        return p;\
    }\
    friend unpacker & operator>> (unpacker & p, name & x) \
    {      \
      p >> x.name1 >> x.name2; \
        return p;\
    }
#define DECLARE_PACKABLE_2(name,type1,name1,type2,name2) DECLARE_PACKABLE_2_START(name,type1,name1,type2,name2) \
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
