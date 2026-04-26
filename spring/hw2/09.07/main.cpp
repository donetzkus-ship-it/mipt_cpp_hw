// Оригинальный Pimpl: Implementation хранится в куче по указателю.
// Накладные расходы: отдельная аллокация на каждый Entity (вызов malloc,
// возможные блокировки в аллокаторе, фрагментация кучи), косвенность при
// каждом доступе (cache miss из-за того, что Implementation лежит вдали от Entity),
// копирование требует ещё одной пары new/delete, ctor может бросить
// bad_alloc. Move дешёвый (swap указателей), но это единственная операция
// без накладных расходов.
//
// Здесь Implementation живёт внутри Entity в выровненном std::array<std::byte, 16>:
// нет аллокаций, Implementation лежит рядом с Entity (cache-friendly), ctor noexcept.
// Платим размером Entity (фиксирован на этапе компиляции через
// static_assert) и тем, что при изменении layout Implementation придётся
// пересобирать клиентов Entity — теряется главное свойство Pimpl, изоляция
// зависимостей по компиляции.

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
      std::destroy_at(get());
      new (storage_.data()) Implementation(std::move(*other.get()));
    }
    return *this;
  }

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
  first.get()->test();

  Entity second(std::move(first));
  assert(second.get()->sum() == 3);
  second.get()->test();

  Entity third;
  third = std::move(second);
  assert(third.get()->sum() == 3);
  third.get()->test();

  return 0;
}
