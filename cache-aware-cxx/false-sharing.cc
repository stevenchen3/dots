#include <iostream>
#include <thread>

using namespace std;


void do_work(std::atomic<int> &a) {
  for (int i = 0; i < 10000; ++i) {
    ++a;
  }
}

int main() {
  std::atomic<int> a; a = 0;
  std::atomic<int> b; b = 0;
  std::atomic<int> c; c = 0;
  std::atomic<int> d; d = 0;

  // Prevent false sharing
  /*
  alignas(64) std::atomic<int> a; a = 0;
  alignas(64) std::atomic<int> b; b = 0;
  alignas(64) std::atomic<int> c; c = 0;
  alignas(64) std::atomic<int> d; d = 0;
  */

  std::thread t1([&]() { do_work(a); });
  std::thread t2([&]() { do_work(a); });
  std::thread t3([&]() { do_work(a); });
  std::thread t4([&]() { do_work(a); });

  t1.join();
  t2.join();
  t3.join();
  t4.join();

  return 0;
}
