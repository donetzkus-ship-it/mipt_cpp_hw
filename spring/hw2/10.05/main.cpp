// Форма кривых: для равномерно распределённого 32-битного хэша ожидаемое
// число коллизий при N строках ~ N^2 / (2 * 2^32). Значит, в идеальном случае
// зависимость близка к квадратичной от размера выборки. Отклонения вверх
// от теоретической кривой = смещение распределения конкретной хэш-функции
// (кластеризация значений). Построю лог график чтобы не слипались линии.

#include <array>
#include <cstdint>
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

std::vector<std::string> make_dataset(std::size_t count) {
  std::default_random_engine engine(42U);
  std::uniform_int_distribution<int> length_dist(8, 32);
  std::uniform_int_distribution<int> char_dist(0, 25);

  std::vector<std::string> values;
  values.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    const int length = length_dist(engine);
    std::string value(static_cast<std::size_t>(length), 'a');
    for (char& ch : value) {
      ch = static_cast<char>('a' + char_dist(engine));
    }
    values.push_back(std::move(value));
  }
  return values;
}

int main() {
  const std::array<HashEntry, 9> functions = {{
      {"RS", rs_hash},   {"JS", js_hash},   {"PJW", pjw_hash},
      {"ELF", elf_hash}, {"BKDR", bkdr_hash}, {"SDBM", sdbm_hash},
      {"DJB", djb_hash}, {"DEK", dek_hash}, {"AP", ap_hash},
  }};

  const std::vector<std::size_t> checkpoints = {10000,  25000,  50000, 100000,
                                                250000, 1000000};
  const std::size_t max_size = checkpoints.back();
  const std::vector<std::string> dataset = make_dataset(max_size);

  std::cout << "hash_function,sample_size,collisions\n";

  std::vector<std::size_t> final_collisions(functions.size(), 0);

  for (std::size_t fi = 0; fi < functions.size(); ++fi) {
    const HashEntry& entry = functions[fi];
    std::unordered_set<std::uint32_t> seen;
    seen.reserve(max_size);
    std::size_t collisions = 0;
    std::size_t next_checkpoint = 0;

    for (std::size_t i = 0; i < dataset.size(); ++i) {
      const std::uint32_t hash = entry.function(dataset[i]);
      if (!seen.insert(hash).second) {
        ++collisions;
      }
      if (next_checkpoint < checkpoints.size() &&
          i + 1 == checkpoints[next_checkpoint]) {
        std::cout << entry.name << ',' << checkpoints[next_checkpoint] << ','
                  << collisions << '\n';
        ++next_checkpoint;
      }
    }
    final_collisions[fi] = collisions;
  }

  std::size_t best_score = std::numeric_limits<std::size_t>::max();
  std::size_t worst_score = 0;
  std::string best_name;
  std::string worst_name;

  for (std::size_t fi = 0; fi < functions.size(); ++fi) {
    if (final_collisions[fi] < best_score) {
      best_score = final_collisions[fi];
      best_name = functions[fi].name;
    }
    if (final_collisions[fi] > worst_score) {
      worst_score = final_collisions[fi];
      worst_name = functions[fi].name;
    }
  }

  std::cout << "best=" << best_name << " collisions_at_" << max_size << '='
            << best_score << '\n';
  std::cout << "worst=" << worst_name << " collisions_at_" << max_size << '='
            << worst_score << '\n';

  return 0;
}
