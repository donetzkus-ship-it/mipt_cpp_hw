#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <random>
#include <utility>
#include <vector>

#include <benchmark/benchmark.h>

class Allocator {
 public:
  enum class Policy { kFirstFit, kBestFit };

  explicit Allocator(std::size_t size, Policy policy) : size_(size), policy_(policy) {
    assert(size_ >= sizeof(Node) + 1);

    begin_ = ::operator new(size_, std::align_val_t(kAlignment));
    head_ = get_node(begin_);
    head_->size = size_ - sizeof(Header);
    head_->next = nullptr;
  }

  ~Allocator() { ::operator delete(begin_, size_, std::align_val_t(kAlignment)); }

  void* allocate(std::size_t size) {
    void* end = get_byte(begin_) + sizeof(Header) + size;
    void* next = end;
    std::size_t free = 2 * alignof(Header);

    next = std::align(alignof(Header), sizeof(Header), next, free);
    if (!next) {
      return nullptr;
    }

    const auto padding =
        static_cast<std::size_t>(get_byte(next) - get_byte(end));
    auto [current, previous] = find(size + padding);
    if (!current) {
      return nullptr;
    }

    std::size_t actual_padding = padding;
    if (current->size >= size + actual_padding + sizeof(Node) + 1) {
      const std::size_t step = sizeof(Header) + size + actual_padding;
      auto* node = get_node(get_byte(current) + step);
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

    auto* header = get_header(current);
    header->size = size + actual_padding;
    return get_byte(current) + sizeof(Header);
  }

  void deallocate(void* ptr) {
    auto* node = get_node(get_byte(ptr) - sizeof(Header));
    Node* previous = nullptr;
    Node* current = head_;

    while (current) {
      if (node < current) {
        node->next = current;
        if (!previous) {
          head_ = node;
        } else {
          previous->next = node;
        }
        merge(previous, node);
        return;
      }

      previous = current;
      current = current->next;
    }

    node->next = nullptr;
    if (!previous) {
      head_ = node;
    } else {
      previous->next = node;
    }
    merge(previous, node);
  }

 private:
  struct Node {
    std::size_t size = 0;
    Node* next = nullptr;
  };

  struct alignas(std::max_align_t) Header {
    std::size_t size = 0;
  };

  std::byte* get_byte(void* ptr) const { return static_cast<std::byte*>(ptr); }
  Node* get_node(void* ptr) const { return static_cast<Node*>(ptr); }
  Header* get_header(void* ptr) const { return static_cast<Header*>(ptr); }

  std::pair<Node*, Node*> find(std::size_t size) const {
    return (policy_ == Policy::kFirstFit) ? find_first(size) : find_best(size);
  }

  std::pair<Node*, Node*> find_first(std::size_t size) const {
    Node* current = head_;
    Node* previous = nullptr;

    while (current && size > current->size) {
      previous = current;
      current = current->next;
    }

    return {current, previous};
  }

  std::pair<Node*, Node*> find_best(std::size_t size) const {
    Node* current = head_;
    Node* previous = nullptr;
    Node* best = nullptr;
    Node* best_previous = nullptr;

    while (current) {
      if (current->size >= size &&
          (!best || current->size < best->size)) {
        best = current;
        best_previous = previous;
      }

      previous = current;
      current = current->next;
    }

    return {best, best_previous};
  }

  void merge(Node* previous, Node* node) const {
    if (node->next &&
        get_byte(node) + sizeof(Header) + node->size == get_byte(node->next)) {
      node->size += sizeof(Header) + node->next->size;
      node->next = node->next->next;
    }

    if (previous &&
        get_byte(previous) + sizeof(Header) + previous->size == get_byte(node)) {
      previous->size += sizeof(Header) + node->size;
      previous->next = node->next;
    }
  }

  std::size_t size_ = 0;
  Policy policy_ = Policy::kFirstFit;
  void* begin_ = nullptr;
  Node* head_ = nullptr;

  static constexpr std::size_t kAlignment = alignof(std::max_align_t);
};

void run_allocator_benchmark(benchmark::State& state, Allocator::Policy policy) {
  constexpr std::size_t kCount = 256;
  constexpr std::size_t kPoolSize = 32 * (1ULL << 20);

  std::default_random_engine engine(42);
  std::uniform_int_distribution<std::size_t> distribution(64, 4096);
  std::array<void*, kCount> pointers{};
  std::array<std::size_t, kCount> sizes{};

  for (auto _ : state) {
    Allocator allocator(kPoolSize, policy);

    for (std::size_t i = 0; i < kCount; ++i) {
      sizes[i] = distribution(engine);
      pointers[i] = allocator.allocate(sizes[i]);
      benchmark::DoNotOptimize(pointers[i]);
    }

    for (std::size_t i = 0; i < kCount; i += 4) {
      if (pointers[i]) {
        allocator.deallocate(pointers[i]);
        pointers[i] = nullptr;
      }
    }

    for (std::size_t i = 0; i < kCount; i += 4) {
      sizes[i] = distribution(engine);
      pointers[i] = allocator.allocate(sizes[i]);
      benchmark::DoNotOptimize(pointers[i]);
    }

    for (void* ptr : pointers) {
      if (ptr) {
        allocator.deallocate(ptr);
      }
    }
  }
}

static void BM_FirstFit(benchmark::State& state) {
  run_allocator_benchmark(state, Allocator::Policy::kFirstFit);
}

static void BM_BestFit(benchmark::State& state) {
  run_allocator_benchmark(state, Allocator::Policy::kBestFit);
}

BENCHMARK(BM_FirstFit);
BENCHMARK(BM_BestFit);

BENCHMARK_MAIN();
