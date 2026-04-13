#include <cassert>
#include <iostream>
#include <source_location>
#include <string_view>

class Tracer {
 public:
  explicit Tracer(
      std::source_location location = std::source_location::current())
      : location_(location) {
    std::cout << "enter: " << location_.function_name() << " ["
              << location_.file_name() << ':' << location_.line() << "]\n";
  }

  ~Tracer() {
    std::cout << "leave: " << location_.function_name() << " ["
              << location_.file_name() << ':' << location_.line() << "]\n";
  }

 private:
  std::source_location location_;
};

#ifdef NDEBUG
#define trace() ((void)0)
#else
#define trace() Tracer tracer_object(std::source_location::current())
#endif

int square(int value) {
  trace();
  return value * value;
}

int sum_of_squares(int left, int right) {
  trace();
  return square(left) + square(right);
}

int main() {
  trace();

  const int result = sum_of_squares(3, 4);
  assert(result == 25);

  std::cout << "result: " << result << '\n';
  return 0;
}
