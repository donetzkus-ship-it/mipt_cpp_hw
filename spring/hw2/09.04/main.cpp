#include <algorithm>
#include <cassert>
#include <deque>
#include <iterator>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

template <typename Iterator>
void insertion_sort(Iterator first, Iterator last) {
  if (first == last) {
    return;
  }

  for (Iterator i = std::next(first); i != last; ++i) {
    Iterator j = i;
    while (j != first && *j < *std::prev(j)) {
      std::iter_swap(j, std::prev(j));
      std::advance(j, -1);
    }
  }
}

template <typename Iterator>
auto choose_pivot(Iterator first, Iterator last) {
  Iterator middle = first;
  std::advance(middle, std::distance(first, last) / 2);
  std::vector<typename std::iterator_traits<Iterator>::value_type> candidates = {
      *first, *middle, *std::prev(last)};
  insertion_sort(candidates.begin(), candidates.end());
  return candidates[1];
}

template <typename Iterator>
Iterator partition(Iterator first, Iterator last) {
  const auto pivot = choose_pivot(first, last);
  Iterator left = first;
  Iterator right = std::prev(last);

  while (true) {
    while (*left < pivot) {
      std::advance(left, 1);
    }
    while (pivot < *right) {
      std::advance(right, -1);
    }
    if (std::distance(left, right) <= 0) {
      return right;
    }
    std::iter_swap(left, right);
    std::advance(left, 1);
    std::advance(right, -1);
  }
}

template <typename Iterator>
void quick_sort(Iterator first, Iterator last) {
  const auto size = std::distance(first, last);
  if (size <= 1) {
    return;
  }

  if (size <= 16) {
    insertion_sort(first, last);
    return;
  }

  Iterator split = partition(first, last);
  quick_sort(first, std::next(split));
  quick_sort(std::next(split), last);
}

template <typename Iterator>
void iterator_sort(Iterator first, Iterator last) {
  quick_sort(first, last);
}

int main() {
  std::vector<int> ints = {5, 1, 3, 3, 2, 8, 1, 0, -4, 5};
  iterator_sort(ints.begin(), ints.end());
  assert(std::ranges::is_sorted(ints));

  std::vector<std::string> strings = {"zebra", "apple", "banana", "apple",
                                      "kiwi"};
  iterator_sort(strings.begin(), strings.end());
  assert(std::ranges::is_sorted(strings));

  std::deque<double> doubles(1000);
  for (std::size_t i = 0; i < doubles.size(); ++i) {
    doubles[i] = static_cast<double>(doubles.size() - i);
  }
  iterator_sort(doubles.begin(), doubles.end());
  assert(std::ranges::is_sorted(doubles));

  return 0;
}
