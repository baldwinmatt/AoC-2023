#include "aoc/helpers.h"
#include <map>
#include <queue>
#include <algorithm>
#include <numeric>

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(px{a<2006:qkq,m>2090:A,rfg}
pv{a>1716:R,A}
lnx{m>1548:A,A}
rfg{s<537:gd,x>2440:R,A}
qs{s>3448:A,lnx}
qkq{x<1416:A,crn}
crn{x>2662:A,R}
in{s<1351:px,qqz}
qqz{s>2770:qs,m<1801:hdj,R}
gd{a>3333:R,R}
hdj{m>838:A,pv}

{x=787,m=2655,a=1222,s=2876}
{x=1679,m=44,a=2067,s=496}
{x=2036,m=264,a=79,s=2244}
{x=2461,m=1339,a=466,s=291}
{x=2127,m=1623,a=2188,s=1013})");
  constexpr int64_t SR_Part1 = 19114;
  constexpr int64_t SR_Part2 = 167409079868000ll;

  enum class Condition {
    None,
    LessThan,
    GreaterThan,
  };

  struct SubRule {
    char c;
    Condition op;
    int rhs;
    std::string target;

    SubRule(std::string_view target)
      : c('-')
      , op(Condition::None)
      , rhs(0)
      , target{target.data(), target.size()}
    { }
    
    SubRule()
      : SubRule(std::string_view{})
    { }

    constexpr bool test(int lhs) const {
      switch (op) {
        case Condition::None:
          return true;
        case Condition::LessThan:
          return lhs < rhs;
        case Condition::GreaterThan:
          return lhs > rhs;
      }
      throw std::runtime_error("Bad op");
    }
  };


  struct Rule {
    std::vector<SubRule> subrules;
  };

  struct Part {
    int x;
    int m;
    int a;
    int s;
  };

  const auto GetPartValue = [](const Part& p, char c) {
    switch (c) {
      case 'x':
        return p.x;
      case 'm':
        return p.m;
      case 'a':
        return p.a;
      case 's':
        return p.s;
      case '-':
        return 0;
    }
    throw std::runtime_error("Bad part");
  };

  const auto ApplySubRule = [](const SubRule& sr, const Part& lhs) {
    const auto val = GetPartValue(lhs, sr.c);
    if (sr.test(val)) {
      return std::string_view(sr.target);
    }
    return std::string_view{};
  };

  const auto ApplyRule = [](const Rule& rule, const Part& lhs) {
    for (auto sr = rule.subrules.rbegin(); sr != rule.subrules.rend(); ++sr) {
      const auto target = ApplySubRule(*sr, lhs);
      if (!target.empty()) {
        return target;
      }
    }
    throw std::runtime_error("No target");
  };

  using RulesMap = std::map<std::string, Rule, std::less<>>;
  using PartList = std::vector<Part>;

  struct Range {
    int min;
    int max;
  };

  using PropertyRangeMap = std::map<char, Range>;
  using RuleRange = std::pair<std::string_view, PropertyRangeMap>;
  using RuleQueue = std::queue<RuleRange>;

  struct Input {
    RulesMap rules;
    PartList parts;
  };

  const auto LoadInput = [](auto f) {
    Input r;
    std::string_view line;
    while (aoc::getline(f, line, aoc::eol_delims, true)) {
      if (line.empty()) break;

      Rule rule;
      auto it = line.find('{');
      const auto name = line.substr(0, it);
      line.remove_prefix(it + 1);
      line.remove_suffix(1);

      auto parts = aoc::split(line, ",");
      SubRule def{parts.back()};
      parts.pop_back();
      rule.subrules.push_back(def);
      while (!parts.empty()) {
        auto& part = parts.back();
        rule.subrules.push_back({});
        auto& subrule = rule.subrules.back();
        subrule.c = part.front();
        part.remove_prefix(1);
        if (part.front() == '<') {
          subrule.op = Condition::LessThan;
        } else if (part.front() == '>') {
          subrule.op = Condition::GreaterThan;
        } else {
          throw std::runtime_error("Invalid subrule");
        }

        part.remove_prefix(1);
        auto pos = part.find(':');
        subrule.rhs = aoc::stoi(part.substr(0, pos));
        part.remove_prefix(pos + 1);
        subrule.target = part;
        parts.pop_back();
      }
      r.rules.emplace(name, std::move(rule));
    }

    while(aoc::getline(f, line)) {
      r.parts.push_back({});
      auto& p = r.parts.back();
      line.remove_prefix(1);
      line.remove_suffix(1);
      auto parts = aoc::split(line, ",");
      while (!parts.empty()) {
        auto &part = parts.back();

        char c = part[0];
        part.remove_prefix(2);

        auto val = aoc::stoi(part);

        switch (c) {
          case 'x': p.x = val; break;
          case 'm': p.m = val; break;
          case 'a': p.a = val; break;
          case 's': p.s = val; break;
          default: throw std::runtime_error("Invalid part");
        }

        parts.pop_back();
      }
    }
    return r;
  };

  int64_t CheckRule(const RulesMap& rules, std::string_view name, PropertyRangeMap& m) {
    if (name == "R") {
      return 0;
    }
    else if (name == "A") {
      return std::accumulate(m.begin(), m.end(), 1ll, [](int64_t acc, const auto& p)
        {
          DEBUG_PRINT("A " << p.first << " " << p.second.min << " " << p.second.max);
          return acc * (p.second.max - p.second.min + 1);
        });
    }

    const auto it = rules.find(name);
    if (it == rules.end()) {
      throw std::runtime_error("Invalid rule " + std::string(name));
    }

    int64_t sum = 0;

    for (const auto& sr : it->second.subrules) {
      if (sr.op == Condition::None) {
        sum += CheckRule(rules, sr.target, m);
        continue;
      }

      auto range_it = m.find(sr.c);
      if (range_it == m.end()) {
        throw std::runtime_error("Invalid property");
      }

      switch (sr.op) {
        case Condition::LessThan:
          if (range_it->second.max < sr.rhs) {
            sum += CheckRule(rules, sr.target, m);
            return sum;
          } else if (range_it->second.min < sr.rhs) {
            // split it
            auto new_ranges = m;
            new_ranges[sr.c].max = sr.rhs - 1;
            m[sr.c].min = sr.rhs;
            DEBUG_PRINT("Split " << sr.target << "  at " << sr.rhs << "[ " << new_ranges[sr.c].min << ", " << new_ranges[sr.c].max << " ]"
              << " [" << m[sr.c].min << ", " << m[sr.c].max << " ]");
            sum += CheckRule(rules, sr.target, new_ranges);
          }
          break;
        case Condition::GreaterThan:
          if (range_it->second.min > sr.rhs) {
            sum += CheckRule(rules, sr.target, m);
            return sum;
          } else if (range_it->second.max > sr.rhs) {
            // split it
            auto new_ranges = m;
            new_ranges[sr.c].min = sr.rhs + 1;
            sum += CheckRule(rules, sr.target, new_ranges);
            m[sr.c].max = sr.rhs;
          }
          break;

        default:
          throw std::runtime_error("Invalid condition");
    }
    }

    return sum;
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

  int64_t part1{};
  {
    aoc::AutoTimer t1{"Part 1"};
    int idx = 0;
    for (const auto& p : r.parts) {
      std::string_view cur = "in";

      std::vector<std::string_view>path;

      while (cur != "R" && cur != "A") {
        const auto it = r.rules.find(cur);
        if (it == r.rules.end()) {
          throw std::runtime_error("Invalid rule");
        }
        path.emplace_back(ApplyRule(it->second, p));
        cur = path.back();
      }

      DEBUG(
        std::cout << idx << ": " << p.x << " " << p.m << " " << p.a << " " << p.s;
        for (const auto& s : path) {
            std::cout << " -> " << s;
        }
        std::cout << std::endl;
      );

      if (cur == "A") {
        int64_t sum = p.x;
        sum += p.m;
        sum += p.a;
        sum += p.s;
        part1 += sum;
      }

      idx++;
    }
  }

  int64_t part2 = 0;

  {
    aoc::AutoTimer t2{"Part 2"};
    constexpr int max = 4000;
    PropertyRangeMap m{
      { 'x', {1, max} },
      { 'm', {1, max} },
      { 'a', {1, max} },
      { 's', {1, max} }
    };

    part2 = CheckRule(r.rules, "in", m);

    /*
    RuleQueue q;
    q.push({"in", m});
    while (!q.empty()) {
      const auto& [loc, ranges] = q.front();
      if (GetPropertyMapProduct(ranges) == 0 ||
        loc == "R") {
        q.pop();
        DEBUG_PRINT("Queue size: " << q.size());
        continue;
      } else if (loc == "A") {
        part2 += GetPropertyMapProduct(ranges);
        q.pop();
        DEBUG_PRINT("Queue size: " << q.size());
        continue;
      }
      const auto it = r.rules.find(loc);
      if (it == r.rules.end()) {
        throw std::runtime_error("Invalid rule");
      }

      auto new_ranges = ranges;
      for (const auto& sr : it->second.subrules) {
        auto& range = new_ranges[sr.c];
        bool done = false;
        switch (sr.op) {
          case Condition::LessThan:
            if (sr.rhs < range.min) {
              continue;
            } else if (sr.rhs < range.max) {
              DEBUG_PRINT("Splitting " << range.min << " " << range.max << " by " << sr.rhs);
              // split the range into two
              auto new_range = range;
              new_range.max = sr.rhs;
              auto split_ranges = new_ranges;
              auto it = split_ranges.emplace(sr.c, Range{0, 0});
              it.first->second = new_range;
              q.push({sr.target, split_ranges});
              range.min = sr.rhs;
            } else {
              q.push({sr.target, new_ranges});
              done = true;
            }
            break;
          case Condition::GreaterThan:
            if (sr.rhs >= range.max) {
              continue;
            } else if (sr.rhs >= range.min) {
              DEBUG_PRINT("Splitting " << range.min << " " << range.max << " by " << sr.rhs);
              // split the range into two
              auto new_range = range;
              new_range.min = sr.rhs + 1;
              auto split_ranges = new_ranges;
              auto it = split_ranges.emplace(sr.c, Range{0, 0});
              it.first->second = new_range;
              q.push({sr.target, split_ranges});
              range.max = sr.rhs + 1;
            } else {
              q.push({sr.target, new_ranges});
              done = true;
            }
            break;
          case Condition::None:
            q.push({sr.target, new_ranges});
            break;
          default:
            throw std::runtime_error("Bad condition");
        }

        if (done) { break; }
      }
      DEBUG_PRINT("Done with " << loc);
      q.pop();
      DEBUG_PRINT("Queue size: " << q.size());
    }

    */
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
