#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>

constexpr double kMutationProbability = 0.05;
constexpr int kPopulationSize = 100;
const std::string kTarget = "methinksitislikeaweasel";

int metric(const std::string& lhs, const std::string& rhs) {
  int distance = 0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    if (lhs[i] != rhs[i]) {
      ++distance;
    }
  }
  return distance;
}

char random_letter(std::default_random_engine& engine,
                   std::uniform_int_distribution<int>& letter_dist) {
  return static_cast<char>('a' + letter_dist(engine));
}

std::string random_string(std::size_t size, std::default_random_engine& engine,
                          std::uniform_int_distribution<int>& letter_dist) {
  std::string result(size, 'a');
  for (char& symbol : result) {
    symbol = random_letter(engine, letter_dist);
  }
  return result;
}

std::string mutate(const std::string& parent,
                   std::default_random_engine& engine,
                   std::uniform_int_distribution<int>& letter_dist,
                   std::uniform_real_distribution<double>& probability_dist) {
  std::string child = parent;
  for (char& symbol : child) {
    if (probability_dist(engine) < kMutationProbability) {
      symbol = random_letter(engine, letter_dist);
    }
  }
  return child;
}

int main() {
  std::random_device random_device;
  std::default_random_engine engine(random_device());
  std::uniform_int_distribution<int> letter_dist(0, 25);
  std::uniform_real_distribution<double> probability_dist(0.0, 1.0);

  std::string parent = random_string(kTarget.size(), engine, letter_dist);

  int iteration = 0;
  while (true) {
    std::vector<std::string> generation;
    generation.reserve(kPopulationSize);

    int best_index = 0;
    int best_metric = static_cast<int>(kTarget.size()) + 1;

    std::cout << "iteration " << iteration << '\n';
    std::cout << "parent:  " << parent << " (metric=" << metric(parent, kTarget)
              << ")\n";

    for (int i = 0; i < kPopulationSize; ++i) {
      generation.push_back(
          mutate(parent, engine, letter_dist, probability_dist));
      const int value = metric(generation.back(), kTarget);

      std::cout << "child " << i << ": " << generation.back()
                << " (metric=" << value << ")\n";

      if (value < best_metric) {
        best_metric = value;
        best_index = i;
      }
    }

    std::cout << "best:   " << generation[best_index]
              << " (metric=" << best_metric << ")\n\n";

    if (best_metric == 0) {
      std::cout << "target reached: " << generation[best_index] << '\n';
      break;
    }

    parent = generation[best_index];
    ++iteration;
  }

  return 0;
}
