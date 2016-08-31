#pragma once

#include "base/packer.h"

namespace agora {
  namespace base {
    struct packet
    {
      enum
      {
        SERVER_TYPE_OFFSET = 2,
        URI_OFFSET = 4,
        BODY_OFFSET = 6,
      };
      packet(uint16_t u)
        : server_type(0)
        , uri(u)
      {}
      packet(uint16_t server, uint16_t u)
        : server_type(server)
        , uri(u)
      {}
      virtual ~packet() {}

      virtual void unmarshall(unpacker & p)
      {
        p >> server_type >> uri;
      }
      virtual void marshall(packer & p) const
      {
        p << server_type << uri;
      }
      virtual void pack(packer & p) const
      {
        marshall(p);
        p.pack();
      }

      friend packer & operator<< (packer& p, const packet & x)
      {
        x.marshall(p);
        return p;
      }
      friend unpacker & operator>> (unpacker & p, packet & x)
      {
        x.unmarshall(p);
        return p;
      }

      uint16_t server_type;
      uint16_t uri;
    };

#define DECLARE_PACKET_0(name, u) \
    struct name : packet {\
      name() : packet(u) {}\
      name(uint16_t server)\
        : packet(server, u)\
      {}\
    };
#define DECLARE_PACKET_1(name, u, type1, name1) \
    struct name : base::packet {\
      type1 name1;\
      name() \
        : packet(u)\
        , name1()\
      {}\
      name(uint16_t server)\
        : packet(server, u)\
        , name1()\
      {}\
      virtual void unmarshall(base::unpacker& p) {\
        packet::unmarshall(p);\
        p >> name1;\
      }\
      virtual void marshall(base::packer& p)  const {\
        packet::marshall(p);\
        p << name1;\
      }\
    };
#define DECLARE_PACKET_2(name, u, type1, name1, type2, name2) \
    struct name : base::packet {\
      type1 name1;\
      type2 name2;\
      name() \
        : packet(u)\
        , name1()\
        , name2()\
      {}\
      name(uint16_t server)\
        : packet(server, u)\
        , name1()\
        , name2()\
      {}\
      virtual void unmarshall(base::unpacker& p) {\
        packet::unmarshall(p);\
        p >> name1 >> name2;\
      }\
      virtual void marshall(base::packer& p) const {  \
        packet::marshall(p);\
        p << name1 << name2;\
      }\
    };
#define DECLARE_PACKET_3(name, u, type1, name1, type2, name2, type3, name3) \
    struct name : base::packet {\
      type1 name1;\
      type2 name2;\
      type3 name3;\
      name() \
        : packet(u)\
        , name1()\
        , name2()\
        , name3()\
      {}\
      name(uint16_t server)\
        : packet(server, u)\
        , name1()\
        , name2()\
        , name3()\
      {}\
      virtual void unmarshall(base::unpacker& p) {\
        packet::unmarshall(p);\
        p >> name1 >> name2 >> name3;\
      }\
      virtual void marshall(base::packer& p) const {\
        packet::marshall(p);\
        p << name1 << name2 << name3;\
      }\
    };
#define DECLARE_PACKET_4_START(name, u, type1, name1, type2, name2, type3, name3, type4, name4) \
    struct name : base::packet {\
      type1 name1;\
      type2 name2;\
      type3 name3;\
      type4 name4;\
      name() \
        : packet(u)\
        , name1()\
        , name2()\
        , name3()\
        , name4()\
      {}\
      name(uint16_t server)\
        : packet(server, u)\
        , name1()\
        , name2()\
        , name3()\
        , name4()\
      {}\
      virtual void unmarshall(base::unpacker& p) {\
        packet::unmarshall(p);\
        p >> name1 >> name2 >> name3 >> name4;\
      }\
      virtual void marshall(base::packer& p) const {\
        packet::marshall(p);\
        p << name1 << name2 << name3 << name4;\
      }

#define DECLARE_PACKET_END  };
#define DECLARE_PACKET_4(name, u, type1, name1, type2, name2, type3, name3, type4, name4) DECLARE_PACKET_4_START(name, u, type1, name1, type2, name2, type3, name3, type4, name4)\
  DECLARE_PACKET_END

#define DECLARE_PACKET_5(name, u, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5) \
    struct name : base::packet {\
      type1 name1;\
      type2 name2;\
      type3 name3;\
      type4 name4;\
      type5 name5;\
      name() \
        : packet(u)\
        , name1()\
        , name2()\
        , name3()\
        , name4()\
        , name5()\
      {}\
      name(uint16_t server)\
        : packet(server, u)\
        , name1()\
        , name2()\
        , name3()\
        , name4()\
        , name5()\
      {}\
      virtual void unmarshall(unpacker& p) {\
        packet::unmarshall(p);\
        p >> name1 >> name2 >> name3 >> name4 >> name5;\
      }\
      virtual void marshall(packer& p) const {\
        packet::marshall(p);\
        p << name1 << name2 << name3 << name4 << name5;\
      }\
    };

#define DECLARE_PACKET_6(name, u, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5, type6, name6) \
    struct name : packet {\
      type1 name1;\
      type2 name2;\
      type3 name3;\
      type4 name4;\
      type5 name5;\
      type6 name6;\
      name() \
        : packet(u)\
        , name1()\
        , name2()\
        , name3()\
        , name4()\
        , name5()\
        , name6()\
      {}\
      name(uint16_t server)\
        : packet(server, u)\
        , name1()\
        , name2()\
        , name3()\
        , name4()\
        , name5()\
        , name6()\
      {}\
      virtual void unmarshall(unpacker& p)\
      {\
        packet::unmarshall(p);\
        p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6;\
      }\
      virtual void marshall(packer& p) const\
      {\
        packet::marshall(p);\
        p << name1 << name2 << name3 << name4 << name5 << name6;\
      }\
    };

#define DECLARE_PACKET_7(name, u, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5, type6, name6, type7, name7) \
    struct name : packet {\
      type1 name1;\
      type2 name2;\
      type3 name3;\
      type4 name4;\
      type5 name5;\
      type6 name6;\
      type7 name7;\
      name() \
        : packet(u)\
        , name1()\
        , name2()\
        , name3()\
        , name4()\
        , name5()\
        , name6()\
        , name7()\
      {}\
      name(uint16_t server)\
        : packet(server, u)\
        , name1()\
        , name2()\
        , name3()\
        , name4()\
        , name5()\
        , name6()\
        , name7()\
      {}\
      virtual void unmarshall(unpacker& p)\
      {\
        packet::unmarshall(p);\
        p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7;\
      }\
      virtual void marshall(packer& p) const\
      {\
        packet::marshall(p);\
        p << name1 << name2 << name3 << name4 << name5 << name6 << name7;\
      }\
    };
#define DECLARE_PACKET_8(name,u,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8) \
struct name : packet {\
    type1 name1;\
    type2 name2;\
    type3 name3;\
    type4 name4;\
    type5 name5;\
    type6 name6;\
    type7 name7;\
    type8 name8;\
    name(): packet(u),name1(),name2(),name3(),name4(),name5(),name6(),name7(),name8(){}\
    name(uint16_t server): packet(server, u),name1(),name2(),name3(),name4(),name5(),name6(),name7(),name8(){}\
    virtual void unmarshall(unpacker& p) {\
        packet::unmarshall(p);\
        p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8;\
    }\
    virtual void marshall(packer& p) const {\
        packet::marshall(p);\
        p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8;\
    }\
};

#define DECLARE_PACKET_9(name,u,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8,type9,name9) \
struct name : packet {\
    type1 name1;\
    type2 name2;\
    type3 name3;\
    type4 name4;\
    type5 name5;\
    type6 name6;\
    type7 name7;\
    type8 name8;\
    type9 name9;\
    name(): packet(u),name1(),name2(),name3(),name4(),name5(),name6(),name7(),name8(),name9(){}\
    name(uint16_t server): packet(server, u),name1(),name2(),name3(),name4(),name5(),name6(),name7(),name8(),name9(){}\
    virtual void unmarshall(unpacker& p) {\
        packet::unmarshall(p);\
        p >> name1 >> name2 >> name3 >> name4 >> name5 >> name6 >> name7 >> name8 >> name9;\
    }\
    virtual void marshall(packer& p) const {\
        packet::marshall(p);\
        p << name1 << name2 << name3 << name4 << name5 << name6 << name7 << name8 << name9;\
    }\
};

  }
}
