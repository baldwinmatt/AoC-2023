#include "aoc/helpers.h"
#include "aoc/point.h"

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(R 6 (#70c710)
D 5 (#0dc571)
L 2 (#5713f0)
D 2 (#d2c081)
R 2 (#59c680)
D 2 (#411b91)
L 5 (#8ceee2)
U 2 (#caa173)
L 1 (#1b58a2)
U 2 (#caa171)
R 2 (#7807d2)
U 3 (#a77fa3)
L 2 (#015232)
U 2 (#7a21e3))");
  constexpr int64_t SR_Part1 = 62;
  constexpr int64_t SR_Part2 = 952408144115;

  using Points = std::vector<aoc::point>;
  using Points2 = std::vector<aoc::Point>;

  const auto LoadInput = [](auto f) {
    Points p;
    Points2 p2;
    std::string_view line;
    aoc::point pos = { 1, 1 };
    aoc::Point pos2 = { 1, 1 };
    p.emplace_back(pos);
    while (aoc::getline(f, line)) {
      const auto parts = aoc::split(line, " ");
      const auto dir = parts[0];
      const auto count = aoc::stoi(parts[1]);
      auto color = parts[2];
      color.remove_prefix(2);
      color.remove_suffix(1);

      switch (dir.front()) {
        case 'U': pos += aoc::point::up() * count; break;
        case 'D': pos += aoc::point::down() * count; break;
        case 'L': pos += aoc::point::left() * count; break;
        case 'R': pos += aoc::point::right() * count; break;
        default: throw std::runtime_error("invalid direction");
      }

      int32_t dist = aoc::char_hex_val(color.front());
      color.remove_prefix(1);
      dist = dist * 16 + aoc::char_hex_val(color.front());
      color.remove_prefix(1);
      dist = dist * 16 + aoc::char_hex_val(color.front());
      color.remove_prefix(1);
      dist = dist * 16 + aoc::char_hex_val(color.front());
      color.remove_prefix(1);
      dist = dist * 16 + aoc::char_hex_val(color.front());
      color.remove_prefix(1);
      DEBUG_PRINT("dist: " << dist << " dir: " << color.front());
      switch(color.front()) {
        case '0': pos2.first += dist; break;
        case '1': pos2.second += dist; break;
        case '2': pos2.first -= dist; break;
        case '3': pos2.second -= dist; break;
        default: throw std::runtime_error("invalid direction");
      }
      p.emplace_back(pos);
      p2.emplace_back(pos2);
    }
    return std::make_pair(p, p2);
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  std::pair<Points, Points2> p;
  if (inTest) {
    p = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    p = LoadInput(f);
  }

  int64_t part1{};
  
  {
    aoc::AutoTimer t1{"Part 1"};
    part1 = aoc::area(p.first.begin(), p.first.end(), aoc::PerimiterMode::Include);
  }

  int64_t part2 = 0;

  {
    aoc::AutoTimer t2{"Part 2"};
    auto start = p.second.begin();
    auto end = p.second.end();
    int64_t area{};
    int64_t perimeter{};
    while (start != end && (start + 1) != end) {
        auto& a = *start;
        auto& b = *(start + 1);
        area += a.first * b.second - a.second * b.first;
        perimeter += aoc::manhattan(a, b);
        start++;
    }
    area = (std::abs(area) + perimeter) / 2;

    part2 = area + 1;
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
