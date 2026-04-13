#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <new>
#include <utility>
#include <vector>

class Allocator {
 public:
  virtual ~Allocator() = default;
  virtual void* allocate(std::size_t size,
                         std::size_t alignment = alignof(std::max_align_t)) = 0;
  virtual void deallocate(void* ptr) = 0;

 protected:
  template <typename T>
  static T* get(void* ptr) {
    return static_cast<T*>(ptr);
  }
};

class LinearAllocator final : public Allocator {
 public:
  explicit LinearAllocator(std::size_t size) : size_(size) {
    begin_ = ::operator new(size_, std::align_val_t(kAlignment));
  }

  ~LinearAllocator() override {
    ::operator delete(begin_, size_, std::align_val_t(kAlignment));
  }

  void* allocate(std::size_t size,
                 std::size_t alignment = alignof(std::max_align_t)) override {
    void* current = get<std::byte>(begin_) + offset_;
    std::size_t free = size_ - offset_;
    if (current = std::align(alignment, size, current, free); current) {
      offset_ = size_ - free + size;
      return current;
    }
    return nullptr;
  }

  void deallocate(void*) override {}

 private:
  static constexpr std::size_t kAlignment = alignof(std::max_align_t);
  std::size_t size_ = 0;
  std::size_t offset_ = 0;
  void* begin_ = nullptr;
};

class StackAllocator final : public Allocator {
 public:
  explicit StackAllocator(std::size_t size) : size_(size) {
    begin_ = ::operator new(size_, std::align_val_t(kAlignment));
  }

  ~StackAllocator() override {
    ::operator delete(begin_, size_, std::align_val_t(kAlignment));
  }

  void* allocate(std::size_t size,
                 std::size_t alignment = alignof(std::max_align_t)) override {
    void* current = get<std::byte>(begin_) + offset_ + sizeof(Header);
    std::size_t free = size_ - offset_ - sizeof(Header);
    if (current = std::align(alignment, size, current, free); current) {
      auto* header = get<Header>(get<std::byte>(current) - sizeof(Header));
      header->padding =
          static_cast<std::uint8_t>(get<std::byte>(current) -
                                    (get<std::byte>(begin_) + offset_));
      offset_ = static_cast<std::size_t>(get<std::byte>(current) -
                                         get<std::byte>(begin_)) +
                size;
      return current;
    }
    return nullptr;
  }

  void deallocate(void* ptr) override {
    auto* header = get<Header>(get<std::byte>(ptr) - sizeof(Header));
    offset_ = static_cast<std::size_t>(get<std::byte>(ptr) -
                                       get<std::byte>(begin_)) -
              header->padding;
  }

 private:
  struct Header {
    std::uint8_t padding = 0;
  };

  static constexpr std::size_t kAlignment = alignof(std::max_align_t);
  std::size_t size_ = 0;
  std::size_t offset_ = 0;
  void* begin_ = nullptr;
};

class PoolAllocator final : public Allocator {
 public:
  PoolAllocator(std::size_t size, std::size_t step) : size_(size), step_(step) {
    assert(size_ % step_ == 0 && step_ >= sizeof(Node));
    begin_ = ::operator new(size_, std::align_val_t(kAlignment));
    auto* bytes = get<std::byte>(begin_);
    head_ = get<Node>(bytes);
    Node* current = head_;
    for (std::size_t offset = step_; offset < size_; offset += step_) {
      current->next = get<Node>(bytes + offset);
      current = current->next;
    }
    current->next = nullptr;
  }

  ~PoolAllocator() override {
    ::operator delete(begin_, size_, std::align_val_t(kAlignment));
  }

  void* allocate(std::size_t size,
                 std::size_t alignment = alignof(std::max_align_t)) override {
    (void)alignment;
    if (size > step_ || !head_) {
      return nullptr;
    }
    Node* node = head_;
    head_ = head_->next;
    return node;
  }

  void deallocate(void* ptr) override {
    auto* node = get<Node>(ptr);
    node->next = head_;
    head_ = node;
  }

