#pragma once
#include <unordered_set>

class python_module;
class python_interpreter {
 private:
  python_interpreter();
  ~python_interpreter();
 public:
  static python_interpreter* get_instance();

  bool add_module(python_module *module);
  bool remove_module(python_module *module);

  bool initialize();
  bool finalize();

  int call_python(const char *scripts);
 private:
  std::unordered_set<python_module *> modules_;
};

