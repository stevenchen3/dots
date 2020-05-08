# Cache-aware C++


## 2D array traversal

Row-oriented (row-major) traversal

```c++
constexpr int m = 128;
constexpr int n = 1048576;
auto A = new int[m][n]; // 128M

for (int i = 0; i < m; ++i) {
  for (int j = 0; j < n; ++j) {
    A[i][j] += j;
  }
}
```

Column-oriented (column-major) traversal

```c++
for (int j = 0; j < n; ++j) {
  for (int i = 0; i < m; ++i) {
    A[i][j] += j;
  }
}
```

Two dimensional array is organized as multiple arrays in memory, e.g.,
`A[0][0],...,A[0][n-1],A[1][0],...,A[1][n-1], ..., A[m-1][n-1]` and CPU access consecutive memory
addresses much faster than random access. Thus, in above example, the row-major access would
perform much faster than the column-major one.


## Aligned memory access

CPU does not directly fetch data from memory, instead it fetches from caches (e.g., L1, L2, L3).
When requested data is not available in cache, it then fetches from memory. And each fetch is per
cache line size, e.g., 64 bytes. Therefore, if the requested data resides in a single cache line, we
could gain better performance than it locates in multiple cache lines. Generally, for primitive data
types that require x bytes, the address must be multiple of x e.g., x86_64, int requires 4 bytes,
thus, int must be stored at address 0, 4, 8, 16, ... Compiler would add paddings to our data
structures.


Take below snippets as an example, basically, `Foo` and `Bar` define the same thing. However, in a
`x86_64` mac OS, `Foo` takes 8 bytes, while `Bar` needs 12 bytes.

```c++
struct Foo {
  int c;
  char a;
  char b;
  // padding1
  // panding2
};

// Suppose we have a array of Bar, we need add paddings to the end so that
// 'c' in every element in the array is aligned.
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
```


## False sharing

Given below example, there's nothing shared between threads. Supposedly, the runtime between program
1 and program 2 should be roughly the same in a multi-core computer. However, program 2 actually
runs twice slower than program 1. Recall data alignment that how memory looks like (which is like as
multiple cache lines. In a computer with cache line of 64 bytes, variables a, b, c, d are aligned
with cache line (in the same cache line). Therefore, we can't really take the advantage of
multi-cores here. One way to prevent this is to align the data with cache line, i.e., `alignas(64)
std::atomic<int> a;`

```c++
void do_work(std::atomic<int> &a) {
  for (int i = 0; i < 10000; ++i) {
    ++a;
  }
}

// program 1
int main() {
  std::atomic<int> a; a = 0;
  std::atomic<int> b; b = 0;

  std::thread t1([&]() { do_work(a); });
  std::thread t2([&]() { do_work(a); });

  t1.join();
  t2.join();
  return 0;
}

// program 2
int main() {
  std::atomic<int> a; a = 0;
  std::atomic<int> b; b = 0;
  std::atomic<int> c; c = 0;
  std::atomic<int> d; d = 0;

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
```
