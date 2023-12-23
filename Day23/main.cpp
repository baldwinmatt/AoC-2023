#include "aoc/helpers.h"
#include "aoc/padded_matrix.h"

#include <queue>
#include <unordered_set>
#include <unordered_map>

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
  constexpr int SR_Part2 = 154;

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

  using Intersections = std::unordered_set<aoc::point, aoc::point_hash>;
  using NodeLengthMap = std::unordered_map<aoc::point, int32_t, aoc::point_hash>;
  using Edges = std::unordered_map<aoc::point, NodeLengthMap, aoc::point_hash>;

  void dfs2(const Intersections& nodes, const Edges& edges, const aoc::point pos, int64_t steps, VisitedMap& visited, MaxMap& max) {
    if (visited.at(pos)) {
      return;
    }
    visited.at(pos) = true;
    max.at(pos) = std::max(max.at(pos), steps);

    const auto it = edges.find(pos);
    
    if (it != edges.end()) {
      const auto& next_nodes = it->second;
      for (const auto& node: next_nodes) {
        dfs2(nodes, edges, node.first, steps + node.second, visited, max);
      }
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

    constexpr static const aoc::point dirs[] =
    {
      aoc::point::right(),
      aoc::point::down(),
      aoc::point::left(),
      aoc::point::up(),
    };
    constexpr static const size_t ndirs = std::size(dirs);

    Intersections intersections;
    for (int32_t x = 0; x < static_cast<int32_t>(r.map.get_width()); ++x) {
      for (int32_t y = 0; y < static_cast<int32_t>(r.map.get_height()); ++y) {
        if (r.map.at(x, y) == '#') {
          continue;
        }

        // count number of ways we can step from here
        std::vector<aoc::point> exits;
        for (size_t i = 0; i < ndirs; ++i) {
          const auto p = aoc::point{x, y} + dirs[i];
          if (r.map.at(p) != '#') {
            exits.push_back(p);
          }
        }

        if (exits.size() > 2) {
          intersections.emplace(x, y);
          DEBUG_PRINT("intersection at " << aoc::point(x, y));
        }
      }
    }
    intersections.emplace(r.start);
    intersections.emplace(r.end);

    const auto find_length = [&](const aoc::point& p, const aoc::point dir) -> std::pair<aoc::point, int32_t> {
      std::unordered_set<aoc::point, aoc::point_hash> visited;
      visited.insert(p);

      int32_t length = 0;
      auto pos = p + dir;
      if (r.map.at(pos) == '#') {
        return std::make_pair(pos, 0);
      }
      visited.insert(pos);

      while (intersections.find(pos) == intersections.end()) {
        ++length;

        for (const auto d : dirs) {
          auto next = pos + d;
          if (r.map.at(next) != '#') {
            const auto it = visited.emplace(next);
            if (it.second == false) {
              continue;
            }
            pos = next;
            break;
          }
        }
      }

      return std::make_pair(pos, length + 1);
    };

    // now for each interseciton, calculate the edges
    Edges edges;
    for (const auto& p: intersections) {
      auto it = edges.emplace(p, NodeLengthMap{});
      auto& edge = it.first->second;
      for (size_t i = 0; i < ndirs; ++i) {
        const auto q = find_length(p, dirs[i]);
        if (q.second > 1) {
          edge.emplace(q.first, q.second);
          DEBUG_PRINT("from " << p << " to " << q.first << " length " << q.second);
        }
      }
    }

    VisitedMap visited(r.map.get_width(), r.map.get_height());
    MaxMap max(r.map.get_width(), r.map.get_height());
    dfs2(intersections, edges, r.start, 0, visited, max);
    part2 = max.at(r.end);
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
