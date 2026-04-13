#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <new>
#include <utility>

class Entity {
 private:
  static constexpr std::size_t kStorageSize = 16;
  class Implementation;

 public:
  Entity() {
    static_assert(sizeof(Implementation) <= kStorageSize,
                  "Implementation does not fit in storage");
    static_assert(alignof(Implementation) <= alignof(std::max_align_t),
                  "Implementation alignment is too large");

    new (storage_.data()) Implementation(1, 2);
  }

  Entity(Entity&& other) noexcept {
    new (storage_.data()) Implementation(std::move(*other.get()));
  }

  ~Entity() { std::destroy_at(get()); }

  Entity& operator=(Entity&& other) noexcept {
    if (this != &other) {
      *get() = std::move(*other.get());
    }
    return *this;
  }

  void test() const { get()->test(); }

  Implementation* get() {
    auto raw = std::bit_cast<Implementation*>(storage_.data());
    return std::launder(raw);
  }

  const Implementation* get() const {
    auto raw = std::bit_cast<const Implementation*>(storage_.data());
    return std::launder(raw);
  }

 private:
  class Implementation {
   public:
    Implementation(int x, int y) : x_(x), y_(y) {}

    void test() const { std::cout << "Implementation::test : " << x_ + y_ << '\n'; }

    int sum() const { return x_ + y_; }

   private:
    int x_ = 0;
    int y_ = 0;
  };

  alignas(std::max_align_t) std::array<std::byte, kStorageSize> storage_{};
};

int main() {
  Entity first;
  assert(first.get()->sum() == 3);
  first.test();

  Entity second(std::move(first));
  assert(second.get()->sum() == 3);
  second.test();

  Entity third;
  third = std::move(second);
  assert(third.get()->sum() == 3);
  third.test();

  return 0;
}
