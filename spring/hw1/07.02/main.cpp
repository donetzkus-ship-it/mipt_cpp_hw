#include <concepts>
#include <exception>
#include <iostream>
#include <numeric>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

template <typename T>
concept Integral = std::integral<T>;

class Exception final : public std::exception {
 public:
  explicit Exception(std::string message) : message_(std::move(message)) {}

  [[nodiscard]] const char* what() const noexcept override {
    return message_.c_str();
  }

 private:
  std::string message_;
};

template <typename Derived>
struct addable {
  friend Derived operator+(Derived lhs, const Derived& rhs) {
    lhs += rhs;
    return lhs;
  }
};

template <typename Derived>
struct subtractable {
  friend Derived operator-(Derived lhs, const Derived& rhs) {
    lhs -= rhs;
    return lhs;
  }
};

template <typename Derived>
struct multipliable {
  friend Derived operator*(Derived lhs, const Derived& rhs) {
    lhs *= rhs;
    return lhs;
  }
};

template <typename Derived>
struct dividable {
  friend Derived operator/(Derived lhs, const Derived& rhs) {
    lhs /= rhs;
    return lhs;
  }
};

template <typename Derived>
struct incrementable {
  friend Derived& operator++(Derived& value) {
    value += Derived(1);
    return value;
  }

  friend Derived operator++(Derived& value, int) {
    Derived copy = value;
    ++value;
    return copy;
  }
};

template <typename Derived>
struct decrementable {
  friend Derived& operator--(Derived& value) {
    value -= Derived(1);
    return value;
  }

  friend Derived operator--(Derived& value, int) {
    Derived copy = value;
    --value;
    return copy;
  }
};

template <Integral T>
class Rational final : public addable<Rational<T>>,
                       public subtractable<Rational<T>>,
                       public multipliable<Rational<T>>,
                       public dividable<Rational<T>>,
                       public incrementable<Rational<T>>,
                       public decrementable<Rational<T>> {
 public:
  Rational(T num = 0, T den = 1) : num_(num), den_(den) {
    if (den_ == 0) {
      throw Exception("Rational: zero denominator");
    }
    Reduce();
  }

  Rational& operator+=(const Rational& other) {
    const T lcm = std::lcm(den_, other.den_);
    num_ = num_ * (lcm / den_) + other.num_ * (lcm / other.den_);
    den_ = lcm;
    Reduce();
    return *this;
  }

  Rational& operator-=(const Rational& other) {
    const T lcm = std::lcm(den_, other.den_);
    num_ = num_ * (lcm / den_) - other.num_ * (lcm / other.den_);
    den_ = lcm;
    Reduce();
    return *this;
  }

  Rational& operator*=(const Rational& other) {
    num_ *= other.num_;
    den_ *= other.den_;
    Reduce();
    return *this;
  }

  Rational& operator/=(const Rational& other) {
    if (other.num_ == 0) {
      throw Exception("Rational: division by zero");
    }
    num_ *= other.den_;
    den_ *= other.num_;
    if (den_ == 0) {
      throw Exception("Rational: zero denominator");
    }
    Reduce();
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& out, const Rational& value) {
    return out << value.num_ << '/' << value.den_;
  }

 private:
  void Reduce() {
    if (den_ < 0) {
      num_ = -num_;
      den_ = -den_;
    }
    const T gcd = std::gcd(num_, den_);
    num_ /= gcd;
    den_ /= gcd;
  }

  T num_;
  T den_;
};

void DemonstrateCustomException() {
  try {
    [[maybe_unused]] Rational<int> invalid(1, 0);
  } catch (const Exception& exception) {
    std::cerr << "Exception: " << exception.what()
              << " (reason: denominator cannot be zero)\n";
  }
}

void DemonstrateStandardExceptions() {
  try {
    throw std::bad_alloc();
  } catch (const std::bad_alloc& exception) {
    std::cerr << "std::bad_alloc: " << exception.what()
              << " (reason: allocation failure)\n";
  }

  try {
    std::variant<int, double> variant = 42;
    [[maybe_unused]] double value = std::get<double>(variant);
  } catch (const std::bad_variant_access& exception) {
    std::cerr << "std::bad_variant_access: " << exception.what()
              << " (reason: wrong active type in std::variant)\n";
  }

  try {
    std::optional<int> optional;
    [[maybe_unused]] int value = optional.value();
  } catch (const std::bad_optional_access& exception) {
    std::cerr << "std::bad_optional_access: " << exception.what()
              << " (reason: value() on empty std::optional)\n";
  }

  try {
    std::vector<int> values;
    values.reserve(values.max_size() + 1);
  } catch (const std::length_error& exception) {
    std::cerr << "std::length_error: " << exception.what()
              << " (reason: reserve size exceeds vector::max_size())\n";
  }

  try {
    std::vector<int> values = {1, 2, 3};
    [[maybe_unused]] int value = values.at(10);
  } catch (const std::out_of_range& exception) {
    std::cerr << "std::out_of_range: " << exception.what()
              << " (reason: index is outside vector bounds)\n";
  }
}

int main() {
  try {
    Rational<int> x(1, 2);
    Rational<int> y(3, 4);
    std::cerr << "Rational demo: " << (x + y) << '\n';

    DemonstrateCustomException();
    DemonstrateStandardExceptions();
  } catch (const std::exception& exception) {
    std::cerr << "Unhandled std::exception: " << exception.what() << '\n';
    return 1;
  } catch (...) {
    std::cerr << "Unhandled unknown exception\n";
    return 1;
  }

  return 0;
}
