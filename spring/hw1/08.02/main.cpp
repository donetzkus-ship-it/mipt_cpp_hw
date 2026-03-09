#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class Integer {
 public:
  Integer() : digits_("0"), sign_(0) {}

  Integer(long long value) {
    if (value == 0) {
      digits_ = "0";
      sign_ = 0;
      return;
    }

    sign_ = (value < 0 ? -1 : 1);
    const unsigned long long abs_value =
        (value < 0) ? (0ULL - static_cast<unsigned long long>(value))
                    : static_cast<unsigned long long>(value);
    unsigned long long current = abs_value;
    digits_.clear();
    while (current > 0) {
      digits_.push_back(static_cast<char>('0' + (current % 10)));
      current /= 10;
    }
    std::reverse(digits_.begin(), digits_.end());
  }

  explicit Integer(std::string value) {
    if (value.empty()) {
      throw std::invalid_argument("empty integer string");
    }

    std::size_t start = 0;
    sign_ = 1;
    if (value[0] == '-') {
      sign_ = -1;
      start = 1;
    } else if (value[0] == '+') {
      start = 1;
    }

    if (start >= value.size()) {
      throw std::invalid_argument("invalid integer string");
    }

    for (std::size_t i = start; i < value.size(); ++i) {
      if (!std::isdigit(static_cast<unsigned char>(value[i]))) {
        throw std::invalid_argument("invalid integer string");
      }
    }

    digits_ = normalize_digits(value.substr(start));
    if (digits_ == "0") {
      sign_ = 0;
    }
  }

  Integer& operator+=(const Integer& other) {
    if (sign_ == 0) {
      *this = other;
      return *this;
    }
    if (other.sign_ == 0) {
      return *this;
    }

    if (sign_ == other.sign_) {
      digits_ = add_abs(digits_, other.digits_);
      return *this;
    }

    const int cmp = compare_abs(digits_, other.digits_);
    if (cmp == 0) {
      digits_ = "0";
      sign_ = 0;
      return *this;
    }

    if (cmp > 0) {
      digits_ = sub_abs(digits_, other.digits_);
      return *this;
    }

    digits_ = sub_abs(other.digits_, digits_);
    sign_ = other.sign_;
    return *this;
  }

  Integer& operator-=(const Integer& other) {
    Integer temp = other;
    temp.sign_ = -temp.sign_;
    return *this += temp;
  }

  Integer& operator*=(const Integer& other) {
    if (sign_ == 0 || other.sign_ == 0) {
      digits_ = "0";
      sign_ = 0;
      return *this;
    }

    digits_ = mul_abs(digits_, other.digits_);
    sign_ *= other.sign_;
    normalize();
    return *this;
  }

  Integer& operator/=(const Integer& other) {
    if (other.sign_ == 0) {
      throw std::invalid_argument("division by zero");
    }
    if (sign_ == 0) {
      return *this;
    }

    auto [quotient_abs, remainder_abs] = div_mod_abs(digits_, other.digits_);
    (void)remainder_abs;

    digits_ = quotient_abs;
    sign_ = (digits_ == "0" ? 0 : sign_ * other.sign_);
    return *this;
  }

  Integer& operator%=(const Integer& other) {
    if (other.sign_ == 0) {
      throw std::invalid_argument("modulo by zero");
    }
    if (sign_ == 0) {
      return *this;
    }

    auto [quotient_abs, remainder_abs] = div_mod_abs(digits_, other.digits_);
    (void)quotient_abs;

    digits_ = remainder_abs;
    sign_ = (digits_ == "0" ? 0 : sign_);
    return *this;
  }

  [[nodiscard]] std::string str() const {
    if (sign_ < 0) {
      return "-" + digits_;
    }
    return "+" + digits_;
  }

  int sign() const { return sign_; }

  Integer abs() const {
    Integer result = *this;
    if (result.sign_ < 0) {
      result.sign_ = 1;
    }
    return result;
  }

  friend Integer operator+(Integer lhs, const Integer& rhs) {
    return lhs += rhs;
  }
  friend Integer operator-(Integer lhs, const Integer& rhs) {
    return lhs -= rhs;
  }
  friend Integer operator*(Integer lhs, const Integer& rhs) {
    return lhs *= rhs;
  }
  friend Integer operator/(Integer lhs, const Integer& rhs) {
    return lhs /= rhs;
  }
  friend Integer operator%(Integer lhs, const Integer& rhs) {
    return lhs %= rhs;
  }

  friend bool operator==(const Integer& lhs, const Integer& rhs) {
    return lhs.sign_ == rhs.sign_ && lhs.digits_ == rhs.digits_;
  }

  friend bool operator==(const Integer& lhs, const std::string& rhs) {
    return lhs.str() == rhs;
  }

  friend bool operator==(const std::string& lhs, const Integer& rhs) {
    return lhs == rhs.str();
  }

  friend bool operator!=(const Integer& lhs, const std::string& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator!=(const std::string& lhs, const Integer& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<(const Integer& lhs, const Integer& rhs) {
    if (lhs.sign_ != rhs.sign_) {
      return lhs.sign_ < rhs.sign_;
    }
    if (lhs.sign_ == 0) {
      return false;
    }
    const int cmp = compare_abs(lhs.digits_, rhs.digits_);
    return (lhs.sign_ > 0 ? (cmp < 0) : (cmp > 0));
  }

  friend std::ostream& operator<<(std::ostream& out, const Integer& value) {
    if (value.sign_ < 0) {
      out << '-';
    }
    return out << value.digits_;
  }

 private:
  static std::string normalize_digits(const std::string& input) {
    std::size_t pos = 0;
    while (pos + 1 < input.size() && input[pos] == '0') {
      ++pos;
    }
    return input.substr(pos);
  }

  void normalize() {
    digits_ = normalize_digits(digits_);
    if (digits_ == "0") {
      sign_ = 0;
    }
  }

  static int compare_abs(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
      return (a.size() < b.size() ? -1 : 1);
    }
    if (a == b) {
      return 0;
    }
    return (a < b ? -1 : 1);
  }

  static std::string add_abs(const std::string& a, const std::string& b) {
    std::string result;
    int carry = 0;
    int i = static_cast<int>(a.size()) - 1;
    int j = static_cast<int>(b.size()) - 1;

    while (i >= 0 || j >= 0 || carry > 0) {
      const int da = (i >= 0 ? a[static_cast<std::size_t>(i)] - '0' : 0);
      const int db = (j >= 0 ? b[static_cast<std::size_t>(j)] - '0' : 0);
      const int sum = da + db + carry;
      result.push_back(static_cast<char>('0' + (sum % 10)));
      carry = sum / 10;
      --i;
      --j;
    }

    std::reverse(result.begin(), result.end());
    return result;
  }

  static std::string sub_abs(const std::string& a, const std::string& b) {
    std::string result;
    int borrow = 0;
    int i = static_cast<int>(a.size()) - 1;
    int j = static_cast<int>(b.size()) - 1;

    while (i >= 0) {
      int da = a[static_cast<std::size_t>(i)] - '0' - borrow;
      const int db = (j >= 0 ? b[static_cast<std::size_t>(j)] - '0' : 0);
      if (da < db) {
        da += 10;
        borrow = 1;
      } else {
        borrow = 0;
      }

      result.push_back(static_cast<char>('0' + (da - db)));
      --i;
      --j;
    }

    std::reverse(result.begin(), result.end());
    return normalize_digits(result);
  }

  static std::string mul_abs(const std::string& a, const std::string& b) {
    std::vector<int> digits(a.size() + b.size(), 0);

    for (int i = static_cast<int>(a.size()) - 1; i >= 0; --i) {
      for (int j = static_cast<int>(b.size()) - 1; j >= 0; --j) {
        const int ia = a[static_cast<std::size_t>(i)] - '0';
        const int ib = b[static_cast<std::size_t>(j)] - '0';
        digits[static_cast<std::size_t>(i + j + 1)] += ia * ib;
      }
    }

    for (int k = static_cast<int>(digits.size()) - 1; k > 0; --k) {
      digits[static_cast<std::size_t>(k - 1)] +=
          digits[static_cast<std::size_t>(k)] / 10;
      digits[static_cast<std::size_t>(k)] %= 10;
    }

    std::string result;
    result.reserve(digits.size());
    for (int digit : digits) {
      result.push_back(static_cast<char>('0' + digit));
    }
    return normalize_digits(result);
  }

  static std::pair<std::string, std::string> div_mod_abs(const std::string& a,
                                                         const std::string& b) {
    if (b == "0") {
      throw std::invalid_argument("division by zero");
    }

    if (compare_abs(a, b) < 0) {
      return {"0", a};
    }

    std::string quotient;
    std::string remainder = "0";

    for (char digit : a) {
      remainder = mul_abs(remainder, "10");
      remainder = add_abs(remainder, std::string(1, digit));

      int q_digit = 0;
      while (compare_abs(remainder, b) >= 0) {
        remainder = sub_abs(remainder, b);
        ++q_digit;
      }
      quotient.push_back(static_cast<char>('0' + q_digit));
    }

    quotient = normalize_digits(quotient);
    remainder = normalize_digits(remainder);
    return {quotient, remainder};
  }

  std::string digits_;
  int sign_ = 0;
};

Integer pow(Integer base, unsigned int exponent) {
  Integer result(1);

  while (exponent > 0) {
    if ((exponent & 1U) != 0U) {
      result *= base;
    }
    exponent >>= 1U;
    if (exponent > 0) {
      base *= base;
    }
  }

  return result;
}

int main() {
  using namespace std::literals;

  Integer x(std::string(32, '1'));
  Integer y(std::string(32, '2'));

  assert((x += y) == "+33333333333333333333333333333333"s);
  assert((x -= y) == "+11111111111111111111111111111111"s);
  assert((x *= y) ==
         "+246913580246913580246913580246908641975308641975308641975308642"s);
  assert((x /= y) == "+11111111111111111111111111111111"s);
  assert((x % Integer("9")) == "+5"s);

  assert(pow(Integer(12), 5) == "+248832"s);
  assert(Integer("-7").sign() == -1);
  assert(Integer("0").sign() == 0);
  assert(Integer("-123").abs() == "+123"s);

  std::cout << "All tests passed\n";

  return 0;
}
