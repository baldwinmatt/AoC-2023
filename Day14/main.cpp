#include "aoc/helpers.h"

#include <map>

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(O....#....
O.OO#....#
.....##...
OO.#O....O
.O.....O#.
O.#..O.#.#
..O..#O..O
.......O..
#....###..
#OO..#....)");
  constexpr int SR_Part1 = 136;
  constexpr int SR_Part2 = 64;

  using Row = std::vector<char>;
  using Grid = std::vector<Row>;

  const auto LoadInput = [](auto f) {
    Grid g;
    std::string_view line;
    while (aoc::getline(f, line)) {
      g.push_back(Row(line.begin(), line.end()));
    }
    return g;
  };

  const auto GridToString = [](const Grid& g, aoc::CardinalDirection dir) {
    std::string s;
    switch (dir) {
      case aoc::CardinalDirection::North:
        s.append("N\n");
        break;
      case aoc::CardinalDirection::South:
        s.append("S\n");
        break;
      case aoc::CardinalDirection::East:
        s.append("E\n");
        break;
      case aoc::CardinalDirection::West:
        s.append("W\n");
        break;
      default:
        break;
    }

    for (const auto& r : g) {
      s.append(r.begin(), r.end());
      s.append("\n");
    }

    return s;
  };

  const auto GridFromString = [](std::string_view s) {
    Grid g;
    std::string_view line;
    // this is the direction, throw it away
    aoc::getline(s, line);

    return LoadInput(s);
  };

  const auto GetColumnLoad = [](const Grid& g, size_t col) {
    int64_t load = 0;
    for (size_t y = 0; y < g.size(); ++y) {
      if (g[y][col] == 'O') {
        load += g.size() - y;
      }
    }
    return load;
  };

  const auto GetGridLoad = [](const Grid& g) {
    int64_t load = 0;
    for (size_t x = 0; x < g[0].size(); ++x) {
      load += GetColumnLoad(g, x);
    }
    return load;
  };

  const auto Transpose = [](Grid& g) {
    for (size_t i = 0; i < g.size(); ++i) {
      for (size_t j = i + 1; j < g.size(); ++j) {
        std::swap(g[i][j], g[j][i]);
      }
    }
  };

  const auto Flip = [](Grid& g) {
    for (size_t y = 0; y < g.size() / 2; ++y) {
      std::swap(g[y], g[g.size() - y - 1]);
    }
  };

  const auto TiltNorth = [](Grid& g) {
    // roll all the 'O' balls into their northmost fixed direction
    for (size_t x = 0; x < g[0].size(); ++x) {
      size_t y = 0;
      while (y < g.size())
      {
        // find the next '#' vertically down from 'y'
        auto end = y;
        for (; end < g.size(); ++end) {
          if (g[end][x] == '#') {
            break;
          }
        }
        // now count the 'O' balls between 'y' and 'end'
        size_t count = 0;
        for (size_t i = y; i < end; ++i) {
          count += g[i][x] == 'O';
        }
        // now place the 'O' balls in the new grid
        for (size_t i = y; i < y + count; ++i) {
          g[i][x] = 'O';
        }
        // now fill the rest of the space with '.'
        for (size_t i = y + count; i < end; ++i) {
          g[i][x] = '.';
        }
        // advance y past our next rock
        y = end + 1;
      }
    }
  };

  const auto TiltSouth = [](Grid& g) {
    // flip the grid upside down
    Flip(g);

    // Tilt it north
    TiltNorth(g);

    // flip the grid back
    Flip(g);
  };

  const auto TiltWest = [](Grid& g) {
    // Transpose the grid
    Transpose(g);
    // Tilt it north
    TiltNorth(g);

    // Transpose it back
    Transpose(g);
  };

  const auto TiltEast = [](Grid& g) {
    // Transpose the grid
    Transpose(g);
    // Flip it
    Flip(g);
    // Tilt it north
    TiltNorth(g);

    // Flip it back
    Flip(g);
    // Transpose it back
    Transpose(g);
  };

  const auto TiltByDir = [](Grid& g, aoc::CardinalDirection dir) {
    switch (dir) {
      case aoc::CardinalDirection::North:
        TiltNorth(g);
        break;
      case aoc::CardinalDirection::South:
        TiltSouth(g);
        break;
      case aoc::CardinalDirection::East:
        TiltEast(g);
        break;
      case aoc::CardinalDirection::West:
        TiltWest(g);
        break;
      default:
        throw std::runtime_error("Invalid direction");
    }
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Grid g;
  if (inTest) {
    g = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    g = LoadInput(f);
  }

  int64_t part1 = 0;
  {
    aoc::AutoTimer t1{"Part 2"};
    Grid ng = g;
    TiltNorth(ng);
    part1 = GetGridLoad(ng);
  }

  int64_t part2 = 0;
  {
    aoc::AutoTimer t2{"Part 2"};

    constexpr int64_t spins = 1000000000;
    constexpr int64_t rotations = 4 * spins;
    constexpr aoc::CardinalDirection dirs[] = {
      aoc::CardinalDirection::North,
      aoc::CardinalDirection::West,
      aoc::CardinalDirection::South,
      aoc::CardinalDirection::East,
    };
    constexpr auto n_dirs = std::size(dirs);

    // this is gonna be cyclical, so we need to find the cycle, then some modulo math
    std::map<std::string, int64_t> seen;
    std::map<int64_t, std::string_view> cache;
    for (int64_t i = 0; i < rotations; ++i) {
      const auto dir = dirs[i % n_dirs];
      const auto s = GridToString(g, dir);
      DEBUG_PRINT(s);
      const auto it = seen.emplace(std::move(s), i);
      if (!it.second) {
        // we've seen this grid before, so we can find the cycle length

        const auto cycle_length = i - it.first->second;
        DEBUG_PRINT("Cycle length: " << cycle_length);
        const auto remaining_cycles = (rotations - i) % cycle_length;
        const auto target_idx = remaining_cycles + it.first->second;

        DEBUG_PRINT("Target index: " << target_idx);

        // find the element in the cache with the target index
        const auto cit = cache.find(target_idx);
        if (cit == cache.end()) {
          throw std::runtime_error("Failed to find target index");
        }
        part2 = GetGridLoad(GridFromString(cit->second));

        break;
      }
      cache.emplace(i, it.first->first);

      TiltByDir(g, dir);
    }

  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
