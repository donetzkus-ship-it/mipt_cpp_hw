#include <cassert>
#include <iostream>
#include <iterator>
#include <vector>

#if __has_include(<boost/iterator/iterator_facade.hpp>)
#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>
#define HW2_09_06_HAS_BOOST 1
#else
#define HW2_09_06_HAS_BOOST 0
#endif

class Iterator {
 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = int;
  using difference_type = std::ptrdiff_t;
  using pointer = const int*;
  using reference = const int&;

  Iterator(int current = 0, int next = 1, int index = 0, int limit = 0)
      : current_(current), next_(next), index_(index), limit_(limit) {}

  Iterator operator++(int) {
    Iterator copy = *this;
    ++(*this);
    return copy;
  }

  Iterator& operator++() {
    const int new_next = current_ + next_;
    current_ = next_;
    next_ = new_next;
    ++index_;
    return *this;
  }

  int operator*() const { return current_; }

  friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
    return lhs.index_ == rhs.index_ && lhs.limit_ == rhs.limit_;
  }

 private:
  int current_ = 0;
  int next_ = 1;
  int index_ = 0;
  int limit_ = 0;
};

std::vector<int> fibonacci_sequence(int count) {
  std::vector<int> values;
  values.reserve(static_cast<std::size_t>(count));

  for (Iterator it(0, 1, 0, count), end(0, 1, count, count); it != end; ++it) {
    values.push_back(*it);
  }

  return values;
}

#if HW2_09_06_HAS_BOOST
class FacadeIterator
    : public boost::iterator_facade<FacadeIterator, int,
                                    boost::forward_traversal_tag, int> {
 public:
  FacadeIterator(int current = 0, int next = 1, int index = 0, int limit = 0)
      : current_(current), next_(next), index_(index), limit_(limit) {}

 private:
  friend class boost::iterator_core_access;

  void increment() {
    const int new_next = current_ + next_;
    current_ = next_;
    next_ = new_next;
    ++index_;
  }

  int dereference() const { return current_; }

  bool equal(const FacadeIterator& other) const {
    return index_ == other.index_ && limit_ == other.limit_;
  }

  int current_ = 0;
  int next_ = 1;
  int index_ = 0;
  int limit_ = 0;
};

std::vector<int> fibonacci_sequence_facade(int count) {
  std::vector<int> values;
  values.reserve(static_cast<std::size_t>(count));

  for (FacadeIterator it(0, 1, 0, count), end(0, 1, count, count); it != end;
       ++it) {
    values.push_back(*it);
  }

  return values;
}
#endif

int main() {
  const std::vector<int> values = fibonacci_sequence(10);
  const std::vector<int> expected = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
  assert(values == expected);

#if HW2_09_06_HAS_BOOST
  const std::vector<int> facade_values = fibonacci_sequence_facade(10);
  assert(facade_values == expected);
  std::cout << "BOOST OK\n";
#endif

  for (int value : values) {
    std::cout << value << ' ';
  }
  std::cout << '\n';

  return 0;
}
