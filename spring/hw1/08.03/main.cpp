#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

static_assert(sizeof(int) == 4, "int must be 4 bytes");
static_assert(sizeof(float) == 4, "float must be 4 bytes");

int floor_log2_int(int value) {
  if (value <= 0) {
    return std::numeric_limits<int>::min();
  }

  unsigned int bits = static_cast<unsigned int>(value);
  int exponent = -1;

  while (bits != 0U) {
    bits >>= 1U;
    ++exponent;
  }

  return exponent;
}

int msb_index(unsigned int bits) {
  if (bits == 0U) {
    return -1;
  }

  int index = -1;
  while (bits != 0U) {
    bits >>= 1U;
    ++index;
  }
  return index;
}

int floor_log2_float(float value) {
  if (value <= 0.0f) {
    return std::numeric_limits<int>::min();
  }

  union {
    float as_float;
    unsigned int as_uint;
  } representation{};

  representation.as_float = value;

  constexpr unsigned int kExponentMask = 0x7F800000U;
  constexpr unsigned int kMantissaMask = 0x007FFFFFU;
  constexpr int kExponentBias = 127;

  const unsigned int exponent_bits =
      (representation.as_uint & kExponentMask) >> 23U;
  const unsigned int mantissa_bits = representation.as_uint & kMantissaMask;

  if (exponent_bits == 0xFFU) {
    return std::numeric_limits<int>::max();
  }

  if (exponent_bits == 0U) {
    if (mantissa_bits == 0U) {
      return std::numeric_limits<int>::min();
    }

    const int highest_mantissa_bit = msb_index(mantissa_bits);
    return highest_mantissa_bit - 149;
  }

  return static_cast<int>(exponent_bits) - kExponentBias;
}

void run_tests() {
  assert(floor_log2_int(1) == 0);
  assert(floor_log2_int(2) == 1);
  assert(floor_log2_int(3) == 1);
  assert(floor_log2_int(1024) == 10);

  assert(floor_log2_float(1.0f) == 0);
  assert(floor_log2_float(2.0f) == 1);
  assert(floor_log2_float(3.5f) == 1);
  assert(floor_log2_float(0.75f) == -1);

  const float denormal = std::numeric_limits<float>::denorm_min();
  assert(floor_log2_float(denormal) == -149);

  const float infinity = std::numeric_limits<float>::infinity();
  assert(floor_log2_float(infinity) == std::numeric_limits<int>::max());

  const float nan_value = std::numeric_limits<float>::quiet_NaN();
  assert(floor_log2_float(nan_value) == std::numeric_limits<int>::max());
}

int main() {
  run_tests();

  int integer_value = 0;
  float float_value = 0.0f;

  if (!(std::cin >> integer_value >> float_value)) {
    return 1;
  }

  std::cout << floor_log2_int(integer_value) << '\n';
  std::cout << floor_log2_float(float_value) << '\n';

  return 0;
}
