#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <thread>
#include <vector>

class Timer {
 public:
  using duration_t = std::chrono::duration<double>;
  using clock_t = std::chrono::steady_clock;

  void start() {
    if (running_) {
      throw std::logic_error("timer is already running");
    }
    running_ = true;
    begin_ = clock_t::now();
  }

  void stop() {
    if (!running_) {
      throw std::logic_error("timer is not running");
    }
    const clock_t::time_point end = clock_t::now();
    measurements_.push_back(
        std::chrono::duration_cast<duration_t>(end - begin_));
    running_ = false;
  }

  double average() const {
    if (measurements_.empty()) {
      return 0.0;
    }

    const duration_t sum = std::accumulate(
        measurements_.begin(), measurements_.end(), duration_t::zero());
    return sum.count() / static_cast<double>(measurements_.size());
  }

  std::size_t count() const { return measurements_.size(); }

 private:
  bool running_ = false;
  clock_t::time_point begin_{};
  std::vector<duration_t> measurements_;
};

bool equal(double lhs, double rhs, double epsilon = 1e-4) {
  return std::abs(lhs - rhs) <= epsilon;
}

int main() {
  Timer timer;

  timer.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  timer.stop();

  timer.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(30));
  timer.stop();

  timer.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(40));
  timer.stop();

  const double avg_seconds = timer.average();

  std::cout << "measurements: " << timer.count() << '\n';
  std::cout << "average (sec): " << avg_seconds << '\n';

  assert(timer.count() == 3U);
  assert(avg_seconds > 0.0);
  assert(equal(avg_seconds, 0.03, 0.02));

  return 0;
}
