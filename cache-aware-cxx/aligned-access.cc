#include <iostream>

using namespace std;

struct Foo {
  int c;
  char a;
  char b;
  // padding1
  // panding2
};

// Suppose we have a array of Bar, we need add paddings to the end so that
// 'c' in every element in the array is aligned. Similarly to Foo
struct Bar {
  char a;
  // padding1
  // padding2
  // padding3
  int c;
  char b;
  // padding1
  // padding2
  // padding3
};

int main() {
  std::cout << "sizeof(Foo) = " << sizeof(Foo) << std::endl; // 8
  std::cout << "sizeof(Bar) = " << sizeof(Bar) << std::endl; // 12
  return 0;
}
