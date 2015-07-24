#pragma once
template <class class_name>
struct method_helper {
 public:
  static PyObject* wrapper_method(class_name *self, PyObject *args);
};

template <class class_name, class ...args>
method_helper<> wrapper_method_helper(PyObject* (*pf)(PObject *, args...)) {
  return 
}

template <class class_name, class 
#define DECLARE_PYTHON_CLASS(class_name) \
  public: \
   static bool register_class(); \
   static bool unregister_class(); \
  private: \
   static python_method python_methods_[];

#define DEFINE_PYTHON_CLASS_START(class_name, cpp_class_name) \
  PyMethodDef cpp_class_name::python_methods_[] = {

#define DEFINE_PYTHON_CLASS_METHOD(python_method, cpp_method) \
  {#python_method, method_helper(&cpp_method)::wrapper_method, \
      method_helper(&cpp_method)::METHOD_ARGS, #python_method \
    "converted from " #cpp_method},

#define DEFINE_PYTHON_CLASS_END(class_name) \
  } \
  bool class_name::register_class() { \
    return python_instance()->register_myself(class_name); \
  } \
  bool class_name::unregister_class() { \
    return python_instance()->unregister_myself(class_name); \
  }

class python_class {
 protected:
  python_class(PyObject *self, PyObject *args, PyObject *kwds);
  ~python_class();
 protected:
  void add_method(PyFunction *method, PyObject *args, PyObject *kwds);
 private:
  DECLARE_PYTHON_CLASS(MyClass);
};

