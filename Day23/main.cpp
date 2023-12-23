#include "aoc/helpers.h"
#include "aoc/padded_matrix.h"

#include <queue>
#include <unordered_set>

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(#.#####################
#.......#########...###
#######.#########.#.###
###.....#.>.>.###.#.###
###v#####.#v#.###.#.###
###.>...#.#.#.....#...#
###v###.#.#.#########.#
###...#.#.#.......#...#
#####.#.#.#######.#.###
#.....#.#.#.......#...#
#.#####.#.#.#########v#
#.#...#...#...###...>.#
#.#.#v#######v###.###v#
#...#.>.#...>.>.#.###.#
#####v#.#.###v#.#.###.#
#.....#...#...#.#.#...#
#.#########.###.#.#.###
#...###...#...#...#.###
###.###.#.###v#####v###
#...#...#.#.>.>.#.>.###
#.###.###.#.###.#.#v###
#.....###...###...#...#
#####################.#)");
  constexpr int SR_Part1 = 94;
  constexpr int SR_Part2 = 0;

  using Map = aoc::PaddedMatrix<char, 1>;

  struct Input {
    Map map;
    aoc::point start;
    aoc::point end;
  };

  const auto LoadInput = [](auto f) {
    std::string_view line;
    Input r;
    auto& map = r.map;
    while (aoc::getline(f, line)) {
      map.set_width(line.size());
      const auto y = map.add_row();
      for (size_t x = 0; x < line.size(); ++x) {
        map.add(x, y, line[x]);
        if (y == 0 && line[x] == '.') {
          r.start = {static_cast<int32_t>(x), 0};
        }
      }
    }
    const int32_t y = static_cast<int32_t>(map.get_height() - 1);
    for (int32_t x = 0; x < map.get_width(); ++x) {
      if (map.at(x, y) == '.') {
        r.end = {x, y};
      }
    }
    r.map.fill_pddding('#');
    return r;
  };

  using VisitedMap = aoc::PaddedMatrix<bool, 1>;
  using MaxMap = aoc::PaddedMatrix<int64_t, 1>;

  void dfs(const Input& input, const aoc::point pos, int64_t steps, VisitedMap& visited, MaxMap& max) {
    const auto& map = input.map;

    if (map.at(pos) == '#') {
      return;
    }
    if (visited.at(pos)) {
      return;
    }
    visited.at(pos) = true;
    max.at(pos) = std::max(max.at(pos), steps);

    const auto c = map.at(pos);
    if (c == '.' || c == '>') {
      dfs(input, pos + aoc::point::right(), steps + 1, visited, max);
    }
    if (c == '.' || c == 'v') {
      dfs(input, pos + aoc::point::down(), steps + 1, visited, max);
    }
    if (c == '.' || c == '<') {
      dfs(input, pos + aoc::point::left(), steps + 1, visited, max);
    }
    if (c == '.' || c == '^') {
      dfs(input, pos + aoc::point::up(), steps + 1, visited, max);
    }
    visited.at(pos) = false;
  }
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
    VisitedMap visited(r.map.get_width(), r.map.get_height());
    MaxMap max(r.map.get_width(), r.map.get_height());

    dfs(r, r.start, 0, visited, max);

    part1 = max.at(r.end);
  }

  int64_t part2{};
  {
    aoc::AutoTimer t2{"Part 2"};
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
