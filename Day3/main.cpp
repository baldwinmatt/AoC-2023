#include "aoc/helpers.h"
#include "aoc/padded_matrix.h"

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(467..114..
...*......
..35..633.
......#...
617*......
.....+.58.
..592.....
......755.
...$.*....
.664.598..)");
  constexpr int SR_Part1 = 4361;
  constexpr int SR_Part2 = 467835;

  const auto LoadInput = [](auto f, aoc::PaddedMatrix<char>& m) {
    std::string_view line;
    while (aoc::getline(f, line)) {
      m.set_width(line.size());
      const auto row = m.add_row();
      for (size_t i = 0; i < line.size(); ++i) {
        m.add(i, row, line[i]);
      }
    }
    m.fill_pddding('.');
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  aoc::PaddedMatrix<char> map;
  if (inTest) {
    LoadInput(SampleInput, map);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    LoadInput(f, map);
  }
  t.elapsed();
  t.reset();
  int64_t part1 = 0;
  int64_t part2 = 0;

  // scan the map to look for runs of numbers, then for each run of numerics, see if there is an adjacent symbol
  for (ssize_t y = 0; y < map.get_height(); ++y) {
    ssize_t runStart = 0;
    bool inRun = false;
    size_t val = 0;
    const auto is_point_symbol = [&map](ssize_t x, ssize_t y) -> bool {
      const auto v = map.at(x,y);
      return v != '.' && !aoc::is_numeric(v);
    };
    const auto get_number_value = [&map](ssize_t x, ssize_t y) -> int64_t {
      int64_t v = 0;
      while (aoc::is_numeric(map.at(x, y))) {
        v *= 10;
        v += map.at(x, y) - '0';
        x++;
      }
      return v;
    };

    const auto is_point_numeric = [&map](ssize_t x, ssize_t y) -> bool {
      const auto v = map.at(x, y);
      return aoc::is_numeric(v);
    };

    const auto find_number_start = [&map](ssize_t x, ssize_t y) -> ssize_t {
      ssize_t x_off = 1;
      while (aoc::is_numeric(map.at(x - x_off, y))) {
        x_off++;
      }
      x_off--;
      return x - x_off;
    };


    for (ssize_t x = 0; x < map.get_width() + 1; ++x) {

      // Part 1
      if (!inRun && is_point_numeric(x, y)) {
        inRun = true;
        runStart = x;
        val = map.at(x, y) - '0';
      } else if (inRun && !is_point_numeric(x, y)) {
        inRun = false;

        // only if one of adject points is a symbol (i.e. not numeric and not a '.'), do we include it in the sum
        bool valid = false;

        valid = is_point_symbol(runStart - 1, y) || is_point_symbol(x, y);

        for (ssize_t lx = runStart - 1; !valid && lx < x + 1; lx++) {
          valid = (is_point_symbol(lx, y - 1) ||
            is_point_symbol(lx, y + 1));
        }

        DEBUG_PRINT("valid: " << valid << " val: " << val);
        if (valid) {
          part1 += val;
        }
      } else if (inRun) {
        val *= 10;
        val += map.at(x, y) - '0';
      }

      // part 2
      if (map.at(x, y) == '*') {
        // find adjacent numbers
        std::vector<int64_t> adj;

        // ? N ?
        // N * N
        // ? N ?
        const std::array<aoc::Point, 4> cardinal_dirs = {
          aoc::Point(x - 1, y),
          aoc::Point(x + 1, y),
          aoc::Point(x, y - 1),
          aoc::Point(x, y + 1)
        };
        for (auto dir : cardinal_dirs) {
          if (is_point_numeric(dir.first, dir.second)) {
            const auto start = find_number_start(dir.first, dir.second);
            const auto v = get_number_value(start, dir.second);
            adj.push_back(v);
            DEBUG_PRINT("adj(" << dir.first << ", " << dir.second << "): " << v);
          }
        }

        // N . N
        // . * .
        // N . N
        const std::array<aoc::Point, 4> diagonal_dirs = {
          aoc::Point(x - 1, y - 1),
          aoc::Point(x - 1, y + 1),
          aoc::Point(x + 1, y - 1),
          aoc::Point(x + 1, y + 1)
        };
        for (auto dir : diagonal_dirs) {
          if (is_point_numeric(dir.first, dir.second) && !is_point_numeric(x, dir.second)) {
            const auto start = find_number_start(dir.first, dir.second);
            const auto v = get_number_value(start, dir.second);
            adj.push_back(v);
            DEBUG_PRINT("adj(" << dir.first << ", " << dir.second << "): " << v);
          }
        }

        if (adj.size() == 2) {
          DEBUG_PRINT("part2: " << adj[0] << " * " << adj[1]);
          part2 += (adj[0] * adj[1]);
        }
      }
    }
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
