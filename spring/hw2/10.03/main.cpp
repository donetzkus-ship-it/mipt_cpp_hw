#include <cstddef>
#include <iostream>

#include <boost/multi_array.hpp>

using Field = boost::multi_array<int, 2>;

constexpr std::size_t kRows = 10;
constexpr std::size_t kCols = 10;
constexpr int kIterations = 50;

int count_neighbors(const Field& field, std::size_t row, std::size_t col) {
  int count = 0;

  for (int dr = -1; dr <= 1; ++dr) {
    for (int dc = -1; dc <= 1; ++dc) {
      if (dr == 0 && dc == 0) {
        continue;
      }

      const int nr = static_cast<int>(row) + dr;
      const int nc = static_cast<int>(col) + dc;
      if (nr >= 0 && nr < static_cast<int>(kRows) && nc >= 0 &&
          nc < static_cast<int>(kCols)) {
        count += field[static_cast<std::size_t>(nr)][static_cast<std::size_t>(nc)];
      }
    }
  }

  return count;
}

Field next_generation(const Field& current) {
  Field next(boost::extents[kRows][kCols]);

  for (std::size_t row = 0; row < kRows; ++row) {
    for (std::size_t col = 0; col < kCols; ++col) {
      const int neighbors = count_neighbors(current, row, col);
      if (current[row][col] == 1) {
        next[row][col] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
      } else {
        next[row][col] = (neighbors == 3) ? 1 : 0;
      }
    }
  }

  return next;
}

void print_field(const Field& field, int iteration) {
  std::cout << "iteration " << iteration << '\n';
  for (std::size_t row = 0; row < kRows; ++row) {
    for (std::size_t col = 0; col < kCols; ++col) {
      std::cout << (field[row][col] == 1 ? '#' : '.');
    }
    std::cout << '\n';
  }
  std::cout << '\n';
}

int main() {
  Field field(boost::extents[kRows][kCols]);

  for (std::size_t row = 0; row < kRows; ++row) {
    for (std::size_t col = 0; col < kCols; ++col) {
      field[row][col] = 0;
    }
  }

  field[9][0] = 1;
  field[9][1] = 1;
  field[8][1] = 1;
  field[7][1] = 1;
  field[7][2] = 1;
  field[6][2] = 1;
  field[6][3] = 1;

  for (int iteration = 0; iteration < kIterations; ++iteration) {
    print_field(field, iteration);
    field = next_generation(field);
  }

  return 0;
}
