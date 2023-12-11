#include "aoc/helpers.h"

#include <set>
#include <vector>

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(...#......
.......#..
#.........
..........
......#...
.#........
.........#
..........
.......#..
#...#.....)");
  constexpr int SR_Part1 = 374;
  constexpr int SR_Part2 = 0;

  using Points = std::vector<aoc::Point>;
  using EmptyList = std::vector<int64_t>;

  struct Input {
    Points points;
    EmptyList empty_rows;
    EmptyList empty_cols;
  };

  const auto LoadInput = [](auto f) {
    std::string_view line;
    int64_t y = 0;
    Input r;
    std::set<int64_t> x_points;
    std::vector<std::string_view> universe;
    while (aoc::getline(f, line)) {
      universe.push_back(line);
      bool empty = true;
      const int64_t w = line.size();
      for (int64_t x = 0; x < w; ++x) {
        if (line[x] == '#') {
          r.points.emplace_back(x, y);
          empty = false;
          x_points.emplace(x);
        }
      }
      if (empty) {
        r.empty_rows.push_back(y);
        DEBUG_PRINT("Empty row " << y);
      }
      y++;
    }

    for (size_t i = 0; i < universe[0].size(); ++i) {
      bool empty = !std::any_of(universe.begin(), universe.end(), [i, &x_points](auto s) {
        return s[i] == '#';
      });
      if (empty) {
        r.empty_cols.push_back(i);
        DEBUG_PRINT("Empty col " << i);
      }
    }

    return r;
  };

  const auto get_cosmic_expansion = [](int64_t a, int64_t b, const EmptyList& expanded, const int64_t expansion_factor) -> int64_t {
    const auto l = std::min(a, b);
    const auto r = std::max(a, b);

    int64_t distance = 0;
    auto idx = std::lower_bound(expanded.begin(), expanded.end(), l);
    while (idx != expanded.end() && *idx < r) {
      distance += expansion_factor - 1;
      ++idx;
    }
    return distance;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Input r;
  if (inTest) {
    r = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f);
  }


  const auto pair_wise_cosmic_distance = [&r](const aoc::Point& p1, const aoc::Point& p2, int64_t expansion_factor) {
    const auto x_exp = get_cosmic_expansion(p1.first, p2.first, r.empty_cols, expansion_factor);
    const auto y_exp = get_cosmic_expansion(p1.second, p2.second, r.empty_rows, expansion_factor);
    const auto dist = aoc::manhattan(p1, p2);
    return dist + x_exp + y_exp;
  };

  int64_t part1 = 0;
  {
    aoc::AutoTimer t1{"Part 1"};
    for (size_t it = 0; it < r.points.size() - 1; ++it) {
      const auto& p = r.points[it];
      for (size_t next = it + 1; next < r.points.size(); ++next) {
        const auto& n = r.points[next];
        const auto dist = pair_wise_cosmic_distance(p, n, 2);
        DEBUG_PRINT("Distance from [" << (it + 1) << "] " << p << " to [" << (next + 1) << "] " << n << " = " << dist);
        part1 += dist;
      }
    }
  }

  int64_t part2 = 0;
  {
    aoc::AutoTimer t2{"Part 2"};
    for (size_t it = 0; it < r.points.size() - 1; ++it) {
      const auto& p = r.points[it];
      for (size_t next = it + 1; next < r.points.size(); ++next) {
        const auto& n = r.points[next];
        const auto dist = pair_wise_cosmic_distance(p, n, 1000000);
        DEBUG_PRINT("Distance from [" << (it + 1) << "] " << p << " to [" << (next + 1) << "] " << n << " = " << dist);
        part2 += dist;
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
