#include "base/packer.h"

#include <cstring>
#include <stdexcept>
#include <utility>

namespace agora {
namespace base {

packer::packer() : buffer_(kDefaultSize), length_(0),
    position_(sizeof(length_)) {
}

packer::~packer() {
}

packer& packer::pack() {
  length_ = position_;
  position_ = 0;
  *this << length_;
  position_ = length_;

  return *this;
}

void packer::reset() {
  length_ = 0;
  position_ = sizeof(length_);
}

void packer::write(uint16_t val, uint32_t position) {
  check_size(sizeof(val), position);
  memcpy(&buffer_[0] + position, &val, sizeof(val));
}

void packer::write(uint32_t val, uint32_t position) {
  check_size(sizeof(val), position);
  memcpy(&buffer_[0] + position, &val, sizeof(val));
}

void packer::push(uint64_t val) {
  check_size(sizeof(val), position_);
  memcpy(&buffer_[0] + position_, &val, sizeof(val));

  position_ = static_cast<uint32_t>(position_ + sizeof(val));
}

void packer::push(int64_t val) {
  push(static_cast<uint64_t>(val));
}

void packer::push(uint32_t val) {
  check_size(sizeof(val), position_);
  memcpy(&buffer_[0] + position_, &val, sizeof(val));

  position_ = static_cast<uint32_t>(position_ + sizeof(val));
}

void packer::push(int32_t val) {
  push(static_cast<uint32_t>(val));
}

std::vector<char> packer::take_buffer() {
  std::vector<char> buf(buffer_.begin(), buffer_.begin() + length_);

  length_ = 0;
  position_ = sizeof(length_);

  return buf;
}

void packer::push(uint16_t val) {
  check_size(sizeof(val), position_);
  memcpy(&buffer_[0] + position_, &val, sizeof(val));

  position_ = static_cast<uint32_t>(position_ + sizeof(val));
}

void packer::push(int16_t val) {
  push(static_cast<uint16_t>(val));
}

void packer::push(uint8_t val) {
  check_size(sizeof(val), position_);
  memcpy(&buffer_[0] + position_, &val, sizeof(val));

  position_ = static_cast<uint32_t>(position_ + sizeof(val));
}

void packer::push(int8_t val) {
  push(static_cast<uint8_t>(val));
}

void packer::push(const std::string &val) {
  push(static_cast<uint32_t>(val.length()));

  size_t length = val.length();
  check_size(length, position_);

  if (length > 0) {
    memcpy(&buffer_[0] + position_, val.data(), length);
    position_ = static_cast<uint32_t>(position_ + length);
  }
}

const char* packer::buffer() const {
  return &buffer_[0];
}

size_t packer::length() const {
  return length_;
}

std::string packer::body() const {
  return std::string(&buffer_[0] + sizeof(length_), length_ - sizeof(length_));
}

void packer::check_size(size_t more, uint32_t position) {
  if (buffer_.size() - position < more) {
    size_t new_size = (std::max)(more + buffer_.size(), buffer_.size() * 2);
    if (new_size > kMaxSize) {
      throw std::overflow_error("packer buffer overflow!");
    }

    buffer_.resize(new_size);
  }
}

packer& operator<<(packer &pkr, uint64_t v) {
  pkr.push(v);
  return pkr;
}

packer& operator<<(packer &pkr, int64_t v) {
  pkr.push(v);
  return pkr;
}

packer& operator<<(packer &pkr, uint32_t v) {
  pkr.push(v);
  return pkr;
}

packer& operator<<(packer &pkr, int32_t v) {
  pkr.push(v);
  return pkr;
}

packer& operator<<(packer &pkr, uint16_t v) {
  pkr.push(v);
  return pkr;
}

packer& operator<<(packer &pkr, int16_t v) {
  pkr.push(v);
  return pkr;
}

packer& operator<<(packer &pkr, uint8_t v) {
  pkr.push(v);
  return pkr;
}

packer& operator<<(packer &pkr, int8_t v) {
  pkr.push(v);
  return pkr;
}

packer& operator<<(packer &pkr, const std::string &v) {
  pkr.push(v);
  return pkr;
}

// Unpacker
unpacker::unpacker(const char *buf, size_t len, bool copy)
    : buffer_(NULL), length_(static_cast<uint32_t>(len)),
    position_(0), copy_(copy) {
  if (copy_) {
    char *tmp = new (std::nothrow)char[len];
    ::memcpy(tmp, buf, len);
    buffer_ = tmp;
  } else {
    buffer_ = buf;
  }
}

unpacker::unpacker(unpacker &&rhs) {
  buffer_ = rhs.buffer_;
  length_ = rhs.length_;
  position_ = rhs.position_;
  copy_ = rhs.copy_;

  rhs.buffer_ = NULL;
}

unpacker& unpacker::operator=(unpacker &&rhs) {
  if (this != &rhs) {
    std::swap(buffer_, rhs.buffer_);
    std::swap(length_, rhs.length_);
    std::swap(position_, rhs.position_);
    std::swap(copy_, rhs.copy_);
  }

  return *this;
}

unpacker::~unpacker()  {
  if (copy_) {
    delete []buffer_;
  }
}

void unpacker::rewind() {
  position_ = sizeof(length_);
}

uint64_t unpacker::pop_uint64() {
  uint64_t v = 0;
  check_size(sizeof(v), position_);
  ::memcpy(&v, buffer_ + position_, sizeof(v));
  position_ = static_cast<uint32_t>(position_ + sizeof(v));
  return v;
}

uint32_t unpacker::pop_uint32() {
  uint32_t v = 0;
  check_size(sizeof(v), position_);
  ::memcpy(&v, buffer_ + position_, sizeof(v));
  position_ = static_cast<uint32_t>(position_ + sizeof(v));
  return v;
}

uint16_t unpacker::pop_uint16() {
  uint16_t v = 0;
  check_size(sizeof(v), position_);
  ::memcpy(&v, buffer_ + position_, sizeof(v));
  position_ = static_cast<uint32_t>(position_ + sizeof(v));
  return v;
}

uint8_t unpacker::pop_uint8() {
  uint8_t v = 0;
  check_size(sizeof(v), position_);
  ::memcpy(&v, buffer_ + position_, sizeof(v));
  position_ = static_cast<uint32_t>(position_ + sizeof(v));
  return v;
}

std::string unpacker::pop_string() {
  uint32_t length = pop_uint32();
  check_size(length, position_);

  std::string s = std::string(buffer_ + position_, length);
  position_ = static_cast<uint32_t>(position_ + length);

  return s;
}

const char* unpacker::buffer() const {
  return buffer_;
}

size_t unpacker::length() const {
  return length_;
}

void unpacker::check_size(size_t more, uint32_t position) const {
  if (static_cast<size_t>(length_ - position) < more) {
    throw std::overflow_error("unpacker buffer overflow!");
  }
}

unpacker& operator>>(unpacker &unpkr, uint64_t &v) {
  v = unpkr.pop_uint64();
  return unpkr;
}

unpacker& operator>>(unpacker &unpkr, uint32_t &v) {
  v = unpkr.pop_uint32();
  return unpkr;
}

unpacker& operator>>(unpacker &unpkr, uint16_t &v) {
  v = unpkr.pop_uint16();
  return unpkr;
}

unpacker& operator>>(unpacker &unpkr, uint8_t &v) {
  v = unpkr.pop_uint8();
  return unpkr;
}

unpacker& operator>>(unpacker &unpkr, int64_t &v) {
  v = static_cast<int64_t>(unpkr.pop_uint64());
  return unpkr;
}

unpacker& operator>>(unpacker &unpkr, int32_t &v) {
  v = static_cast<int32_t>(unpkr.pop_uint32());
  return unpkr;
}

unpacker& operator>>(unpacker &unpkr, int16_t &v) {
  v = static_cast<int16_t>(unpkr.pop_uint16());
  return unpkr;
}

unpacker& operator>>(unpacker &unpkr, int8_t &v) {
  v = static_cast<int8_t>(unpkr.pop_uint8());
  return unpkr;
}

unpacker& operator>>(unpacker &unpkr, std::string &v) {
  v = unpkr.pop_string();
  return unpkr;
}

}
}
