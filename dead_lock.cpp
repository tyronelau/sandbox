#include <iostream>
#include <functional>
#include <mutex>
#include <thread>

using namespace std;

void foo(mutex &a, mutex &b) {
  a.lock();
  sleep(1);
  b.lock();

  for (int i = 0; i < 10000; ++i) {
    cout << i << " in foo" << endl;
  }

  b.unlock();
  a.unlock();
}

void bar(mutex &a, mutex &b) {
  b.lock();
  sleep(1);
  a.lock();

  for (int i = 0; i < 10000; ++i) {
    cout << i << " in bar" << endl;
  }

  a.unlock();
  b.unlock();
}

int main() {
  mutex a, b;
  thread t1(foo, ref(a), ref(b));
  thread t2(bar, ref(a), ref(b));
  t1.join();
  t2.join();
  
  cout << "I'm done??" << endl;
  return 0;
}

