#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>

class Entity_v1 {
 public:
  explicit Entity_v1(int value) : value_(value) {}

  int value() const { return value_; }

 private:
  int value_;
};

class Entity_v2 {
 public:
  int value = 0;
};

void mutate_via_reinterpret_cast(Entity_v1& entity, int hacked_value) {
  auto& alias = reinterpret_cast<Entity_v2&>(entity);
  alias.value = hacked_value;
}

void mutate_via_raw_memory(Entity_v1& entity, int hacked_value) {
  unsigned char* raw = reinterpret_cast<unsigned char*>(&entity);
  std::memcpy(raw, &hacked_value, sizeof(hacked_value));
}

int main() {
  Entity_v1 first(10);
  std::cout << "before reinterpret_cast hack: " << first.value() << '\n';
  mutate_via_reinterpret_cast(first, 777);
  std::cout << "after reinterpret_cast hack:  " << first.value() << '\n';
  assert(first.value() == 777);

  Entity_v1 second(20);
  std::cout << "before raw memory hack:       " << second.value() << '\n';
  mutate_via_raw_memory(second, -42);
  std::cout << "after raw memory hack:        " << second.value() << '\n';
  assert(second.value() == -42);

  std::cout << "Done\n";
  return 0;
}
