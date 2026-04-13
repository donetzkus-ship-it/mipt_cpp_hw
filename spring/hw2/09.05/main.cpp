#include <cassert>
#include <iterator>
#include <memory>

template <typename T>
class List {
 private:
  struct Node {
    explicit Node(T value) : x(std::move(value)) {}

    T x = T();
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;
  };

 public:
  class Iterator {
   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    Iterator(std::shared_ptr<Node> node = nullptr,
             std::shared_ptr<Node> tail = nullptr)
        : node_(std::move(node)), tail_(std::move(tail)) {}

    Iterator operator++(int) {
      Iterator copy = *this;
      node_ = node_->next;
      return copy;
    }

    Iterator& operator++() {
      node_ = node_->next;
      return *this;
    }

    Iterator operator--(int) {
      Iterator copy = *this;
      --(*this);
      return copy;
    }

    Iterator& operator--() {
      if (node_) {
        node_ = node_->prev.lock();
      } else {
        node_ = tail_;
      }
      return *this;
    }

    reference operator*() const { return node_->x; }
    pointer operator->() const { return &node_->x; }

    friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
      return lhs.node_ == rhs.node_;
    }

   private:
    std::shared_ptr<Node> node_;
    std::shared_ptr<Node> tail_;
  };

  Iterator begin() const { return Iterator(head_, tail_); }
  Iterator end() const { return Iterator(nullptr, tail_); }

  void push_back(T value) {
    auto node = std::make_shared<Node>(std::move(value));

    if (tail_) {
      tail_->next = node;
      node->prev = tail_;
      tail_ = node;
    } else {
      head_ = node;
      tail_ = node;
    }
  }

 private:
  std::shared_ptr<Node> head_;
  std::shared_ptr<Node> tail_;
};

int main() {
  List<int> list;
  list.push_back(1);
  list.push_back(2);
  list.push_back(3);

  auto it = list.begin();
  assert(*it == 1);
  ++it;
  assert(*it == 2);
  ++it;
  assert(*it == 3);
  --it;
  assert(*it == 2);

  auto end = list.end();
  --end;
  assert(*end == 3);
  --end;
  assert(*end == 2);
  --end;
  assert(*end == 1);

  for ([[maybe_unused]] int value : list) {
  }

  return 0;
}
