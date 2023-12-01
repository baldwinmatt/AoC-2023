#include "aoc/helpers.h"

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"()");
  constexpr int SR_Part1 = 0;
  constexpr int SR_Part2 = 0;

  const auto ProcessLine = [](std::string_view line) {
    int line_val = 0;
    while (!line.empty() && !aoc::is_numeric(line.front())) {
      line.remove_prefix(1);
    }

    if (line.empty()) {
      return 0;
    }

    line_val = aoc::stoi({&(line.front()), 1});

    while (!line.empty() && !aoc::is_numeric(line.back())) {
      line.remove_suffix(1);
    }

    if (line.empty()) {
      return 0;
    }

    line_val *= 10;
    line_val += aoc::stoi({&(line.back()), 1});

    return line_val;
  };

  const auto LoadInput = [](auto f) {
    Result r{0, 0};
    std::string_view line;
    while (aoc::getline(f, line)) {
      r.first += ProcessLine(line);
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
