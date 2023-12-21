#include "aoc/helpers.h"
#include "aoc/padded_matrix.h"
#include "aoc/point.h"

#include <set>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(...........
.....###.#.
.###.##..#.
..#.#...#..
....#.#....
.##..S####.
.##..#...#.
.......##..
.##.#.####.
.##..##.##.
...........)");
  constexpr int SR_Part1 = 42;
  constexpr int SR_Part2 = 16733044;

  // points which are 'true' can be occupied
  using Map = aoc::PaddedMatrix<bool, 1>;
  
  struct Input {
    Map map;
    aoc::point start;
  };

  const auto LoadInput = [](auto f) {
    Input r;
    std::string_view line;
    while (aoc::getline(f, line)) {
      r.map.set_width(line.size());
      const auto y = r.map.add_row();
      for (size_t x = 0; x < line.size(); ++x) {
        const char c = line[x];
        if (c == 'S') {
          r.start = aoc::point{static_cast<int32_t>(x), static_cast<int32_t>(y)};
          r.map.add(x, y, true);
        } else {
          r.map.add(x, y, c == '.');
        }
      }
    }
    r.map.fill_pddding(false);
    return r;
  };

  using Positions = std::set<aoc::point>;

  const auto Step = [](const Map& m, Positions& positions, bool expand = false, Positions* history = nullptr) {
    Positions next_positions;

    static const std::vector<aoc::point> directions {
      aoc::point::up(),
      aoc::point::down(),
      aoc::point::left(),
      aoc::point::right(),
    };

    const int64_t width = m.get_width();
    const int64_t height = m.get_height();

    for (const auto& p : positions) {
      // for each of the four directions, if we can move there, then add it
      for (const auto& d : directions) {
        auto next_p{p + d};
        auto check_p = next_p;
        if (expand) {
          check_p.x = ((next_p.x % width) + width) % width;
          check_p.y = ((next_p.y % height) + height) % height;
        }
        if (m.at(check_p)) {
          if (history) {
            auto it = history->emplace(next_p);
            if (!it.second) {
              continue;
            }
          }

          next_positions.emplace(next_p);
        }
      }
    }

    positions = std::move(next_positions);
  };

  // Walk the path, returning the number of possible tiles after the given number of steps
  const auto Walk = [](const Input& r, int64_t max_steps) {
    const auto& m = r.map;
    Positions positions;
    positions.emplace(r.start);

    for (int64_t step = 0; step < max_steps; ++step) {
      Step(m, positions);
    }
    return positions.size();
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

  int64_t part1{};

  {
    aoc::AutoTimer t1{"Part 1"};

    part1 = Walk(r, 64);
  }

  int64_t part2{};

  {
    aoc::AutoTimer t2{"Part 2"};

    // Solve a quadtratic, as we expand in a square pattern
    // we need to gather 3 parameters
    std::vector<int64_t> params;

    // we need to know the number of squares after each step
    std::vector<int64_t> counts;
    counts.push_back(1);

    // we need to keep history this time
    Positions visited;
    visited.emplace(r.start);
    constexpr int64_t goal_steps = 26501365;

    // has to be a square for this to work
    assert(r.map.get_height() == r.map.get_width());

    Positions positions;
    positions.emplace(r.start);
    const auto& map = r.map;
    const int64_t m_width = map.get_width();
    const int64_t delta = (goal_steps % m_width);
    for (int64_t i = 0; params.size() < 3 && i < goal_steps; ++i) {
      Step(map, positions, true, &visited);

      if ((i % m_width) == delta) {
        DEBUG_PRINT("Found " << i);
        int64_t len{0};
        for (size_t j = 0; j < counts.size(); ++j) {
          if (static_cast<int64_t>(j % 2) == static_cast<int64_t>(i % 2)) {
            len += counts[j];
          }
        }
        params.push_back(len);
      }
      counts.push_back(positions.size());
    }

    const auto a = params[0];
    const auto b = params[1] - params[0];
    const auto c = params[2] - params[1];
    const auto d = goal_steps / m_width;
    DEBUG_LOG("part2", a, b, c ,d );
    part2 = {a + b * d + (d * (d - 1) / 2) * (c - b)};
  }

  aoc::print_results(part1, part2); // 637537341306357

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
