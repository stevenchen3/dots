#include <iostream>

using namespace std;

// Generally, for primitive data types that require x bytes, the address must be multiple of x
// e.g., x86_64, int requires 4 bytes, thus, int must be stored at address 0, 4, 8, 16, ...

// char: 0, 1, 2, 3, ...
// int : 0, 4, 8, 16, ...
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

// every object of type Foobar will be aligned to 16-byte boundary
struct alignas(16) Foobar {
  char a;
  int c;
  char b;
};

int main() {
  std::cout << sizeof(Foo) << std::endl;    // 8
  std::cout << sizeof(Bar) << std::endl;    // 12
  std::cout << sizeof(Foobar) << std::endl; // 16
}
