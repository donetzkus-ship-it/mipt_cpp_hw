#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

using HashFunction = std::uint32_t (*)(std::string_view);

std::uint32_t rs_hash(std::string_view text) {
  std::uint32_t a = 63689U;
  std::uint32_t b = 378551U;
  std::uint32_t hash = 0;
  for (unsigned char ch : text) {
    hash = hash * a + ch;
    a *= b;
  }
  return hash;
}

std::uint32_t js_hash(std::string_view text) {
  std::uint32_t hash = 1315423911U;
  for (unsigned char ch : text) {
    hash ^= (hash << 5) + ch + (hash >> 2);
  }
  return hash;
}

std::uint32_t pjw_hash(std::string_view text) {
  constexpr std::uint32_t bits = 32U;
  constexpr std::uint32_t three_quarters = (bits * 3U) / 4U;
  constexpr std::uint32_t one_eighth = bits / 8U;
  constexpr std::uint32_t high_bits = 0xFFFFFFFFU << (bits - one_eighth);

  std::uint32_t hash = 0;
  for (unsigned char ch : text) {
    hash = (hash << one_eighth) + ch;
    const std::uint32_t test = hash & high_bits;
    if (test != 0U) {
      hash = (hash ^ (test >> three_quarters)) & (~high_bits);
    }
  }
  return hash;
}

std::uint32_t elf_hash(std::string_view text) {
  std::uint32_t hash = 0;
  for (unsigned char ch : text) {
    hash = (hash << 4U) + ch;
    const std::uint32_t x = hash & 0xF0000000U;
    if (x != 0U) {
      hash ^= x >> 24U;
      hash &= ~x;
    }
  }
  return hash;
}

std::uint32_t bkdr_hash(std::string_view text) {
  constexpr std::uint32_t seed = 131U;
  std::uint32_t hash = 0;
  for (unsigned char ch : text) {
    hash = hash * seed + ch;
  }
  return hash;
}

std::uint32_t sdbm_hash(std::string_view text) {
  std::uint32_t hash = 0;
  for (unsigned char ch : text) {
    hash = ch + (hash << 6U) + (hash << 16U) - hash;
  }
  return hash;
}

std::uint32_t djb_hash(std::string_view text) {
  std::uint32_t hash = 5381U;
  for (unsigned char ch : text) {
    hash = ((hash << 5U) + hash) + ch;
  }
  return hash;
}

std::uint32_t dek_hash(std::string_view text) {
  std::uint32_t hash = static_cast<std::uint32_t>(text.size());
  for (unsigned char ch : text) {
    hash = ((hash << 5U) ^ (hash >> 27U)) ^ ch;
  }
  return hash;
}

std::uint32_t ap_hash(std::string_view text) {
  std::uint32_t hash = 0xAAAAAAAAU;
  for (std::size_t i = 0; i < text.size(); ++i) {
    const auto ch = static_cast<unsigned char>(text[i]);
    if ((i & 1U) == 0U) {
      hash ^= ((hash << 7U) ^ ch * (hash >> 3U));
    } else {
      hash ^= (~((hash << 11U) + (ch ^ (hash >> 5U))));
    }
  }
  return hash;
}

struct HashEntry {
  const char* name;
  HashFunction function;
};

struct Measurement {
  std::string name;
  std::size_t sample_size = 0;
  std::size_t collisions = 0;
};

std::string random_string(std::default_random_engine& engine,
                          std::uniform_int_distribution<int>& length_dist,
                          std::uniform_int_distribution<int>& char_dist) {
  const int length = length_dist(engine);
  std::string value(static_cast<std::size_t>(length), 'a');
  for (char& ch : value) {
    ch = static_cast<char>('a' + char_dist(engine));
  }
  return value;
}

std::vector<std::string> make_dataset(std::size_t count) {
  std::default_random_engine engine(42U + static_cast<unsigned>(count));
  std::uniform_int_distribution<int> length_dist(8, 32);
  std::uniform_int_distribution<int> char_dist(0, 25);

  std::vector<std::string> values;
  values.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    values.push_back(random_string(engine, length_dist, char_dist));
  }
  return values;
}

std::size_t count_collisions(const std::vector<std::string>& values,
                             HashFunction function) {
  std::unordered_set<std::uint32_t> seen;
  std::size_t collisions = 0;

  for (const std::string& value : values) {
    const std::uint32_t hash = function(value);
    if (!seen.insert(hash).second) {
      ++collisions;
    }
  }

  return collisions;
}

int main() {
  const std::array<HashEntry, 9> functions = {{
      {"RS", rs_hash},   {"JS", js_hash},   {"PJW", pjw_hash},
      {"ELF", elf_hash}, {"BKDR", bkdr_hash}, {"SDBM", sdbm_hash},
      {"DJB", djb_hash}, {"DEK", dek_hash}, {"AP", ap_hash},
  }};

  const std::vector<std::size_t> sample_sizes = {10000, 25000, 50000, 100000,
                                                 250000};
  std::vector<Measurement> measurements;
  measurements.reserve(functions.size() * sample_sizes.size());

  std::cout << "hash_function,sample_size,collisions\n";
  for (std::size_t sample_size : sample_sizes) {
    const std::vector<std::string> dataset = make_dataset(sample_size);
    for (const auto& entry : functions) {
      const std::size_t collisions = count_collisions(dataset, entry.function);
      measurements.push_back({entry.name, sample_size, collisions});
      std::cout << entry.name << ',' << sample_size << ',' << collisions << '\n';
    }
  }

  std::size_t best_score = std::numeric_limits<std::size_t>::max();
  std::size_t worst_score = 0;
  std::string best_name;
  std::string worst_name;

  for (const auto& entry : functions) {
    std::size_t total = 0;
    for (const auto& measurement : measurements) {
      if (measurement.name == entry.name) {
        total += measurement.collisions;
      }
    }

    if (total < best_score) {
      best_score = total;
      best_name = entry.name;
    }
    if (total > worst_score) {
      worst_score = total;
      worst_name = entry.name;
    }
  }

  std::cout << "best=" << best_name << " total_collisions=" << best_score
            << '\n';
  std::cout << "worst=" << worst_name << " total_collisions=" << worst_score
            << '\n';

  return 0;
}
