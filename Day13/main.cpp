#include "aoc/helpers.h"

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(#.##..##.
..#.##.#.
##......#
##......#
..#.##.#.
..##..##.
#.#.##.#.

#...##..#
#....#..#
..##..###
#####.##.
#####.##.
..##..###
#....#..#)");
  constexpr int SR_Part1 = 405;
  constexpr int SR_Part2 = 400;

  using Pattern = std::vector<std::string>;

  const auto Transpose = [](const Pattern& p) {
    Pattern r;
    r.reserve(p[0].size());
    for (size_t i = 0; i < p[0].size(); ++i) {
      std::string s;
      s.reserve(p.size());
      for (const auto& l : p) {
        s.push_back(l[i]);
      }
      r.push_back(s);
    }
    return r;
  };

  const auto CountMissmatches = [](const std::string_view p1, const std::string_view p2) {
    int64_t m = 0;
    for (size_t i = 0; i < p1.size(); ++i) {
      m += (p1[i] != p2[i]);
    }
    return m;
  };

  const auto ScoreImpl = [](const Pattern& p, int64_t idx, int64_t base, int64_t mistakes) {
    auto i = idx;
    int64_t di = 1;
    while (i >= 0 && i + di < static_cast<int64_t>(p.size()) && mistakes >= 0) {
      // fold vertically
      std::string_view u{p[i]};
      std::string_view v{p[i + di]};

      const auto m = CountMissmatches(u, v);

      i -= 1;
      di += 2;
      mistakes -= m;
    }

    if (mistakes != 0) {
      return int64_t{0};
    }

    DEBUG_PRINT("Pattern matched at " << idx);

    DEBUG(
      for (size_t x = 0; x < p.size(); ++x) {
        DEBUG_PRINT(x << ": " << p[x]);
      }
    );

    // if we matched, return the score as base * (idx + 1)
    return base * (idx + 1);
  };

  const auto ScorePattern = [](const Pattern& p, int64_t base, int64_t misstake = 0) {
    int64_t sum = 0;
    for (int64_t i = 0; i < static_cast<int64_t>(p.size()) - 1; ++i) {
      const auto m = CountMissmatches(p[i], p[i + 1]);
      if (m <= misstake) {
        DEBUG_PRINT("Checking at " << i);
        sum += ScoreImpl(p, i, base, misstake);
      }
    }
    return sum;
  };

  const auto Score = [](const Pattern& p) {
    const auto n = ScorePattern(p, 100);
    if (n) { return n; }
    return ScorePattern(Transpose(p), 1);
  };

  const auto ScoreMisstake = [](const Pattern& p) {
    const auto n = ScorePattern(p, 100, 1);
    if (n) { return n; }
    return ScorePattern(Transpose(p), 1, 1);
  };

  const auto LoadInput = [](auto f) {
    Result r{0, 0};
    std::string_view line;
    Pattern p;
    while (aoc::getline(f, line, aoc::eol_delims, true)) {
      if (line.empty()) {

        r.first += Score(p);
        r.second += ScoreMisstake(p);

        p.clear();
        continue;
      }

      p.emplace_back(line.data(), line.size());
    }

    if (!p.empty()) {
      r.first += Score(p);
      r.second += ScoreMisstake(p);
    }
    return r;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Result r;
  if (inTest) {
    r = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f);
  }

  int part1 = 0;
  int part2 = 0;

  std::tie(part1, part2) = r;

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
