#include <cassert>
#include <cstddef>
#include <deque>
#include <iomanip>
#include <iostream>
#include <vector>

void analyze_vector_growth() {
  std::vector<int> values;
  std::size_t previous_capacity = values.capacity();

  std::cout << "vector capacity changes\n";
  for (int i = 0; i < 256; ++i) {
    values.push_back(i);
    if (values.capacity() != previous_capacity) {
      const std::size_t new_capacity = values.capacity();
      std::cout << "size=" << values.size() << " capacity=" << new_capacity;
      if (previous_capacity != 0) {
        const double ratio =
            static_cast<double>(new_capacity) / previous_capacity;
        std::cout << " ratio=" << std::fixed << std::setprecision(2) << ratio;
      }
      std::cout << '\n';
      previous_capacity = new_capacity;
    }
  }
}

void analyze_deque_pages() {
  std::deque<int> values;
  values.push_back(0);

  std::cout << "deque address jumps\n";

  std::byte* previous =
      reinterpret_cast<std::byte*>(std::addressof(values.front()));
  std::size_t current_page_size = sizeof(int);

  for (int i = 1; i < 512; ++i) {
    values.push_back(i);
    std::byte* current =
        reinterpret_cast<std::byte*>(std::addressof(values.back()));
    const auto diff = current - previous;

    if (static_cast<std::size_t>(diff) != sizeof(int)) {
      std::cout << "page boundary before index " << i
                << " observed_page_bytes=" << current_page_size << '\n';
      current_page_size = sizeof(int);
    } else {
      current_page_size += sizeof(int);
    }

    previous = current;
  }
}

void run_tests() {
  std::vector<int> vector_values;
  std::vector<std::size_t> capacities;
  std::size_t previous_capacity = vector_values.capacity();

  for (int i = 0; i < 32; ++i) {
    vector_values.push_back(i);
    if (vector_values.capacity() != previous_capacity) {
      capacities.push_back(vector_values.capacity());
      previous_capacity = vector_values.capacity();
    }
  }

  assert(capacities.size() >= 2);
  assert(capacities[1] >= capacities[0]);

  std::deque<int> deque_values;
  for (int i = 0; i < 16; ++i) {
    deque_values.push_back(i);
  }
  assert(deque_values.front() == 0);
  assert(deque_values.back() == 15);
}

int main() {
  run_tests();
  analyze_vector_growth();
  analyze_deque_pages();
  return 0;
}
