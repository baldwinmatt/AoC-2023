#include "aoc/helpers.h"

#include <map>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(LLR

AAA = (BBB, BBB)
BBB = (AAA, ZZZ)
ZZZ = (ZZZ, ZZZ))");
  constexpr std::string_view SampleInput2(R"(LR

11A = (11B, XXX)
11B = (XXX, 11Z)
11Z = (11B, XXX)
22A = (22B, XXX)
22B = (22C, 22C)
22C = (22Z, 22Z)
22Z = (22B, 22B)
XXX = (XXX, XXX))");

  constexpr int SR_Part1 = 6;
  constexpr int SR_Part2 = 6;

  struct Node {
    struct Node *L{nullptr};
    struct Node *R{nullptr};
    bool is_end{false};
    bool is_terminal{false};
  };

  using NodeMap = std::map<std::string, Node>;

  struct Input {
    std::string pattern;
    NodeMap map;
    Node *start{nullptr};
    std::vector<Node *>sources;
  };
  
  const auto LoadInput = [](auto f) {
    std::string_view line;

    // Pattern is the first line
    aoc::getline(f, line);
    Input r;
    r.pattern = std::string{line.data(), line.size()};

    // Load the map
    while (aoc::getline(f, line)) {
      auto p = line.find(' ');
      std::string_view source = line.substr(0, p);
      line.remove_prefix(p + 1);
      while (line.front() == ' ' || line.front() == '(' || line.front() == '=') { line.remove_prefix(1); }
      p = line.find(',');
      std::string_view left = line.substr(0, p);
      line.remove_prefix(p + 1);
      while (line.front() == ' ') { line.remove_prefix(1); }
      p = line.find(')');
      std::string_view right = line.substr(0, p);

      // create, or locate the nodes in the map
      auto sit = r.map.emplace(source, Node{});
      auto lit = r.map.emplace(left, Node{});
      auto rit = r.map.emplace(right, Node{});

      auto *source_node = &sit.first->second;

      source_node->L = &lit.first->second;
      source_node->R = &rit.first->second;
      if (source == "ZZZ") {
        source_node->is_end = true;
      } else if (source == "AAA") {
        r.start = source_node;
      }
      source_node->is_terminal = source.back() == 'Z';
      if (source.back() == 'A') {
        r.sources.push_back(source_node);
      }
    }
    return r;
  };

  int64_t FindLeastCommonMultiple(std::vector<int64_t> v, size_t idx) {
    if (idx == v.size() - 1) {
      return v[idx];
    }
    const auto a = v[idx];
    const auto b = FindLeastCommonMultiple(v, idx + 1);
    return (a * b / std::__gcd(a, b));
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

  int64_t part1 = 0;
  int64_t part2 = 0;

  {
    aoc::AutoTimer t{"Part 1"};

    auto *p = r.start;
    while (!p->is_end) {
      const auto c = r.pattern.data()[part1 % r.pattern.size()];
      part1++;
      if (c == 'L') {
        p = p->L;
      } else if (c == 'R') {
        p = p->R;
      }
    }
  }

  {
    aoc::AutoTimer t{"Part 2"};
    std::vector<int64_t> distances;
    for (auto *p : r.sources) {
      int64_t d = 0;
      while (!p->is_terminal) {
        const auto c = r.pattern.data()[d % r.pattern.size()];
        d++;
        if (c == 'L') {
          p = p->L;
        } else if (c == 'R') {
          p = p->R;
        }
      }
      distances.push_back(d);
    }
    part2 = FindLeastCommonMultiple(distances, 0);
  }

  if (inTest) {
    r = LoadInput(SampleInput2);
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
