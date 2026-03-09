#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

using Roots = std::variant<double, std::pair<double, double>, std::monostate>;
using SolveResult = std::optional<Roots>;

constexpr double kEpsilon = 1e-9;

bool equal(double lhs, double rhs) { return std::abs(lhs - rhs) <= kEpsilon; }

SolveResult solve(double a, double b, double c) {
  if (std::abs(a) <= kEpsilon) {
    if (std::abs(b) <= kEpsilon) {
      if (std::abs(c) <= kEpsilon) {
        return Roots(std::monostate{});
      }
      return std::nullopt;
    }

    return Roots(-c / b);
  }

  double const discriminant = b * b - 4.0 * a * c;

  if (discriminant < -kEpsilon) {
    return std::nullopt;
  }

  if (std::abs(discriminant) <= kEpsilon) {
    return Roots(-b / (2.0 * a));
  }

  double const sqrt_discriminant = std::sqrt(discriminant);
  double x1 = (-b - sqrt_discriminant) / (2.0 * a);
  double x2 = (-b + sqrt_discriminant) / (2.0 * a);

  if (x1 > x2) {
    std::swap(x1, x2);
  }

  return Roots(std::pair<double, double>{x1, x2});
}

void run_tests() {
  {
    SolveResult const result = solve(1.0, 0.0, -1.0);
    assert(result.has_value());
    assert((std::holds_alternative<std::pair<double, double>>(*result)));
    auto const roots = std::get<std::pair<double, double>>(*result);
    assert(equal(roots.first, -1.0));
    assert(equal(roots.second, 1.0));
  }

  {
    SolveResult const result = solve(1.0, 2.0, 1.0);
    assert(result.has_value());
    assert(std::holds_alternative<double>(*result));
    assert(equal(std::get<double>(*result), -1.0));
  }

  {
    SolveResult const result = solve(0.0, 0.0, 0.0);
    assert(result.has_value());
    assert(std::holds_alternative<std::monostate>(*result));
  }

  {
    SolveResult const result = solve(0.0, 0.0, 1.0);
    assert(!result.has_value());
  }
}

int main() {
  run_tests();

  double a = 0.0;
  double b = 0.0;
  double c = 0.0;

  if (!(std::cin >> a >> b >> c)) {
    return 1;
  }

  SolveResult const result = solve(a, b, c);

  if (!result.has_value()) {
    std::cout << "no roots\n";
    return 0;
  }

  if (std::holds_alternative<std::monostate>(*result)) {
    std::cout << "infinite roots\n";
  } else if (std::holds_alternative<double>(*result)) {
    std::cout << std::get<double>(*result) << '\n';
  } else {
    auto const roots = std::get<std::pair<double, double>>(*result);
    std::cout << roots.first << ' ' << roots.second << '\n';
  }

  return 0;
}
