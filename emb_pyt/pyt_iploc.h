#pragma once
#include <Python.h>

namespace extension {
class IpLocDatabase;

struct IpLocDbObject {
  PyObject_HEAD
  IpLocDatabase *db_;
};

// IpLocDb methods.
// class IpLocDb(object):
//   def __init__(self):
//       pass
//   def __destroy__(self):
//       pass
//   def openDatabase(self, db_txt):
//       pass
//   def closeDatabase(self):
//       pass
//   def findIpLocation(self, ip):
//       pass

extern PyTypeObject IpLocDbType;
extern PyMemberDef IpLocDbTypeMembers[];
}

