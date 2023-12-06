#include "aoc/helpers.h"

#include <cmath>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(Time:      7  15   30
Distance:  9  40  200)");
  constexpr int64_t SR_Part1 = 288;
  constexpr int64_t SR_Part2 = 71503;

  struct RaceRecord {
    int64_t time;
    int64_t distance{0};

    RaceRecord(int t) : time(t) { }
  };

  using RaceRecords = std::vector<RaceRecord>;

  const auto LoadInput = [](auto f) {
    RaceRecords r;
    std::string_view line;

    // first line is the time
    aoc::getline(f, line);
    // skip Time:
    auto it = line.find(':');
    line.remove_prefix(it + 1);
    // eat whitespace
    while (line.front() == ' ') {
      line.remove_prefix(1);
    }
    aoc::parse_as_integers(line, ' ', [&r](const auto &n) {
      r.emplace_back(n);
    });

    // second line is the distance
    aoc::getline(f, line);
    // skip Distance:
    it = line.find(':');
    line.remove_prefix(it + 1);
    // eat whitespace
    while (line.front() == ' ') {
      line.remove_prefix(1);
    }
    size_t i = 0;
    aoc::parse_as_integers(line, ' ', [&r, &i](const auto &n) {
      r[i++].distance = n;
    });
    return r;
  };

  // Solve for a * x^2 + b * x + c = 0
  const auto SolveQuadtraic = [](double a, double b, double c) -> std::pair<double, double> {
    const auto d = (b * b - 4.0 * a * c);
    assert(d >= 0.0);
    const auto sqrt_d = std::sqrt(d);
    const auto x1 = (-b + sqrt_d) / (2.0 * a);
    const auto x2 = (-b - sqrt_d) / (2.0 * a);
    return {std::ceil(x1), std::ceil(x2)};
  };

  const auto CountWaysToWin = [](const auto &rr) {
    const auto res = SolveQuadtraic(-1, rr.time, -rr.distance);
    const auto start = res.first;
    const auto end = res.second;
    return end - start;
  };

  const auto CountWaysToWinSimple = [](const auto &rr) {
    // for each ms we hold the button, the speed will increase by 1 ms per ms
    // holding the button down counts towards race time
    // distance is (race time) - (hold time) * (hold time)

    // find the range for which (rr.time - ms) * ms is greater than rr.distance
    int64_t start = 0;
    for (int64_t ms = 1; !start && ms < rr.time; ms++) {
      const auto distance = (rr.time - ms) * ms;
      if (distance > rr.distance) {
        start = ms;
      }
    }

    int64_t end = 0;
    for (int64_t ms = rr.time - 1; !end && ms > 0; ms--) {
      const auto distance = (rr.time - ms) * ms;
      if (distance > rr.distance) {
        end = ms;
      }
    }

    // for everything from start to end, we would win, so number of ways is
    return end - start + 1;
  };

  const auto CountDigits = [](int64_t n) {
    if (n == 0) { return 1; }

    int count = 0;
    while (n) {
      count++;
      n /= 10;
    }
    return count;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  RaceRecords r;
  if (inTest) {
    r = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f);
  }

  int64_t part1 = 0;
  int64_t part2 = 0;

  // Part 1, count the ways to win for each race and multiply them together
  {
    aoc::AutoTimer t{"Part 1"};
    for (const auto &rr : r) {
      const auto ways_to_win = CountWaysToWinSimple(rr);
      if (!part1) { part1 = ways_to_win; }
      else { part1 *= ways_to_win; }
    }
  }

  // Part 2, the input had bad kerning, and its one race, so combine the numbers
  {
    aoc::AutoTimer t{"Part 2"};
    RaceRecord rr{0};
    for (const auto &i : r) {
      const auto time_width = CountDigits(i.time);
      const auto distance_width  = CountDigits(i.distance);
      rr.time = rr.time * std::pow(10, time_width) + i.time;
      rr.distance = rr.distance * std::pow(10, distance_width) + i.distance;
    }
    part2 = CountWaysToWin(rr);
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
