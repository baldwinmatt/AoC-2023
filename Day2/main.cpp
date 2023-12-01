#include "aoc/helpers.h"

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(Game 1: 3 blue, 4 red; 1 red, 2 green, 6 blue; 2 green
Game 2: 1 blue, 2 green; 3 green, 4 blue, 1 red; 1 green, 1 blue
Game 3: 8 green, 6 blue, 20 red; 5 blue, 4 red, 13 green; 5 green, 1 red
Game 4: 1 green, 3 red, 6 blue; 3 green, 6 red; 3 green, 15 blue, 14 red
Game 5: 6 red, 1 blue, 3 green; 2 blue, 1 red, 2 green)");
  constexpr int SR_Part1 = 8;
  constexpr int SR_Part2 = 2286;

  constexpr int64_t max_red = 12;
  constexpr int64_t max_green = 13;
  constexpr int64_t max_blue = 14;

  // If the game line is possible, return the Game ID, else 0
  const auto ParseLine = [](std::string_view line) -> std::pair<int64_t, int64_t> {
    constexpr size_t prefix_len = sizeof("Game ") - 1;
    // eat the "Game " prefix:
    line.remove_prefix(prefix_len);

    // read the Id (evertything up to a ':')
    int64_t id = 0;
    const auto id_end = line.find(':');
    id = aoc::stoi(line.substr(0, id_end));

    // skip the ':'
    line.remove_prefix(id_end + 1);

    int64_t game_max_red = 0;
    int64_t game_max_green = 0;
    int64_t game_max_blue = 0;
    // look until the end
    while (!line.empty()) {
      // skip the leading ' '
      line.remove_prefix(1);
      const auto quant_end = line.find(' ');
      const auto quant = aoc::stoi(line.substr(0, quant_end));
      line.remove_prefix(quant_end + 1);
      // read the color
      const auto color_end = line.find_first_of(",;");
      const auto color = line.substr(0, color_end);
      line.remove_prefix(color_end);
      line.remove_prefix(1);

      if (color == "red") {
        game_max_red = std::max(game_max_red, quant);

        if (quant > max_red) {
          DEBUG_PRINT("id: " << id << " red: " << quant << " > " << max_red);
          id = 0;
        }
      } else if (color == "green") {
        game_max_green = std::max(game_max_green, quant);
        if (quant > max_green) {
          DEBUG_PRINT("id: " << id << " green: " << quant << " > " << max_green);
          id = 0;
        }
      } else if (color == "blue") {
        game_max_blue = std::max(game_max_blue, quant);
        if (quant > max_blue) {
          DEBUG_PRINT("id: " << id << " blue: " << quant << " > " << max_blue);
          id = 0;
        }
      }
      DEBUG_PRINT(color << ": " << quant);
      if (color_end == std::string_view::npos) {
        break;
      }
      DEBUG_PRINT("line: " << line);
    }
    
    return { id, game_max_red * game_max_blue * game_max_green};
  };

  const auto LoadInput = [](auto f) {
    Result r{0, 0};
    std::string_view line;
    while (aoc::getline(f, line)) {
      auto lr = ParseLine(line);
      r.first += lr.first;
      r.second += lr.second;
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
