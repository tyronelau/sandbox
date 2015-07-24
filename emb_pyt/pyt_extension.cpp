#include <Python.h>
#include <cstdio>

#include "pyt_iploc.h"

static PyMethodDef exports[] = {
   {NULL, NULL, 0, NULL}
};

PyObject* init_python_module() {
  extension::IpLocDbType.tp_new = PyType_GenericNew;

  if (PyType_Ready(&extension::IpLocDbType) < 0) {
    fprintf(stderr, "Initializing IpLocDbObject failed");
    return NULL;
  }

  Py_Initialize();

  static PyMethodDef ModuleMethods[] = {
    {NULL, NULL, 0, NULL}
  };

  // static PyModuleDef embedded_module = {
  //   PyModuleDef_HEAD_INIT,
  //   "strategy",
  //   NULL,
  //   -1,
  //   ModuleMethods,
  //   NULL,
  //   NULL,
  //   NULL,
  //   NULL
  // };
  
  PyObject *m = Py_InitModule("strategy", ModuleMethods);
  // PyObject *m = PyModule_Create(&embedded_module);

  Py_INCREF(reinterpret_cast<PyObject *>(&extension::IpLocDbType));
  PyModule_AddObject(m, "IpLocDatabase", reinterpret_cast<PyObject *>(
      &extension::IpLocDbType));

  return m;
}

void destroy_python_module() {
  Py_Finalize();
}

