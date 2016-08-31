#pragma once

namespace agora { namespace base {
#define DECLARE_STRUCT_END };

#define DECLARE_SIMPLE_STRUCT_1_START(name,type1,name1) \
  struct name { \
    type1 name1;\
    name():name1(){}\
    name(type1 n1):name1(n1){}\
    name(const name& r):name1(r.name1){}\
    name & operator=(const name & r) { \
      if (this == &r) return *this; \
      name1 = r.name1; \
      return *this; \
    }
#define DECLARE_SIMPLE_STRUCT_1(name,type1,name1) DECLARE_SIMPLE_STRUCT_1_START(name,type1,name1) \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_2_START(name,type1,name1,type2,name2) \
  struct name { \
    type1 name1;\
    type2 name2;\
    name():name1(),name2(){}\
    name(type1 n1,type2 n2):name1(n1),name2(n2){}\
    name(const name& r):name1(r.name1),name2(r.name2){}\
    name & operator=(const name & r) { \
      if (this == &r) return *this; \
      name1 = r.name1; \
      name2 = r.name2; \
      return *this; \
    }
#define DECLARE_SIMPLE_STRUCT_2(name,type1,name1,type2,name2) DECLARE_SIMPLE_STRUCT_2_START(name,type1,name1,type2,name2) \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_3_START(name,type1,name1,type2,name2,type3,name3) \
  struct name { \
    type1 name1;\
    type2 name2;\
    type3 name3;\
    name():name1(),name2(),name3(){}\
    name(type1 n1,type2 n2,type3 n3):name1(n1),name2(n2),name3(n3){}\
    name(const name& r):name1(r.name1),name2(r.name2),name3(r.name3){}\
    name & operator=(const name & r) { \
      if (this == &r) return *this; \
      name1 = r.name1; \
      name2 = r.name2; \
      name3 = r.name3; \
      return *this; \
    }
#define DECLARE_SIMPLE_STRUCT_3(name,type1,name1,type2,name2,type3,name3) DECLARE_SIMPLE_STRUCT_3_START(name,type1,name1,type2,name2,type3,name3) \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_4_START(name,type1,name1,type2,name2,type3,name3,type4,name4) \
  struct name { \
    type1 name1;\
    type2 name2;\
    type3 name3;\
    type4 name4;\
    name():name1(),name2(),name3(),name4(){}\
    name(type1 n1,type2 n2,type3 n3,type4 n4):name1(n1),name2(n2),name3(n3),name4(n4){}\
    name(const name& r):name1(r.name1),name2(r.name2),name3(r.name3),name4(r.name4){}\
    name & operator=(const name & r) { \
      if (this == &r) return *this; \
      name1 = r.name1; \
      name2 = r.name2; \
      name3 = r.name3; \
      name4 = r.name4; \
      return *this; \
    }
#define DECLARE_SIMPLE_STRUCT_4(name,type1,name1,type2,name2,type3,name3,type4,name4) DECLARE_SIMPLE_STRUCT_4_START(name,type1,name1,type2,name2,type3,name3,type4,name4) \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_5_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5) \
  struct name { \
    type1 name1;\
    type2 name2;\
    type3 name3;\
    type4 name4;\
    type5 name5;\
    name():name1(),name2(),name3(),name4(),name5(){}\
    name(type1 n1,type2 n2,type3 n3,type4 n4,type5 n5):name1(n1),name2(n2),name3(n3),name4(n4),name5(n5){}\
    name(const name& r):name1(r.name1),name2(r.name2),name3(r.name3),name4(r.name4),name5(r.name5){}\
    name & operator=(const name & r) { \
      if (this == &r) return *this; \
      name1 = r.name1; \
      name2 = r.name2; \
      name3 = r.name3; \
      name4 = r.name4; \
      name5 = r.name5; \
      return *this; \
    }
#define DECLARE_SIMPLE_STRUCT_5(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5) DECLARE_SIMPLE_STRUCT_5_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5) \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_6_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6) \
  struct name { \
    type1 name1;\
    type2 name2;\
    type3 name3;\
    type4 name4;\
    type5 name5;\
    type6 name6;\
    name():name1(),name2(),name3(),name4(),name5(),name6(){}\
    name(type1 n1,type2 n2,type3 n3,type4 n4,type5 n5,type6 n6):name1(n1),name2(n2),name3(n3),name4(n4),name5(n5),name6(n6){}\
    name(const name& r):name1(r.name1),name2(r.name2),name3(r.name3),name4(r.name4),name5(r.name5),name6(r.name6){}\
    name & operator=(const name & r) { \
      if (this == &r) return *this; \
      name1 = r.name1; \
      name2 = r.name2; \
      name3 = r.name3; \
      name4 = r.name4; \
      name5 = r.name5; \
      name6 = r.name6; \
      return *this; \
    }
#define DECLARE_SIMPLE_STRUCT_6(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6) DECLARE_SIMPLE_STRUCT_6_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6) \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_7_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7) \
  struct name { \
    type1 name1;\
    type2 name2;\
    type3 name3;\
    type4 name4;\
    type5 name5;\
    type6 name6;\
    type7 name7;\
    name():name1(),name2(),name3(),name4(),name5(),name6(),name7(){}\
    name(type1 n1,type2 n2,type3 n3,type4 n4,type5 n5,type6 n6,type7 n7):name1(n1),name2(n2),name3(n3),name4(n4),name5(n5),name6(n6),name7(n7){}\
    name(const name& r):name1(r.name1),name2(r.name2),name3(r.name3),name4(r.name4),name5(r.name5),name6(r.name6),name7(r.name7){}\
    name & operator=(const name & r) { \
      if (this == &r) return *this; \
      name1 = r.name1; \
      name2 = r.name2; \
      name3 = r.name3; \
      name4 = r.name4; \
      name5 = r.name5; \
      name6 = r.name6; \
      name7 = r.name7; \
      return *this; \
    }
#define DECLARE_SIMPLE_STRUCT_7(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7) DECLARE_SIMPLE_STRUCT_7_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7) \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_8_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8) \
  struct name { \
    type1 name1;\
    type2 name2;\
    type3 name3;\
    type4 name4;\
    type5 name5;\
    type6 name6;\
    type7 name7;\
    type8 name8;\
    name():name1(),name2(),name3(),name4(),name5(),name6(),name7(),name8(){}\
    name(type1 n1,type2 n2,type3 n3,type4 n4,type5 n5,type6 n6,type7 n7,type8 n8):name1(n1),name2(n2),name3(n3),name4(n4),name5(n5),name6(n6),name7(n7),name8(n8){}\
    name(const name& r):name1(r.name1),name2(r.name2),name3(r.name3),name4(r.name4),name5(r.name5),name6(r.name6),name7(r.name7),name8(r.name8){}\
    name & operator=(const name & r) { \
      if (this == &r) return *this; \
      name1 = r.name1; \
      name2 = r.name2; \
      name3 = r.name3; \
      name4 = r.name4; \
      name5 = r.name5; \
      name6 = r.name6; \
      name7 = r.name7; \
      name8 = r.name8; \
      return *this; \
    }
#define DECLARE_SIMPLE_STRUCT_8(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8) DECLARE_SIMPLE_STRUCT_8_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8) \
  DECLARE_STRUCT_END

#define DECLARE_SIMPLE_STRUCT_9_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8,type9,name9) \
  struct name { \
    type1 name1;\
    type2 name2;\
    type3 name3;\
    type4 name4;\
    type5 name5;\
    type6 name6;\
    type7 name7;\
    type8 name8;\
    type9 name9;\
    name():name1(),name2(),name3(),name4(),name5(),name6(),name7(),name8(),name9(){}\
    name(type1 n1,type2 n2,type3 n3,type4 n4,type5 n5,type6 n6,type7 n7,type8 n8,type9 n9):name1(n1),name2(n2),name3(n3),name4(n4),name5(n5),name6(n6),name7(n7),name8(n8),name9(n9){}\
    name(const name& r):name1(r.name1),name2(r.name2),name3(r.name3),name4(r.name4),name5(r.name5),name6(r.name6),name7(r.name7),name8(r.name8),name9(r.name9){}\
    name & operator=(const name & r) { \
      if (this == &r) return *this; \
      name1 = r.name1; \
      name2 = r.name2; \
      name3 = r.name3; \
      name4 = r.name4; \
      name5 = r.name5; \
      name6 = r.name6; \
      name7 = r.name7; \
      name8 = r.name8; \
      name9 = r.name9; \
      return *this; \
    }
#define DECLARE_SIMPLE_STRUCT_9(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8,type9,name9) DECLARE_SIMPLE_STRUCT_9_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8,type9,name9) \
  DECLARE_STRUCT_END


#define DECLARE_STRUCT_1_START(name,type1,name1) DECLARE_SIMPLE_STRUCT_1_START(name,type1,name1) \
    friend bool operator<(const name & l, const name & r) \
    { \
      if (l.name1 != r.name1) \
      { \
        return l.name1 < r.name1; \
      } \
     return false; \
   } \
   friend bool operator==(const name & l, const name & r) \
   { \
      if (l.name1 != r.name1) \
      { \
        return false; \
      } \
      return true; \
    } \
    friend bool operator!=(const name & l, const name & r) \
    { \
      return !(l == r); \
    }

#define DECLARE_STRUCT_1(name,type1,name1) DECLARE_STRUCT_1_START(name,type1,name1) \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_2_START(name,type1,name1,type2,name2) DECLARE_SIMPLE_STRUCT_2_START(name,type1,name1,type2,name2) \
    friend bool operator<(const name & l, const name & r) \
    { \
      if (l.name1 != r.name1) \
      { \
        return l.name1 < r.name1; \
      } \
      if (l.name2 != r.name2) \
      { \
        return l.name2 < r.name2; \
      } \
     return false; \
   } \
   friend bool operator==(const name & l, const name & r) \
   { \
      if (l.name1 != r.name1) \
      { \
        return false; \
      } \
      if (l.name2 != r.name2) \
      { \
        return false; \
      } \
      return true; \
    } \
    friend bool operator!=(const name & l, const name & r) \
    { \
      return !(l == r); \
    }

#define DECLARE_STRUCT_2(name,type1,name1,type2,name2) DECLARE_STRUCT_2_START(name,type1,name1,type2,name2) \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_3_START(name,type1,name1,type2,name2,type3,name3) DECLARE_SIMPLE_STRUCT_3_START(name,type1,name1,type2,name2,type3,name3) \
    friend bool operator<(const name & l, const name & r) \
    { \
      if (l.name1 != r.name1) \
      { \
        return l.name1 < r.name1; \
      } \
      if (l.name2 != r.name2) \
      { \
        return l.name2 < r.name2; \
      } \
      if (l.name3 != r.name3) \
      { \
        return l.name3 < r.name3; \
      } \
     return false; \
   } \
   friend bool operator==(const name & l, const name & r) \
   { \
      if (l.name1 != r.name1) \
      { \
        return false; \
      } \
      if (l.name2 != r.name2) \
      { \
        return false; \
      } \
      if (l.name3 != r.name3) \
      { \
        return false; \
      } \
      return true; \
    } \
    friend bool operator!=(const name & l, const name & r) \
    { \
      return !(l == r); \
    }

#define DECLARE_STRUCT_3(name,type1,name1,type2,name2,type3,name3) DECLARE_STRUCT_3_START(name,type1,name1,type2,name2,type3,name3) \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_4_START(name,type1,name1,type2,name2,type3,name3,type4,name4) DECLARE_SIMPLE_STRUCT_4_START(name,type1,name1,type2,name2,type3,name3,type4,name4) \
    friend bool operator<(const name & l, const name & r) \
    { \
      if (l.name1 != r.name1) \
      { \
        return l.name1 < r.name1; \
      } \
      if (l.name2 != r.name2) \
      { \
        return l.name2 < r.name2; \
      } \
      if (l.name3 != r.name3) \
      { \
        return l.name3 < r.name3; \
      } \
      if (l.name4 != r.name4) \
      { \
        return l.name4 < r.name4; \
      } \
     return false; \
   } \
   friend bool operator==(const name & l, const name & r) \
   { \
      if (l.name1 != r.name1) \
      { \
        return false; \
      } \
      if (l.name2 != r.name2) \
      { \
        return false; \
      } \
      if (l.name3 != r.name3) \
      { \
        return false; \
      } \
      if (l.name4 != r.name4) \
      { \
        return false; \
      } \
      return true; \
    } \
    friend bool operator!=(const name & l, const name & r) \
    { \
      return !(l == r); \
    }

#define DECLARE_STRUCT_4(name,type1,name1,type2,name2,type3,name3,type4,name4) DECLARE_STRUCT_4_START(name,type1,name1,type2,name2,type3,name3,type4,name4) \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_5_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5) DECLARE_SIMPLE_STRUCT_5_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5) \
    friend bool operator<(const name & l, const name & r) \
    { \
      if (l.name1 != r.name1) \
      { \
        return l.name1 < r.name1; \
      } \
      if (l.name2 != r.name2) \
      { \
        return l.name2 < r.name2; \
      } \
      if (l.name3 != r.name3) \
      { \
        return l.name3 < r.name3; \
      } \
      if (l.name4 != r.name4) \
      { \
        return l.name4 < r.name4; \
      } \
      if (l.name5 != r.name5) \
      { \
        return l.name5 < r.name5; \
      } \
     return false; \
   } \
   friend bool operator==(const name & l, const name & r) \
   { \
      if (l.name1 != r.name1) \
      { \
        return false; \
      } \
      if (l.name2 != r.name2) \
      { \
        return false; \
      } \
      if (l.name3 != r.name3) \
      { \
        return false; \
      } \
      if (l.name4 != r.name4) \
      { \
        return false; \
      } \
      if (l.name5 != r.name5) \
      { \
        return false; \
      } \
      return true; \
    } \
    friend bool operator!=(const name & l, const name & r) \
    { \
      return !(l == r); \
    }

#define DECLARE_STRUCT_5(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5) DECLARE_STRUCT_5_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5) \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_6_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6) DECLARE_SIMPLE_STRUCT_6_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6) \
    friend bool operator<(const name & l, const name & r) \
    { \
      if (l.name1 != r.name1) \
      { \
        return l.name1 < r.name1; \
      } \
      if (l.name2 != r.name2) \
      { \
        return l.name2 < r.name2; \
      } \
      if (l.name3 != r.name3) \
      { \
        return l.name3 < r.name3; \
      } \
      if (l.name4 != r.name4) \
      { \
        return l.name4 < r.name4; \
      } \
      if (l.name5 != r.name5) \
      { \
        return l.name5 < r.name5; \
      } \
      if (l.name6 != r.name6) \
      { \
        return l.name6 < r.name6; \
      } \
     return false; \
   } \
   friend bool operator==(const name & l, const name & r) \
   { \
      if (l.name1 != r.name1) \
      { \
        return false; \
      } \
      if (l.name2 != r.name2) \
      { \
        return false; \
      } \
      if (l.name3 != r.name3) \
      { \
        return false; \
      } \
      if (l.name4 != r.name4) \
      { \
        return false; \
      } \
      if (l.name5 != r.name5) \
      { \
        return false; \
      } \
      if (l.name6 != r.name6) \
      { \
        return false; \
      } \
      return true; \
    } \
    friend bool operator!=(const name & l, const name & r) \
    { \
      return !(l == r); \
    }

#define DECLARE_STRUCT_6(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6) DECLARE_STRUCT_6_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6) \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_7_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7) DECLARE_SIMPLE_STRUCT_7_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7) \
    friend bool operator<(const name & l, const name & r) \
    { \
      if (l.name1 != r.name1) \
      { \
        return l.name1 < r.name1; \
      } \
      if (l.name2 != r.name2) \
      { \
        return l.name2 < r.name2; \
      } \
      if (l.name3 != r.name3) \
      { \
        return l.name3 < r.name3; \
      } \
      if (l.name4 != r.name4) \
      { \
        return l.name4 < r.name4; \
      } \
      if (l.name5 != r.name5) \
      { \
        return l.name5 < r.name5; \
      } \
      if (l.name6 != r.name6) \
      { \
        return l.name6 < r.name6; \
      } \
      if (l.name7 != r.name7) \
      { \
        return l.name7 < r.name7; \
      } \
     return false; \
   } \
   friend bool operator==(const name & l, const name & r) \
   { \
      if (l.name1 != r.name1) \
      { \
        return false; \
      } \
      if (l.name2 != r.name2) \
      { \
        return false; \
      } \
      if (l.name3 != r.name3) \
      { \
        return false; \
      } \
      if (l.name4 != r.name4) \
      { \
        return false; \
      } \
      if (l.name5 != r.name5) \
      { \
        return false; \
      } \
      if (l.name6 != r.name6) \
      { \
        return false; \
      } \
      if (l.name7 != r.name7) \
      { \
        return false; \
      } \
      return true; \
    } \
    friend bool operator!=(const name & l, const name & r) \
    { \
      return !(l == r); \
    }

#define DECLARE_STRUCT_7(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7) DECLARE_STRUCT_7_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7) \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_8_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8) DECLARE_SIMPLE_STRUCT_8_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8) \
    friend bool operator<(const name & l, const name & r) \
    { \
      if (l.name1 != r.name1) \
      { \
        return l.name1 < r.name1; \
      } \
      if (l.name2 != r.name2) \
      { \
        return l.name2 < r.name2; \
      } \
      if (l.name3 != r.name3) \
      { \
        return l.name3 < r.name3; \
      } \
      if (l.name4 != r.name4) \
      { \
        return l.name4 < r.name4; \
      } \
      if (l.name5 != r.name5) \
      { \
        return l.name5 < r.name5; \
      } \
      if (l.name6 != r.name6) \
      { \
        return l.name6 < r.name6; \
      } \
      if (l.name7 != r.name7) \
      { \
        return l.name7 < r.name7; \
      } \
      if (l.name8 != r.name8) \
      { \
        return l.name8 < r.name8; \
      } \
     return false; \
   } \
   friend bool operator==(const name & l, const name & r) \
   { \
      if (l.name1 != r.name1) \
      { \
        return false; \
      } \
      if (l.name2 != r.name2) \
      { \
        return false; \
      } \
      if (l.name3 != r.name3) \
      { \
        return false; \
      } \
      if (l.name4 != r.name4) \
      { \
        return false; \
      } \
      if (l.name5 != r.name5) \
      { \
        return false; \
      } \
      if (l.name6 != r.name6) \
      { \
        return false; \
      } \
      if (l.name7 != r.name7) \
      { \
        return false; \
      } \
      if (l.name8 != r.name8) \
      { \
        return false; \
      } \
      return true; \
    } \
    friend bool operator!=(const name & l, const name & r) \
    { \
      return !(l == r); \
    }

#define DECLARE_STRUCT_8(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8) DECLARE_STRUCT_8_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8) \
  DECLARE_STRUCT_END
#define DECLARE_STRUCT_9_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8,type9,name9) DECLARE_SIMPLE_STRUCT_9_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8,type9,name9) \
    friend bool operator<(const name & l, const name & r) \
    { \
      if (l.name1 != r.name1) \
      { \
        return l.name1 < r.name1; \
      } \
      if (l.name2 != r.name2) \
      { \
        return l.name2 < r.name2; \
      } \
      if (l.name3 != r.name3) \
      { \
        return l.name3 < r.name3; \
      } \
      if (l.name4 != r.name4) \
      { \
        return l.name4 < r.name4; \
      } \
      if (l.name5 != r.name5) \
      { \
        return l.name5 < r.name5; \
      } \
      if (l.name6 != r.name6) \
      { \
        return l.name6 < r.name6; \
      } \
      if (l.name7 != r.name7) \
      { \
        return l.name7 < r.name7; \
      } \
      if (l.name8 != r.name8) \
      { \
        return l.name8 < r.name8; \
      } \
      if (l.name9 != r.name9) \
      { \
        return l.name9 < r.name9; \
      } \
     return false; \
   } \
   friend bool operator==(const name & l, const name & r) \
   { \
      if (l.name1 != r.name1) \
      { \
        return false; \
      } \
      if (l.name2 != r.name2) \
      { \
        return false; \
      } \
      if (l.name3 != r.name3) \
      { \
        return false; \
      } \
      if (l.name4 != r.name4) \
      { \
        return false; \
      } \
      if (l.name5 != r.name5) \
      { \
        return false; \
      } \
      if (l.name6 != r.name6) \
      { \
        return false; \
      } \
      if (l.name7 != r.name7) \
      { \
        return false; \
      } \
      if (l.name8 != r.name8) \
      { \
        return false; \
      } \
      if (l.name9 != r.name9) \
      { \
        return false; \
      } \
      return true; \
    } \
    friend bool operator!=(const name & l, const name & r) \
    { \
      return !(l == r); \
    }

#define DECLARE_STRUCT_9(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8,type9,name9) DECLARE_STRUCT_9_START(name,type1,name1,type2,name2,type3,name3,type4,name4,type5,name5,type6,name6,type7,name7,type8,name8,type9,name9) \
  DECLARE_STRUCT_END

} }

