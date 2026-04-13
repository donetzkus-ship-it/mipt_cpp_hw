#include <cassert>
#include <cstddef>
#include <iostream>
#include <new>

template <typename D>
class Entity {
 public:
  static void* operator new(std::size_t size) {
    std::cout << "Entity::operator new\n";
    return ::operator new(size);
  }

  static void operator delete(void* ptr) noexcept {
    std::cout << "Entity::operator delete\n";
    ::operator delete(ptr);
  }

  static void* operator new[](std::size_t size) {
    std::cout << "Entity::operator new[]\n";
    return ::operator new[](size);
  }

  static void operator delete[](void* ptr) noexcept {
    std::cout << "Entity::operator delete[]\n";
    ::operator delete[](ptr);
  }

  static void* operator new(std::size_t size, const std::nothrow_t& tag) noexcept {
    std::cout << "Entity::operator new (nothrow)\n";
    return ::operator new(size, tag);
  }

  static void operator delete(void* ptr, const std::nothrow_t&) noexcept {
    std::cout << "Entity::operator delete (nothrow)\n";
    ::operator delete(ptr);
  }

  static void* operator new[](std::size_t size,
                              const std::nothrow_t& tag) noexcept {
    std::cout << "Entity::operator new[] (nothrow)\n";
    return ::operator new[](size, tag);
  }

  static void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
    std::cout << "Entity::operator delete[] (nothrow)\n";
    ::operator delete[](ptr);
  }

 protected:
  Entity() = default;
};

class Client : private Entity<Client> {
 public:
  Client() { std::cout << "Client::Client\n"; }
  ~Client() { std::cout << "Client::~Client\n"; }

  using Entity<Client>::operator delete;
  using Entity<Client>::operator delete[];
  using Entity<Client>::operator new;
  using Entity<Client>::operator new[];
};

int main() {
  Client* one = new Client;
  delete one;

  Client* many = new Client[2];
  delete[] many;

  Client* maybe_one = new (std::nothrow) Client;
  assert(maybe_one != nullptr);
  delete maybe_one;

  Client* maybe_many = new (std::nothrow) Client[2];
  assert(maybe_many != nullptr);
  delete[] maybe_many;

  return 0;
}
