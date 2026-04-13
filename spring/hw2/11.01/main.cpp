#include <cassert>
#include <iostream>

class Wrapper {
 public:
  using function_t = void (*)();

  explicit Wrapper(function_t function = nullptr) : function_(function) {}

  operator function_t() const { return function_; }

 private:
  function_t function_ = nullptr;
};

void test() { std::cout << "test\n"; }

Wrapper make_wrapper() { return Wrapper(&test); }

int main() {
  Wrapper function = make_wrapper();
  (*function)();

  Wrapper::function_t raw = function;
  assert(raw == &test);

  return 0;
}
