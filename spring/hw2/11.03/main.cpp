#include <algorithm>
#include <cassert>
#include <deque>
#include <functional>
#include <iterator>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

template <typename Iterator, typename Compare>
void insertion_sort(Iterator first, Iterator last, Compare compare) {
  if (first == last) {
    return;
  }

  for (Iterator i = std::next(first); i != last; ++i) {
    Iterator j = i;
    while (j != first && compare(*j, *std::prev(j))) {
      std::iter_swap(j, std::prev(j));
      std::advance(j, -1);
    }
  }
}

template <typename Iterator, typename Compare>
auto choose_pivot(Iterator first, Iterator last, Compare compare) {
  Iterator middle = first;
  std::advance(middle, std::distance(first, last) / 2);
  std::vector<typename std::iterator_traits<Iterator>::value_type> candidates = {
      *first, *middle, *std::prev(last)};
  insertion_sort(candidates.begin(), candidates.end(), compare);
  return candidates[1];
}

template <typename Iterator, typename Compare>
Iterator do_partition(Iterator first, Iterator last, Compare compare) {
  const auto pivot = choose_pivot(first, last, compare);
  Iterator left = first;
  Iterator right = std::prev(last);

  while (true) {
    while (compare(*left, pivot)) {
      std::advance(left, 1);
    }
    while (compare(pivot, *right)) {
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

template <typename Iterator, typename Compare>
void quick_sort(Iterator first, Iterator last, Compare compare) {
  const auto size = std::distance(first, last);
  if (size <= 1) {
    return;
  }

  if (size <= 16) {
    insertion_sort(first, last, compare);
    return;
  }

  Iterator split = do_partition(first, last, compare);
  quick_sort(first, std::next(split), compare);
  quick_sort(std::next(split), last, compare);
}

template <typename Iterator, typename Compare>
void iterator_sort(Iterator first, Iterator last, Compare compare) {
  quick_sort(first, last, compare);
}

bool longer_string_first(const std::string& lhs, const std::string& rhs) {
  if (lhs.size() != rhs.size()) {
    return lhs.size() > rhs.size();
  }
  return lhs < rhs;
}

int main() {
  std::vector<std::string> strings = {"a", "aaaaa", "aaaaaaaa", "a",
                                      "aaaaaaa", "aa"};
  iterator_sort(strings.begin(), strings.end(), longer_string_first);
  assert(std::ranges::is_sorted(strings, longer_string_first));

  std::deque<int> ints = {5, 1, 3, 3, 2, 8, 1, 0, -4, 5};
  iterator_sort(ints.begin(), ints.end(), std::less<int>{});
  assert(std::ranges::is_sorted(ints, std::less<int>{}));

  std::vector<double> doubles = {3.5, -1.0, 2.25, 2.25, 9.0, 0.5};
  auto descending = [](double lhs, double rhs) { return lhs > rhs; };
  iterator_sort(doubles.begin(), doubles.end(), descending);
  assert(std::ranges::is_sorted(doubles, descending));

  return 0;
}
