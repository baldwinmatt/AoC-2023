#include "aoc/helpers.h"
#include <array>
#include <deque>
#include <map>

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(broadcaster -> a
%a -> inv, con
&inv -> b
%b -> con
&con -> output)");
  constexpr int64_t SR_Part1 = 11687500;
  constexpr int64_t SR_Part2 = 0;

  enum class ModuleType {
    None,
    FlipFlop,
    Conjunction,
  };

  enum class Signal {
    Low,
    High,
  };

  struct Emission {
    std::string_view source;
    Signal signal;
    std::string_view destination;
  };

  using OutputVector = std::vector<std::string>;
  using SignalMap = std::map<std::string_view, Signal, std::less<>>;
  using EmissionVector = std::vector<Emission>;

  struct Module {
    std::string id;
    ModuleType type;
    OutputVector outputs;
    SignalMap inputs;
    bool on;

    EmissionVector emit(Signal signal) const {
      EmissionVector r;

      std::transform(outputs.begin(), outputs.end(), std::back_inserter(r), [this, signal](auto& o) {
        //DEBUG_PRINT(id << " -> " << o << " (" << (int)signal << ")");
        return Emission{std::string_view{id.data(), id.size()}, signal, std::string_view{o.data(), o.size()}};
      });

      return r;
    }

    EmissionVector receive(Signal signal, std::string_view source) {
      switch (type) {
        case ModuleType::None:
          return emit(signal);
        case ModuleType::FlipFlop:
        {
          if (signal == Signal::High) {
            return {};
          }
          const auto to_emit = on ? Signal::Low : Signal::High;
          on = !on;
          return emit(to_emit);
        }
        case ModuleType::Conjunction:
        {
          auto it = inputs.find(source);
          assert(it != inputs.end());
          it->second = signal;

          const auto all_high = std::all_of(inputs.begin(), inputs.end(), [](auto& p) {
            return p.second == Signal::High;
          });

          return emit(all_high ? Signal::Low : Signal::High);
        }
      }
      throw std::runtime_error("Unreachable");
    }

    static Module Parse(std::string_view line) {
      Module m;
      const auto pos = line.find(" -> ");
      auto id = line.substr(0, pos);
      if (id.front() == '%') {
        m.type = ModuleType::FlipFlop;
        id.remove_prefix(1);
      } else if (id.front() == '&') {
        m.type = ModuleType::Conjunction;
        id.remove_prefix(1);
      } else {
        m.type = ModuleType::None;
      }
      m.id = std::string{id.data(), id.size()};
      line.remove_prefix(pos + 4);
      std::string_view output;
      while (aoc::getline(line, output, ", ")) {
        m.outputs.emplace_back(output.data(), output.size());
      }
      m.on = false;
    
      return m;
    }
  };

  using ModuleMap = std::map<std::string, Module, std::less<>>;

  const auto LoadInput = [](auto f) {
    ModuleMap map;
    std::string_view line;
    while (aoc::getline(f, line)) {
      Module m = Module::Parse(line);
      map.emplace(m.id, std::move(m));
    }

    for (auto& mit : map) {
      for (auto& o : mit.second.outputs) {
        auto it = map.find(o);
        if (it != map.end()) {
          it->second.inputs.emplace(mit.first, Signal::Low);
        }
      }
    }

    return map;
  };

  const std::pair<int64_t, int64_t> sentinel{0, 0};
  const auto PushTheButton = [](auto& map, std::string_view target = {}) {
    std::array<int64_t, 2> counts{0, 0};
    
    constexpr std::string_view button{"button"};
    constexpr std::string_view broadcaster{"broadcaster"};

    std::deque<Emission> q;
    q.emplace_back(button, Signal::Low, broadcaster);
    while (!q.empty()) {
      auto& v = q.front();
      if (v.signal == Signal::Low && v.destination == target) {
        return sentinel;
      }
      counts[static_cast<int>(v.signal)]++;
      auto r = map.find(v.destination);
      if (r != map.end()) {
        const auto& emissions = r->second.receive(v.signal, v.source);
        for (const auto& e : emissions) {
          q.emplace_back(e);
        }
      }
      q.pop_front();
    }

    return std::make_pair(counts[0], counts[1]);
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  ModuleMap map;
  if (inTest) {
    map = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    map = LoadInput(f);
  }

  int64_t part1{};
  int64_t part2{};

  {
    aoc::AutoTimer t1{"Part 1"};
    std::array<int64_t, 2> counts{0, 0};

    auto part1_map = map;

    for (int i = 0; i < 1000; i++) {
      const auto c = PushTheButton(part1_map);
      counts[0] += c.first;
      counts[1] += c.second;
    }
    part1 = counts[0] * counts[1];
  }

  {
    aoc::AutoTimer t2{"Part 2"};
    std::deque<Emission> q;

    // find all inputs to "rx"
    std::vector<std::string_view> rx_inputs;
    for (auto& mit : map) {
      for (auto& o : mit.second.outputs) {
        if (o == "rx") {
          rx_inputs.emplace_back(mit.first);
        }
      }
    }
    assert(rx_inputs.size() < 2);
    std::vector<int64_t>outputs;
    for (auto& rxi : rx_inputs) {
      const auto it = map.find(rxi);
      assert(it != map.end());
      assert(it->second.type == ModuleType::Conjunction);
      for (auto& i : it->second.inputs) {

        int64_t count{1};
        auto part2_map = map;
        while (true) {
          const auto c = PushTheButton(part2_map, i.first);
          if (c == sentinel) {
            break;
          }
          count ++;
        }
        DEBUG_PRINT("Tracking: " << i.first << " count: " << count);
        outputs.emplace_back(count);
      }
    }
    part2 = aoc::lcm(outputs.begin(), outputs.end());
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
