#include "pyt_interpreter.h"

python_interpreter::python_interpreter() {
}

python_interpreter::~python_interpreter() {
  finalize();
}

python_interpreter* python_interpreter::get_instance() {
  static python_interpreter interpreter;
  return &interpreter;
}

bool python_interpreter::add_module(python_module *module) {
  if (modules_.find(module) != modules_.end())
    return false;

  if (!module->initialize(this))
    return false;

  modules_.insert(module);
  return true;
}

bool python_interpreter::remove_module(python_module *module) {
  if (modules_.find(module) == modules_.end())
    return false;

  modules_.erase(module);
  
  return module->finalize();
}

bool python_interpreter::initialize() {
  return Py_Initialize();
}

bool python_interpreter::finalize() {
  for (python_module *module : modules_) {
    module->finalize();
  }
  return Py_Finalize();
}

int python_interpreter::call_python(const char *scripts) {
  return PyRun_SimpleString(scripts);
}

