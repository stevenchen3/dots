#include <iostream>

int main() {
  constexpr int m = 128;
  constexpr int n = 1048576;
  auto A = new int[m][n]; // 128M

  // fill with some data
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      A[i][j] = i % 2;
    }
  }

  // col-oriented access
  for (int j = 0; j < n; ++j) {
    for (int i = 0; i < m; ++i) {
      A[i][j] += j;
    }
  }

  return 0;
}
