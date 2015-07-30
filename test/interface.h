struct base {
  virtual ~base() {}
  virtual void foo() = 0;
  virtual void bar() = 0;
};

base* create_instance();
base* destroy_instance(base *p);

