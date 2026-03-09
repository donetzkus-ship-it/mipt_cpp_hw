#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

template <typename T>
void insertion_sort(std::vector<T>& values, std::size_t left,
                    std::size_t right) {
  for (std::size_t i = left + 1; i < right; ++i) {
    for (std::size_t j = i; j > left && values[j - 1] > values[j]; --j) {
      std::swap(values[j], values[j - 1]);
    }
  }
}

template <typename T>
T choose_pivot(const std::vector<T>& values, std::size_t left,
               std::size_t right) {
  std::vector<T> candidates = {values[left], values[left + (right - left) / 2],
                               values[right - 1]};
  insertion_sort(candidates, 0, candidates.size());
  return candidates[1];
}

template <typename T>
std::size_t partition(std::vector<T>& values, std::size_t left,
                      std::size_t right) {
  const T pivot = choose_pivot(values, left, right);
  std::size_t i = left;
  std::size_t j = right - 1;

  while (true) {
    while (values[i] < pivot) {
      ++i;
    }
    while (values[j] > pivot) {
      --j;
    }
    if (i >= j) {
      return j;
    }
    std::swap(values[i], values[j]);
    ++i;
    --j;
  }
}

template <typename T>
void quick_sort(std::vector<T>& values, std::size_t left, std::size_t right) {
  if (right <= left + 1) {
    return;
  }

  if (right - left <= 16) {
    insertion_sort(values, left, right);
    return;
  }

  const std::size_t split = partition(values, left, right);
  quick_sort(values, left, split + 1);
  quick_sort(values, split + 1, right);
}

template <typename T>
void sort(std::vector<T>& values) {
  quick_sort(values, 0, values.size());
}

TEST(SortTests, HandlesEmptyVector) {
  std::vector<int> values;
  sort(values);
  EXPECT_TRUE(values.empty());
}

TEST(SortTests, HandlesSingleElement) {
  std::vector<int> values = {42};
  sort(values);
  EXPECT_EQ(values.size(), 1u);
  EXPECT_EQ(values[0], 42);
}

TEST(SortTests, SortsIntegersWithDuplicates) {
  std::vector<int> values = {5, 1, 3, 3, 2, 8, 1, 0, -4, 5};
  sort(values);
  EXPECT_TRUE(std::ranges::is_sorted(values));
}

TEST(SortTests, SortsReverseOrderedRange) {
  std::vector<double> values(1000);
  for (std::size_t i = 0; i < values.size(); ++i) {
    values[i] = static_cast<double>(values.size() - i);
  }

  sort(values);
  EXPECT_TRUE(std::ranges::is_sorted(values));
}

TEST(SortTests, SortsStrings) {
  std::vector<std::string> values = {"zebra", "apple", "banana", "apple",
                                     "kiwi"};
  sort(values);
  EXPECT_TRUE(std::ranges::is_sorted(values));
}
