#include "aoc/helpers.h"

#include <set>
#include <map>
#include <vector>
#include <cmath>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(Card 1: 41 48 83 86 17 | 83 86  6 31 17  9 48 53
Card 2: 13 32 20 16 61 | 61 30 68 82 17 32 24 19
Card 3:  1 21 53 59 44 | 69 82 63 72 16 21 14  1
Card 4: 41 92 73 84 69 | 59 84 76 51 58  5 54 83
Card 5: 87 83 26 28 32 | 88 30 70 12 93 22 82 36
Card 6: 31 18 13 56 72 | 74 77 10 23 35 67 36 11)");
  constexpr int SR_Part1 = 13;
  constexpr int SR_Part2 = 30;

  const auto ParseLine = [](std::string_view line) -> int32_t {
    // eat the part up to the colon
    const auto n = line.find(':');
    line.remove_prefix(n + 1);
    while (line.front() == ' ') {
      line.remove_prefix(1);
    }

    // split at the '|', the left is the card, the right is the winning numbers
    const auto m = line.find('|');
    const auto card = line.substr(0, m);
    line.remove_prefix(m + 1);
    while (line.front() == ' ') {
      line.remove_prefix(1);
    }
    const auto winning = line;

    // convert the space seperated list into a set
    std::set<int64_t> card_set;
    aoc::parse_as_integers(card, ' ', [&card_set](const auto n) { card_set.insert(n); });

    // count the number of matches from winning numbers in the card
    int32_t count = 0;
    aoc::parse_as_integers(winning, ' ', [&count, &card_set](const auto n) {
      count += card_set.count(n);
    });

    return count;
  };

  const auto LoadInput = [](auto f) {
    Result r{0, 0};
    std::string_view line;
    std::vector<std::pair<int32_t, int32_t>>card_wins;
    int32_t idx = 1;
    while (aoc::getline(f, line)) {
      const auto n = ParseLine(line);
      if (n > 0) {
        // part 1 is trivial 2^(number of wins - 1)
        r.first += std::pow(2, n - 1);
      }
      // part 2 requires some state
      card_wins.push_back({idx++, n});
    }

    // determine, for each card, how many we win of the next cards in the set
    std::map<int32_t, int32_t> extra_copies;
    for (const auto& [i, n] : card_wins) {
      const auto idx = extra_copies.find(i);
      const int32_t current_num = (idx == extra_copies.end() ? 0 : idx->second) + 1;
      // now increment number of the next cards by the number of the current card, plus this instance
      for (int32_t j = 1; j < n + 1; ++j) {
        auto it = extra_copies.emplace(i + j, 0);
        it.first->second += current_num;
      }
    }

    // part 2 result is total number of extra cards we won
    std::for_each(extra_copies.begin(), extra_copies.end(), [&r](auto& p) {
      r.second += p.second;
    });
    // plus the number of cards we started with
    r.second += card_wins.size();

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