 private:
  struct Node {
    Node* next = nullptr;
  };

  static constexpr std::size_t kAlignment = alignof(std::max_align_t);
  std::size_t size_ = 0;
  std::size_t step_ = 0;
  void* begin_ = nullptr;
  Node* head_ = nullptr;
};

class FreeListAllocator final : public Allocator {
 public:
  explicit FreeListAllocator(std::size_t size) : size_(size) {
    begin_ = ::operator new(size_, std::align_val_t(kAlignment));
    head_ = get<Node>(begin_);
    head_->size = size_ - sizeof(Header);
    head_->next = nullptr;
  }

  ~FreeListAllocator() override {
    ::operator delete(begin_, size_, std::align_val_t(kAlignment));
  }

  void* allocate(std::size_t size,
                 std::size_t alignment = alignof(std::max_align_t)) override {
    void* end = get<std::byte>(begin_) + sizeof(Header) + size;
    void* next = end;
    std::size_t free = 2 * alignof(Header);

    next = std::align(alignment, sizeof(Header), next, free);
    if (!next) {
      return nullptr;
    }

    const std::size_t padding =
        static_cast<std::size_t>(get<std::byte>(next) - get<std::byte>(end));

    Node* current = head_;
    Node* previous = nullptr;
    while (current && size + padding > current->size) {
      previous = current;
      current = current->next;
    }
    if (!current) {
      return nullptr;
    }

    std::size_t actual_padding = padding;
    if (current->size >= size + actual_padding + sizeof(Node) + 1) {
      const std::size_t step = sizeof(Header) + size + actual_padding;
      auto* node = get<Node>(get<std::byte>(current) + step);
      node->size = current->size - step;
      node->next = current->next;
      current->next = node;
    } else {
      actual_padding += current->size - size - actual_padding;
    }

    if (!previous) {
      head_ = current->next;
    } else {
      previous->next = current->next;
    }

    auto* header = get<Header>(current);
    header->size = size + actual_padding;
    return get<std::byte>(current) + sizeof(Header);
  }

  void deallocate(void* ptr) override {
    auto* node = get<Node>(get<std::byte>(ptr) - sizeof(Header));

    Node* previous = nullptr;
    Node* current = head_;
    while (current && current < node) {
      previous = current;
      current = current->next;
    }

    node->next = current;
    if (!previous) {
      head_ = node;
    } else {
      previous->next = node;
    }

    if (node->next &&
        get<std::byte>(node) + sizeof(Header) + node->size ==
            get<std::byte>(node->next)) {
      node->size += sizeof(Header) + node->next->size;
      node->next = node->next->next;
    }

    if (previous &&
        get<std::byte>(previous) + sizeof(Header) + previous->size ==
            get<std::byte>(node)) {
      previous->size += sizeof(Header) + node->size;
      previous->next = node->next;
    }
  }

 private:
  struct Node {
    std::size_t size = 0;
    Node* next = nullptr;
  };

  struct alignas(std::max_align_t) Header {
    std::size_t size = 0;
  };

  static constexpr std::size_t kAlignment = alignof(std::max_align_t);
  std::size_t size_ = 0;
  void* begin_ = nullptr;
  Node* head_ = nullptr;
};

int main() {
  std::vector<std::unique_ptr<Allocator>> allocators;
  allocators.push_back(std::make_unique<LinearAllocator>(1024));
  allocators.push_back(std::make_unique<StackAllocator>(1024));
  allocators.push_back(std::make_unique<PoolAllocator>(1024, 32));
  allocators.push_back(std::make_unique<FreeListAllocator>(1024));

  for (auto& allocator : allocators) {
    void* first = allocator->allocate(16, alignof(int));
    assert(first != nullptr && "allocator returned nullptr on first allocation");

    void* second = allocator->allocate(16, alignof(int));
    if (second != nullptr) {
      allocator->deallocate(second);
    }
    allocator->deallocate(first);
  }

  std::cout << "All allocators worked\n";
  return 0;
}
