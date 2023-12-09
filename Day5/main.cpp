#include "aoc/helpers.h"

#include <map>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(seeds: 79 14 55 13

seed-to-soil map:
50 98 2
52 50 48

soil-to-fertilizer map:
0 15 37
37 52 2
39 0 15

fertilizer-to-water map:
49 53 8
0 11 42
42 0 7
57 7 4

water-to-light map:
88 18 7
18 25 70

light-to-temperature map:
45 77 23
81 45 19
68 64 13

temperature-to-humidity map:
0 69 1
1 0 69

humidity-to-location map:
60 56 37
56 93 4)");
  constexpr int SR_Part1 = 35;
  constexpr int SR_Part2 = 46;

  struct Range {
    int64_t start;
    int64_t end;

    constexpr bool contains(int64_t v) const {
      return v >= start && v <= end;
    }

    Range(int64_t s, int64_t n)
      : start(s)
      , end(s + n - 1)
    {
    }

    bool operator<(const Range& r) const {
      return start < r.start;
    }

    constexpr int64_t offset(int64_t v) const {
      return v - start;
    }
  };

  using RangeMap = std::vector<std::pair<Range, Range>>;

  const auto LocateInMap = [](const RangeMap& m, int64_t v) -> int64_t {
    for (const auto& r : m) {
      if (r.first.contains(v)) {
        DEBUG_PRINT("Found " << v << " in " << r.first.start << "-" << r.first.end << " offset: " << r.first.offset(v));
        return r.second.start + r.first.offset(v);
      }
    }
    return v;
  };

  struct Almanac {
    std::vector<int64_t> Seeds;
    RangeMap SeedToSoil;
    RangeMap SoilToFertilizer;
    RangeMap FertilizerToWater;
    RangeMap WaterToLight;
    RangeMap LightToTemperature;
    RangeMap TemperatureToHumidity;
    RangeMap HumidityToLocation;

    int64_t GetSeedLocation(int64_t seed) const {
      // locate the soil from the seed->soil map
      DEBUG_PRINT("Locating soil for seed: " << seed);
      const auto soil = LocateInMap(SeedToSoil, seed);
      DEBUG_PRINT("Soil: " << soil);
      const auto fertilizer = LocateInMap(SoilToFertilizer, soil);
      DEBUG_PRINT("Fertilizer: " << fertilizer);
      const auto water = LocateInMap(FertilizerToWater, fertilizer);
      DEBUG_PRINT("Water: " << water);
      const auto light = LocateInMap(WaterToLight, water);
      DEBUG_PRINT("Light: " << light);
      const auto temperature = LocateInMap(LightToTemperature, light);
      DEBUG_PRINT("Temperature: " << temperature);
      const auto humidity = LocateInMap(TemperatureToHumidity, temperature);
      DEBUG_PRINT("Humidity: " << humidity);
      return LocateInMap(HumidityToLocation, humidity);
    }
  };

  const auto ParseMap = [](auto& f, RangeMap& r) {
    std::string_view line;
    while (aoc::getline(f, line, "\r\n", true)) {
      if (line.empty()) {
        break;
      }

      std::array<int64_t, 3>values;
      int i = 0;
      aoc::parse_as_integers(line, ' ', [&values, &i](const auto v){ values[i++] = v; });
      DEBUG_PRINT("Destination: " << values[0] << " Source: " << values[1] << " Range: " << values[2]);
      r.emplace_back(Range(values[1], values[2]), Range(values[0], values[2]));
    }
  };
      

  const auto LoadInput = [](auto f, Almanac& r) {
    std::string_view line;
    // seeds on the first line
    aoc::getline(f, line);
    // skip the seeds label
    line.remove_prefix(sizeof("seeds:") - 1);
    // eat the whitespace
    while (line.front() == ' ') { line.remove_prefix(1); }
    // parse the seeds
    aoc::parse_as_integers(line, ' ', [&r](const auto i){ r.Seeds.push_back(i); });

    // skip the blank line, and header
    aoc::getline(f, line);
    DEBUG_PRINT(line);
    assert(line == "seed-to-soil map:");
    // parse the seed-to-soil map
    ParseMap(f, r.SeedToSoil);

    // skip the blank line, and header
    aoc::getline(f, line);
    DEBUG_PRINT(line);
    assert(line == "soil-to-fertilizer map:");
    // parse the soil-to-fertilizer map
    ParseMap(f, r.SoilToFertilizer);

    // skip the blank line, and header
    aoc::getline(f, line);
    DEBUG_PRINT(line);
    assert(line == "fertilizer-to-water map:");
    // parse the fertilizer-to-water map
    ParseMap(f, r.FertilizerToWater);

    // skip the blank line, and header
    aoc::getline(f, line);
    DEBUG_PRINT(line);
    assert(line == "water-to-light map:");
    // parse the water-to-light map
    ParseMap(f, r.WaterToLight);

    // skip the blank line, and header
    aoc::getline(f, line);
    DEBUG_PRINT(line);
    assert(line == "light-to-temperature map:");
    // parse the light-to-temperature map
    ParseMap(f, r.LightToTemperature);

    // skip the blank line, and header
    aoc::getline(f, line);
    DEBUG_PRINT(line);
    assert(line == "temperature-to-humidity map:");
    // parse the temperature-to-humidity map
    ParseMap(f, r.TemperatureToHumidity);

    // skip the blank line, and header
    aoc::getline(f, line);
    // parse the humidity-to-location map
    DEBUG_PRINT(line);
    assert(line == "humidity-to-location map:");
    ParseMap(f, r.HumidityToLocation);
  };
}

int main(int argc, char** argv) {
  const bool inTest = argc < 2;

  aoc::AutoTimer t;
  Almanac r;
  {
    aoc::AutoTimer t1{"Load"};
    if (inTest) {
      LoadInput(SampleInput, r);
    } else {
      std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
      std::string_view f(m->data(), m->size());
      LoadInput(f, r);
    }
  }

  int64_t part1 = INT64_MAX;

  {
    aoc::AutoTimer t2{ "Part 1"};
    for (const auto seed : r.Seeds) {
      const auto location = r.GetSeedLocation(seed);
      part1 = std::min(part1, location);
    }
  }
  int64_t part2 = INT64_MAX;
  {
    aoc::AutoTimer t3{ "Part 2" };
    for (size_t i = 0; i < r.Seeds.size(); i += 2) {
      Range s(r.Seeds[i], r.Seeds[i + 1]);
      for (auto j = s.start; j <= s.end; j++) {
        part2 = std::min(part2, r.GetSeedLocation(j));
      }
    }
  }
  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
