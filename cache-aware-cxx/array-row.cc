#include <iostream>

using namespace std;

int main() {
  constexpr int m = 128;
  constexpr int n = 1048576;

  // fill with some data
  auto A = new int[m][n]; // 128M
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      A[i][j] = i % 2;
    }
  }

  // row-oriented access
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      A[i][j] += j;
    }
  }

  return 0;
}
