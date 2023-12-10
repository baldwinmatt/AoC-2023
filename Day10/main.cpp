#include "aoc/helpers.h"

#include <vector>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(FF7FSF7F7F7F7F7F---7
L|LJ||||||||||||F--J
FL-7LJLJ||||||LJL-77
F--JF--7||LJLJ7F7FJ-
L---JF-JLJ.||-FJLJJ7
|F|F-JF---7F7-L7L|7|
|FFJF7L7F-JF7|JL---7
7-L-JL7||F7|L7F-7F7|
L.L7LFJ|||||FJL7||LJ
L7JLJL-JLJLJL--JLJ.L
)");
  constexpr size_t SR_Part1 = 80;
  constexpr size_t SR_Part2 = 10;

  enum Direction
  {
    None = 0,
    Up = 1,
    Down = 2,
    Left = 4,
    Right = 8,
  };

  const auto GetDirection = [](char c) {
    switch (c) {
      case '|': return Direction::Up | Direction::Down;
      case '-': return Direction::Left | Direction::Right;
      case 'L': return Direction::Up | Direction::Right;
      case 'J': return Direction::Up | Direction::Left;
      case 'F': return Direction::Down | Direction::Right;
      case '7': return Direction::Down | Direction::Left;
      case '.': return static_cast<int>(Direction::None);
      case '#': return static_cast<int>(Direction::None);
      case 'S': return Direction::Up | Direction::Down | Direction::Left | Direction::Right;
    }
    throw std::runtime_error("Unknown direction");
  };

  using Path = std::vector<aoc::Point>;

  const auto LoadInput = [](auto f) {
    std::string_view line;

    aoc::Point p{ -1, -1 };
    std::vector<std::string> data;
    while (aoc::getline(f, line)) {
      if (p.first == -1) {
        const auto i = line.find('S');
        if (i != std::string::npos) {
          p.first = i;
          p.second = data.size();

          DEBUG_PRINT("Found Start at " << p);
        }
      }
      data.emplace_back(line.data(), line.size());
    }

    aoc::Point limit{ data[0].size() - 1, data.size() - 1 };

    Path path;
    while (1) {
      path.push_back(p);
      auto& c = data[p.second][p.first];
      auto d = GetDirection(c);
      // the pipes must be connected, find one to move to
      if (p.first < limit.first && (d & Direction::Right) && GetDirection(data[p.second][p.first + 1]) & Direction::Left) {
        p.first++;
      } else if (p.second < limit.second && (d & Direction::Down) && GetDirection(data[p.second + 1][p.first]) & Direction::Up) {
        p.second++;
      } else if (p.first > 0 && (d & Direction::Left) && GetDirection(data[p.second][p.first - 1]) & Direction::Right) {
        p.first--;
      } else if (p.second > 0 && (d & Direction::Up) && GetDirection(data[p.second - 1][p.first]) & Direction::Down) {
        p.second--;
      } else {
        break;
      }
      c = '#';
      DEBUG_PRINT("Moved to " << p);
    }

    return path;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Path path;
  if (inTest) {
    path = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    path = LoadInput(f);
  }

  const auto part1{path.size() / 2};
  size_t part2{};

  path.push_back(path.front());

  // calculate the area of the enclosed path.
  size_t pos{};
  size_t next{1};
  int64_t area{};
  while (next < path.size()) {
    auto& a = path[pos];
    auto& b = path[next];
    area += a.first * b.second - a.second * b.first;

    pos++;
    next++;
  }
  // subtract the circumference (which is path.size() - 1, as we pushed the start to close the loop)
  area = (std::abs(area) - (path.size() - 1)) / 2;
  part2 = area + 1;

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
