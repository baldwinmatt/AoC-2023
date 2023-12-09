#include "aoc/helpers.h"

#include <vector>
#include <functional>
#include <numeric>
#include <ranges>
#include <memory>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(0 3 6 9 12 15
1 3 6 10 15 21
10 13 16 21 30 45)");
  constexpr int SR_Part1 = 114;
  constexpr int SR_Part2 = 2;

  using Sequence = std::vector<int64_t>;
  using Sequences = std::vector<Sequence>;

  const auto LoadInput = [](auto f) {
    Sequences s;
    std::string_view line;
    while (aoc::getline(f, line)) {
      s.emplace_back();
      aoc::parse_as_integers(line, ' ', [&s](int64_t i) { s.back().push_back(i); });
    }
    return s;
  };

  const auto Extrapolate = [](Sequence s, bool reverse) {
    int64_t c{1};
    int64_t v{0};

    if (reverse) {
      std::reverse(s.begin(), s.end());
    }
    while (std::any_of(s.begin(), s.end() - c, std::identity{})) {
      std::adjacent_difference(s.begin() + 1, s.end() - c + 1, s.begin());
      v += *(s.end() - c);
      ++c;
    }

    return v;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Sequences seq;
  if (inTest) {
    seq = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    seq = LoadInput(f);
  }

  auto fwd = [&](const auto& q){ return Extrapolate(q, false); };
  auto rev = [&](const auto& q){ return Extrapolate(q, true); }; 

  int64_t part1 = 0;
  {
    aoc::AutoTimer t1{"Part 1"};
    part1 = std::transform_reduce(seq.begin(), seq.end(), 0, std::plus<>(), fwd);
  }
  int64_t part2 = 0;
  {
    aoc::AutoTimer t2{"Part 2"};
    part2 = std::transform_reduce(seq.begin(), seq.end(), 0, std::plus<>(), rev);
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
