#include <algorithm>
#include <cassert>
#include <stack>
#include <utility>

template <typename T>
class Stack_v1 {
 public:
  void push(T value) {
    stack_.emplace(value, std::empty(stack_) ? value
                                             : std::min(value, stack_.top().second));
  }

  T top() const { return stack_.top().first; }

  void pop() { stack_.pop(); }

  T min() const { return stack_.top().second; }

 private:
  std::stack<std::pair<T, T>> stack_;
};

template <typename T>
class Stack_v2 {
 public:
  void push(T value) {
    if (std::empty(stack_)) {
      stack_.push(value);
      min_ = value;
    } else if (value < min_) {
      stack_.push(2 * value - min_);
      min_ = value;
    } else {
      stack_.push(value);
    }
  }

  T top() const { return stack_.top() < min_ ? min_ : stack_.top(); }

  void pop() {
    if (const T top_value = stack_.top(); top_value < min_) {
      min_ = 2 * min_ - top_value;
    }
    stack_.pop();
  }

  T min() const { return min_; }

 private:
  std::stack<T> stack_;
  T min_ = T();
};

int main() {
  {
    Stack_v1<int> stack;
    stack.push(3);
    assert(stack.top() == 3 && stack.min() == 3);

    stack.push(1);
    assert(stack.top() == 1 && stack.min() == 1);

    stack.push(2);
    assert(stack.top() == 2 && stack.min() == 1);

    stack.pop();
    assert(stack.top() == 1 && stack.min() == 1);

    stack.pop();
    assert(stack.top() == 3 && stack.min() == 3);
  }

  {
    Stack_v2<int> stack;
    stack.push(3);
    assert(stack.top() == 3 && stack.min() == 3);

    stack.push(1);
    assert(stack.top() == 1 && stack.min() == 1);

    stack.push(2);
    assert(stack.top() == 2 && stack.min() == 1);

    stack.pop();
    assert(stack.top() == 1 && stack.min() == 1);

    stack.pop();
    assert(stack.top() == 3 && stack.min() == 3);
  }

  return 0;
}
