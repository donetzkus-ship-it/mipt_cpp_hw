#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
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
void hybrid_sort(std::vector<T>& values, std::size_t left, std::size_t right,
                 std::size_t threshold) {
  if (right <= left + 1) {
    return;
  }

  if (right - left <= threshold) {
    insertion_sort(values, left, right);
    return;
  }

  const std::size_t split = partition(values, left, right);
  hybrid_sort(values, left, split + 1, threshold);
  hybrid_sort(values, split + 1, right, threshold);
}

template <typename T>
void sort(std::vector<T>& values, std::size_t threshold) {
  hybrid_sort(values, 0, values.size(), threshold);
}

std::vector<double> make_reverse_sorted_data() {
  std::vector<double> values(10'000);
  for (std::size_t i = 0; i < values.size(); ++i) {
    values[i] = static_cast<double>(values.size() - i);
  }
  return values;
}

static void BM_HybridSortThreshold(benchmark::State& state) {
  const std::size_t threshold = static_cast<std::size_t>(state.range(0));

  for (auto _ : state) {
    std::vector<double> values = make_reverse_sorted_data();
    sort(values, threshold);
    benchmark::DoNotOptimize(values.data());
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(state.iterations() * 10'000);
}

BENCHMARK(BM_HybridSortThreshold)->Arg(4)->Arg(8)->Arg(16)->Arg(32)->Arg(64);

BENCHMARK_MAIN();

// Running ./build/spring/hw1/07.05/hw1_07_05_bench
// Run on (2 X 48 MHz CPU s)
// Load Average: 0.15, 0.05, 0.05
// ***WARNING*** Library was built as DEBUG. Timings may be affected.
// ------------------------------------------------------------------------------------
// Benchmark                          Time             CPU   Iterations
// UserCounters...
// ------------------------------------------------------------------------------------
// BM_HybridSortThreshold/4      573481 ns       572914 ns         1038
// items_per_second=17.4546M/s BM_HybridSortThreshold/8      445454 ns 445367 ns
// 1577 items_per_second=22.4534M/s BM_HybridSortThreshold/16     347779 ns
// 347721 ns         2014 items_per_second=28.7587M/s BM_HybridSortThreshold/32
// 285754 ns       285690 ns         2441 items_per_second=35.003M/s
// BM_HybridSortThreshold/64     242019 ns       241954 ns         2856
// items_per_second=41.3302M/s