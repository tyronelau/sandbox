#pragma once

#include "base/packer.h"

namespace agora {
namespace base {
struct packet {
  explicit packet(uint16_t u) : uri_(u) {
  }

  virtual ~packet() {
  }

  virtual void unmarshall(unpacker &p) {
    p >> uri_;
  }

  virtual void marshall(packer &p) const {
    p << uri_;
  }

  virtual void pack(packer &p) const {
    marshall(p);
    p.pack();
  }

  uint16_t uri_;
};

inline packer& operator <<(packer& p, const packet &x) {
  x.marshall(p);
  return p;
}

inline unpacker& operator >>(unpacker &p, packet &x) {
  x.unmarshall(p);
  return p;
}

}
}

#define DECLARE_PACKET_0(name, u) \
  struct name : base::packet { \
    name() : packet(u) {} \
  };

#define DECLARE_PACKET_1(name, u, type1, name1) \
  struct name : agora::base::packet { \
    type1 name1; \
    name() \
      : packet(u) \
      , name1() \
    {} \
    virtual void unmarshall(agora::base::unpacker &p) { \
      packet::unmarshall(p); \
      p >> name1; \
    } \
    virtual void marshall(agora::base::packer &p)  const { \
      packet::marshall(p); \
      p << name1; \
    } \
  };

#define DECLARE_PACKET_2(name, u, type1, name1, type2, name2) \
  struct name : agora::base::packet { \
    type1 name1; \
    type2 name2; \
    name() \
      : packet(u) \
      , name1() \
      , name2() \
    {} \
    virtual void unmarshall(agora::base::unpacker &p) { \
      packet::unmarshall(p); \
      p >> name1 >> name2; \
    } \
    virtual void marshall(agora::base::packer &p) const {  \
      packet::marshall(p); \
      p << name1 << name2; \
    } \
  };

#define DECLARE_PACKET_3(name, u, type1, name1, type2, name2, type3, name3) \
  struct name : agora::base::packet { \
    type1 name1; \
    type2 name2; \
    type3 name3; \
    name() \
      : packet(u) \
      , name1() \
      , name2() \
      , name3() \
    {} \
    virtual void unmarshall(agora::base::unpacker& p) { \
      packet::unmarshall(p); \
      p >> name1 >> name2 >> name3; \
    } \
    virtual void marshall(agora::base::packer& p) const { \
      packet::marshall(p); \
      p << name1 << name2 << name3; \
    } \
  };

#define DECLARE_PACKET_4(name, u, type1, name1, type2, name2, \
    type3, name3, type4, name4) \
  struct name : agora::base::packet { \
    type1 name1; \
    type2 name2; \
    type3 name3; \
    type4 name4; \
    name() \
      : packet(u) \
      , name1() \
      , name2() \
      , name3() \
      , name4() \
    {} \
    virtual void unmarshall(agora::base::unpacker &p) { \
      packet::unmarshall(p); \
      p >> name1 >> name2 >> name3 >> name4; \
    } \
    virtual void marshall(agora::base::packer &p) const { \
      packet::marshall(p); \
      p << name1 << name2 << name3 << name4; \
    } \
  };

#define DECLARE_PACKET_5(name, u, type1, name1, type2, name2, type3, name3, \
    type4, name4, type5, name5) \
  struct name : agora::base::packet { \
    type1 name1; \
    type2 name2; \
    type3 name3; \
    type4 name4; \
    type5 name5; \
    name() \
      : packet(u) \
      , name1() \
      , name2() \
      , name3() \
      , name4() \
      , name5() \
    {} \
    virtual void unmarshall(unpacker& p) { \
      packet::unmarshall(p); \
      p >> name1 >> name2 >> name3 >> name4 >> name5; \
    } \
    virtual void marshall(packer& p) const { \
      packet::marshall(p); \
      p << name1 << name2 << name3 << name4 << name5; \
    } \
  };

#define DECLARE_PACKET_6(name, u, type1, name1, type2, name2, type3, name3, \
    type4, name4, type5, name5, type6, name6) \
  struct name : agora::base::packet { \
    type1 name1; \
    type2 name2; \
    type3 name3; \
    type4 name4; \
    type5 name5; \
    type6 name6; \
    name() \
      : packet(u) \
      , name1() \
      , name2() \
      , name3() \
      , name4() \
      , name5() \
      , name6() \
    {} \
    virtual void unmarshall(agora::base::unpacker &p) { \
      packet::unmarshall(p); \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6; \
    } \
    virtual void marshall(agora::base::packer &p) const { \
      packet::marshall(p); \
      p << name1 << name2 << name3 << name4 << name5 << name6; \
    } \
  };

#define DECLARE_PACKET_7(name, u, type1, name1, type2, name2, type3, name3, \
    type4, name4, type5, name5, type6, name6, type7, name7) \
  struct name : agora::base::packet { \
    type1 name1; \
    type2 name2; \
    type3 name3; \
    type4 name4; \
    type5 name5; \
    type6 name6; \
    type7 name7; \
    name() \
      : packet(u) \
      , name1() \
      , name2() \
      , name3() \
      , name4() \
      , name5() \
      , name6() \
      , name7() \
    {} \
    virtual void unmarshall(agora::base::unpacker &p) { \
      packet::unmarshall(p); \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7; \
    } \
    virtual void marshall(agora::base::packer &p) const { \
      packet::marshall(p); \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7; \
    } \
  };

#define DECLARE_PACKET_8(name, u, type1, name1, type2, name2, type3, name3, \
    type4, name4, type5, name5, type6, name6, type7, name7, type8, name8) \
  struct name : agora::base::packet { \
    type1 name1; \
    type2 name2; \
    type3 name3; \
    type4 name4; \
    type5 name5; \
    type6 name6; \
    type7 name7; \
    type8 name8; \
    name() : packet(u), name1(), name2(), name3(), name4(), name5(), name6(), \
        name7(), name8() { \
    } \
    virtual void unmarshall(agora::base::unpacker &p) { \
      packet::unmarshall(p); \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 \
          >> name7 >> name8; \
    } \
    virtual void marshall(agora::base::packer &p) const { \
      packet::marshall(p); \
      p << name1 << name2 << name3 << name4 << name5 << name6 \
          << name7 << name8; \
    } \
  };

#define DECLARE_PACKET_9(name, u, type1, name1, type2, name2, type3, name3, \
    type4, name4, type5, name5, type6, name6, type7, name7, type8, name8, \
    type9, name9) \
  struct name : agora::base::packet { \
    type1 name1; \
    type2 name2; \
    type3 name3; \
    type4 name4; \
    type5 name5; \
    type6 name6; \
    type7 name7; \
    type8 name8; \
    type9 name9; \
    name() : packet(u), name1(), name2(), name3(), name4(), name5(), name6(), \
        name7(), name8(), name9() { \
    } \
    virtual void unmarshall(agora::base::unpacker &p) { \
      packet::unmarshall(p); \
      p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 \
          >> name8 >> name9; \
    } \
    virtual void marshall(agora::base::packer &p) const { \
      packet::marshall(p); \
      p << name1 << name2 << name3 << name4 << name5 << name6 << name7 \
          << name8 << name9; \
    } \
  };

