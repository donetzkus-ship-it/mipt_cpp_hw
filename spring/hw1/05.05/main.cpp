#include <cassert>
#include <compare>
#include <concepts>
#include <iostream>
#include <numeric>
#include <sstream>

template <typename T>
concept Integral = std::integral<T>;

template <typename Derived>
struct addable {
  friend Derived operator+(Derived lhs, Derived const& rhs) {
    lhs += rhs;
    return lhs;
  }
};

template <typename Derived>
struct subtractable {
  friend Derived operator-(Derived lhs, Derived const& rhs) {
    lhs -= rhs;
    return lhs;
  }
};

template <typename Derived>
struct multipliable {
  friend Derived operator*(Derived lhs, Derived const& rhs) {
    lhs *= rhs;
    return lhs;
  }
};

template <typename Derived>
struct dividable {
  friend Derived operator/(Derived lhs, Derived const& rhs) {
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
  Rational(T num = 0, T den = 1) : m_num(num), m_den(den) {
    assert(m_den != 0);
    reduce();
  }

  explicit operator double() const {
    return static_cast<double>(m_num) / static_cast<double>(m_den);
  }

  Rational& operator+=(Rational const& other) {
    T const lcm = std::lcm(m_den, other.m_den);
    m_num = m_num * (lcm / m_den) + other.m_num * (lcm / other.m_den);
    m_den = lcm;
    reduce();
    return *this;
  }

  Rational& operator-=(Rational const& other) {
    T const lcm = std::lcm(m_den, other.m_den);
    m_num = m_num * (lcm / m_den) - other.m_num * (lcm / other.m_den);
    m_den = lcm;
    reduce();
    return *this;
  }

  Rational& operator*=(Rational const& other) {
    m_num *= other.m_num;
    m_den *= other.m_den;
    reduce();
    return *this;
  }

  Rational& operator/=(Rational const& other) {
    assert(other.m_num != 0);
    m_num *= other.m_den;
    m_den *= other.m_num;
    reduce();
    return *this;
  }

  friend std::strong_ordering operator<=>(Rational const& lhs,
                                          Rational const& rhs) {
    return lhs.m_num * rhs.m_den <=> rhs.m_num * lhs.m_den;
  }

  friend bool operator==(Rational const& lhs, Rational const& rhs) {
    return lhs.m_num == rhs.m_num && lhs.m_den == rhs.m_den;
  }

  friend std::istream& operator>>(std::istream& stream, Rational& rational) {
    T num = 0;
    T den = 1;
    char slash = '\0';

    if (!(stream >> num >> slash >> den) || slash != '/' || den == 0) {
      stream.setstate(std::ios::failbit);
      return stream;
    }

    rational = Rational(num, den);
    return stream;
  }

  friend std::ostream& operator<<(std::ostream& stream,
                                  Rational const& rational) {
    return stream << rational.m_num << '/' << rational.m_den;
  }

 private:
  void reduce() {
    if (m_den < 0) {
      m_num = -m_num;
      m_den = -m_den;
    }

    T const gcd = std::gcd(m_num, m_den);
    m_num /= gcd;
    m_den /= gcd;
  }

  T m_num;
  T m_den;
};

int main() {
  using Rat = Rational<int>;

  Rat x(1, 2);
  Rat y(3, 4);

  assert((x + y) == Rat(5, 4));
  assert((x - y) == Rat(-1, 4));
  assert((x * y) == Rat(3, 8));
  assert((x / y) == Rat(2, 3));

  assert((++x) == Rat(3, 2));
  assert((x++) == Rat(3, 2));
  assert(x == Rat(5, 2));
  assert((--x) == Rat(3, 2));
  assert((x--) == Rat(3, 2));
  assert(x == Rat(1, 2));

  assert((x + Rat(1)) == Rat(3, 2));
  assert((Rat(1) + y) == Rat(7, 4));
  assert((y - Rat(1)) == Rat(-1, 4));

  assert(Rat(1, 2) < Rat(2, 3));
  assert(Rat(2, 3) > Rat(1, 2));
  assert(Rat(2, 4) == Rat(1, 2));

  std::stringstream input("10/20");
  std::stringstream output;
  Rat z;
  input >> z;
  output << z;
  assert(output.str() == "1/2");

  std::cout << "All tests passed\n";
  std::cout << "Example: " << Rat(5, 6) + Rat(1, 3) << '\n';
}
