#include <cassert>
#include <iostream>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/operation.hpp>

using Matrix =
    boost::numeric::ublas::matrix<unsigned long long, boost::numeric::ublas::row_major>;

Matrix identity_matrix() {
  Matrix matrix(2, 2);
  matrix(0, 0) = 1;
  matrix(0, 1) = 0;
  matrix(1, 0) = 0;
  matrix(1, 1) = 1;
  return matrix;
}

Matrix fibonacci_matrix() {
  Matrix matrix(2, 2);
  matrix(0, 0) = 1;
  matrix(0, 1) = 1;
  matrix(1, 0) = 1;
  matrix(1, 1) = 0;
  return matrix;
}

Matrix power(Matrix base, unsigned long long exponent) {
  Matrix result = identity_matrix();

  while (exponent > 0) {
    if ((exponent & 1ULL) != 0ULL) {
      result = boost::numeric::ublas::prod(result, base);
    }
    exponent >>= 1ULL;
    if (exponent > 0) {
      base = boost::numeric::ublas::prod(base, base);
    }
  }

  return result;
}

unsigned long long fibonacci(unsigned long long n) {
  if (n == 0) {
    return 0;
  }

  const Matrix powered = power(fibonacci_matrix(), n - 1);
  return powered(0, 0);
}

int main() {
  assert(fibonacci(0) == 0);
  assert(fibonacci(1) == 1);
  assert(fibonacci(2) == 1);
  assert(fibonacci(10) == 55);
  assert(fibonacci(20) == 6765);

  unsigned long long n = 0;
  if (!(std::cin >> n)) {
    return 1;
  }

  std::cout << fibonacci(n) << '\n';
  return 0;
}
