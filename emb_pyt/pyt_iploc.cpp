#include <Python.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <new>

#include <structmember.h>

#include "pyt_iploc.h"
#include "iploc_database.h"

namespace extension {

void IpLocDatabase_Dealloc(IpLocDbObject *self) {
  delete self->db_;
  self->ob_type->tp_free(reinterpret_cast<PyObject *>(self));
}

int IpLocDatabase_Init(IpLocDbObject *self, PyObject *args, PyObject *kwds) {
  (void)self;
  (void)args;
  (void)kwds;

  self->db_ = new (std::nothrow) IpLocDatabase();
  return 0;
}

PyObject* IpLocDatabase_OpenDatabase(IpLocDbObject *self, PyObject *args) {
  char *db_path = NULL;

  if (!PyArg_ParseTuple(args, "s", &db_path)) {
    fprintf(stderr, "Invalid arguments in calling function");
    PyErr_SetFromErrno(PyExc_StandardError);
    return NULL;
  }

  bool success = false;
Py_BEGIN_ALLOW_THREADS
  success = self->db_->open_database(db_path);
Py_END_ALLOW_THREADS

  if (!success) {
    PyErr_SetFromErrno(PyExc_IOError);
    return NULL;
  }

  Py_IncRef(Py_None);
  return Py_None;
}

PyObject* IpLocDatabase_CloseDatabase(IpLocDbObject *self) {
  if (self->db_) {
    delete self->db_;
    self->db_ = NULL;
  }

  Py_IncRef(Py_None);
  return Py_None;
}

PyObject* IpLocDatabase_FindIpLocation(IpLocDbObject *self, PyObject *args) {
  if (!self->db_) {
    PyErr_SetString(PyExc_MemoryError, "db is NULL");
    return NULL;
  }

  PyObject *p = NULL;
  if (!PyArg_ParseTuple(args, "O", &p)) {
    PyErr_SetString(PyExc_TypeError, "No argument provided");
    return NULL;
  }

  uint32_t ip = 0;
  if (PyInt_Check(p)) {
    ip = static_cast<uint32_t>(PyInt_AsUnsignedLongMask(p));
  } else if (PyString_Check(p)) {
    const char *ip_str = PyString_AsString(p);
    ip = inet_addr(ip_str);
  } else {
    PyErr_SetString(PyExc_TypeError, "Required argument: int or string");
    return  NULL;
  }

  ip = ntohl(ip);

  extension::GeoInfo info;
  bool success = self->db_->find_location(ip, &info);
  if (!success) {
    Py_IncRef(Py_None);
    return Py_None;
  }

  return Py_BuildValue("(s, s, s, s, d, d)", info.country_name.c_str(),
      info.country_abbr.c_str(), info.province.c_str(),
      info.city.c_str(), info.longtitude, info.lattitude);
}

PyMemberDef IpLocDbTypeMembers[] = {
  {NULL, 0, 0, 0, NULL}  // now no members
};

PyMethodDef IpLocDbTypeMethods[] = {
  {"OpenDatabase", (PyCFunction)IpLocDatabase_OpenDatabase, METH_VARARGS,
    "open the ip database"},
  {"CloseDatabase", (PyCFunction)IpLocDatabase_CloseDatabase, METH_NOARGS,
    "Close the ip database"},
  {"FindIpLocation", (PyCFunction)IpLocDatabase_FindIpLocation, METH_VARARGS,
    "lookup the ip location"},
  {NULL, NULL, 0, NULL}
};

PyTypeObject IpLocDbType = {
      PyObject_HEAD_INIT(NULL)
      0,                         /*ob_size*/
      "pyiploc.IpLocDatabase",        /*tp_name*/
      sizeof(IpLocDbObject),   /*tp_basicsize*/
      0,                         /*tp_itemsize*/
      (destructor)IpLocDatabase_Dealloc, /*tp_dealloc*/
      0,                         /*tp_print*/
      0,                         /*tp_getattr*/
      0,                         /*tp_setattr*/
      0,                         /*tp_compare*/
      0,                         /*tp_repr*/
      0,                         /*tp_as_number*/
      0,                         /*tp_as_sequence*/
      0,                         /*tp_as_mapping*/
      0,                         /*tp_hash */
      0,                         /*tp_call*/
      0,                         /*tp_str*/
      0,                         /*tp_getattro*/
      0,                         /*tp_setattro*/
      0,                         /*tp_as_buffer*/
      Py_TPFLAGS_DEFAULT,        /*tp_flags*/
      "IpLocDatabase implementation",     /* tp_doc  */
      0,                   /* tp_traverse */
      0,                   /* tp_clear */
      0,                   /* tp_richcompare */
      0,                   /* tp_weaklistoffset */
      0,                   /* tp_iter */
      0,                   /* tp_iternext */
      IpLocDbTypeMethods,        /* tp_methods */
      IpLocDbTypeMembers,             /* tp_members*/
      0,                         /* tp_getset*/
      0,                         /* tp_base*/
      0,                         /* tp_dict*/
      0,                         /* tp_descr_get */
      0,                         /* tp_descr_set */
      0,                         /* tp_dictoffset* */
      (initproc)IpLocDatabase_Init,      /* tp_init */
      0,                         /* tp_alloc* */
      0,                 /* tp_new */
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0
};
}

