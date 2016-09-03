#include <functional>
#include <iostream>
#include <string>

struct foo {
  typedef std::function<int (int, double, int)> function_type_t;

  template <typename Callable, typename ...Args>
  void set_callback(Callable &&f, Args&&... args);

  void call_func() {
    func_(1, 2.0, 3);
  }

  function_type_t func_;
};

template <typename Callable, typename... Args>
void foo::set_callback(Callable &&f, Args&&... args) {
  func_ = std::bind<int>(std::forward<Callable>(f),
      std::forward<Args>(args)...);
}

struct bar {
  int void_func();
  int int_func(int a, double b);
  int string_func(const std::string &b, int c);
};

int bar::void_func() {
  std::cout << "void func called" << std::endl;
  return 0;
}

int bar::int_func(int a, double b) {
  std::cout << "int func called " << a << ", " << b << std::endl;
  return 0;
}

int bar::string_func(const std::string &b, int c) {
  std::cout << "string func called " << b << ", " << c << std::endl;
  return 0;
}

int main() {
  foo a;
  bar b;
  a.set_callback(&bar::void_func, &b);
  a.call_func();

  a.set_callback(&bar::int_func, &b, 1, 2.0);
  a.call_func();

  a.set_callback(&bar::string_func, &b, "abcd", 3);
  a.call_func();
  return 0;
}

