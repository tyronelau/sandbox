#pragma once

class python_module {
 public:
  explicit python_module(const char *name);
  ~python_module();
  
  bool initialize();
  bool finalize();

  bool add_class(python_class *cls);
  // bool remove_class(python_class *cls);
 private:
};

