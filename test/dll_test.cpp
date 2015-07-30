#include "interface.h"

#include <iostream>
#include <vector>

struct derived : public base {
  derived() { a_.resize(100); }
  virtual ~derived() {}

  virtual void foo() {
    // std::cout << "foo" << std::endl;
  }

  virtual void bar() {
    // std::cout << "bar" << std::endl;
  }

  std::vector<int> a_;
};

__attribute__((visibility("default"))) base* create_instance() {
  return new derived;
}

__attribute__((visibility("default"))) void destory_instance(base *p) {
  delete p;
}

