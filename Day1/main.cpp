#include "aoc/helpers.h"

#include <array>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(two1nine
eightwothree
abcone2threexyz
xtwone3four
4nineeightseven2
zoneight234
7pqrstsixteen
)");
  constexpr int SR_Part1 = 209;
  constexpr int SR_Part2 = 281;

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

    DEBUG_PRINT("line: " << line << " output: " << line_val);

    return line_val;
  };

  constexpr std::string_view ONE{"one"};
  constexpr std::string_view TWO{"two"};
  constexpr std::string_view THREE{"three"};
  constexpr std::string_view FOUR{"four"};
  constexpr std::string_view FIVE{"five"};
  constexpr std::string_view SIX{"six"};
  constexpr std::string_view SEVEN{"seven"};
  constexpr std::string_view EIGHT{"eight"};
  constexpr std::string_view NINE{"nine"};

  constexpr std::array<std::string_view, 9> NUMBERS{ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};

  const auto PreProcessLine = [](std::string_view line) {
    std::string out{line.data(), line.size()};
    std::string_view out_view{out.data(), out.size()};

    size_t i = 0;
    while (i < out.size()) {
      for (size_t j = 0; j < NUMBERS.size(); j++) {
        const auto& num = NUMBERS[j];
        if (aoc::starts_with(out_view, num)) {
          out[i] = j + '1';
          break;
        }
      }
      out_view.remove_prefix(1);
      i+=1;
    }

    DEBUG_PRINT("input: " << line << " output: " << out);

    return out;
  };

  const auto LoadInput = [](auto f) {
    Result r{0, 0};
    std::string_view line;
    while (aoc::getline(f, line)) {
      r.first += ProcessLine(line);
      r.second += ProcessLine(PreProcessLine(line));
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
