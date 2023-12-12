#include "aoc/helpers.h"

#include <algorithm>
#include <numeric>

namespace {
  using Result = std::pair<int64_t, int64_t >;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(???.### 1,1,3
.??..??...?##. 1,1,3
?#?#?#?#?#?#?#? 1,3,1,6
????.#...#... 4,1,1
????.######..#####. 1,6,5
?###???????? 3,2,1)");
  constexpr int64_t SR_Part1 = 21;
  constexpr int64_t SR_Part2 = 525152;

  using SpringConfig = std::pair<std::string_view, std::vector<int64_t>>;

  const auto GetGroupEnds = [](int64_t hashes, const std::vector<int64_t>& groups) {
    std::vector<bool> group_ends(hashes + 1);
    group_ends[0] = true;

    int64_t group_end = 0;
    for (const auto& g : groups) {
      group_end += g;
      group_ends[group_end] = true;
    }

    return group_ends;
  };

  const auto CountPermutations = [](const SpringConfig& s) {
    const size_t total_hashes = std::accumulate(s.second.begin(), s.second.end(), 0);

    const auto group_ends = GetGroupEnds(total_hashes, s.second);
    constexpr int64_t max_hashes = 100;
    enum {
      DOT = 0,
      HASH = 1,
    };
    auto cache = new int64_t[s.first.size() + 1][max_hashes][2];
    for (size_t i = 0; i < s.first.size() + 1; ++i) {
      for (size_t j = 0; j < max_hashes; ++j) {
        cache[i][j][DOT] = 0;
        cache[i][j][HASH] = 0;
      }
    }
    cache[0][0][DOT] = 1;

    for (size_t i = 1; i <= s.first.size(); ++i) {
      const auto c = s.first[i - 1];

      auto& current = cache[i];
      auto& prev = cache[i - 1];

      if (c != '.') {
        // Could be a hash
        for (size_t placed = 1; placed <= std::min(i, total_hashes); placed++) {
          if (group_ends[placed - 1]) {
            // if a new group, must follow a hash
            current[placed][HASH] = prev[placed - 1][DOT];
          } else {
            // else, it should be a hash
            current[placed][HASH] = prev[placed - 1][HASH];
          }
        }
      }
      if (c != '#') {
        // Could be a dot
        for (size_t placed = 0; placed <= std::min(i - 1, total_hashes); placed++) {
          if (group_ends[placed]) {
            // if end of a group, we must place a dot
            current[placed][DOT] = prev[placed][HASH] + prev[placed][DOT];
          }
        }
      }
    }

    // number of possibilities is the sum of the lats element
    const int64_t r = cache[s.first.size()][total_hashes][DOT] + cache[s.first.size()][total_hashes][HASH];
    delete [] cache;
    return r;
  };

  const auto LoadInput = [](auto f) {
    Result r;
    std::string_view line;
    while (aoc::getline(f, line)) {
      SpringConfig s;
      const auto it = line.find(' ');
      s.first = line.substr(0, it);
      const auto rest = line.substr(it + 1);
      aoc::parse_as_integers(rest, ',', [&s](const auto n) { s.second.push_back(n); } );

      r.first += CountPermutations(s);

      // unfolod the input
      std::string unfolded{s.first.data(), s.first.size()};
      SpringConfig s2;
      s2.second = s.second;
      for (auto i = 0; i < 4; i++) {
        unfolded.append(1, '?')
          .append(s.first.data(), s.first.size());
        s2.second.insert(s2.second.end(), s.second.begin(), s.second.end());
      }
      s2.first = { unfolded.data(), unfolded.size() };
      r.second += CountPermutations(s2);
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

  int64_t part1 = 0;
  int64_t part2 = 0;

  std::tie(part1, part2) = r;

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
