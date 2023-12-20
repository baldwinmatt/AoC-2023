#include "aoc/helpers.h"
#include "aoc/point.h"
#include "aoc/padded_matrix.h"
#include <thread>
#include <set>
#include <unordered_set>
#include <stack>
#include <numeric>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(.|...\....
|.-.\.....
.....|-...
........|.
..........
.........\
..../.\\..
.-.-/..|..
.|....-|.\
..//.|....)");
  constexpr int SR_Part1 = 46;
  constexpr int SR_Part2 = 51;

  // A beam has a position and a direction
  struct Beam {
    aoc::point pos;
    aoc::CardinalDirection dir;

    constexpr operator int64_t() const {
      return pos.x * 1000000 + pos.y * 1000 + static_cast<int64_t>(dir);
    }
  };

  using Beams = std::stack<Beam>;
  // We need to keep track of all beams
  Beams beams;

  // We will maintain a grid of energized spaces
  using EnergizedGrid = aoc::PaddedMatrix<bool, 1>;

  // Our initial grid
  using Grid = aoc::PaddedMatrix<char, 1>;

  // We need to track all the places, and directions a beam has been
  using Visited = std::unordered_set<int64_t>;

  const auto LoadInput = [](auto f) {
    Grid g;
    std::string_view line;
    while (aoc::getline(f, line)) {
      g.set_width(line.size());
      auto y = g.add_row();
      for (size_t x = 0; x < line.size(); ++x) {
        g.add(x, y, line[x]);
      }
    }
    g.fill_pddding('X');
    return g;
  };

  const auto PrintGrid = [](const Grid& g, const EnergizedGrid& e) {
    aoc::reset(std::cout);
    for (ssize_t y = -1; y < g.get_height() + 1; ++y) {
      for (ssize_t x = -1; x < g.get_width() + 1; ++x) {
        char c = g.at(x, y);
        if (c == '.' && e.at(x, y)) { c = '#'; }
        if (e.at(x, y)) {
          std::cout << aoc::bold_on;
        }
        std::cout << c;
        if (e.at(x, y)) {
          std::cout << aoc::bold_off;
        }
      }
      std::cout << std::endl;
    }
  };

  const auto EnergizeGrid = [](const Grid& g, Beam initial, Visited& visited) {
    EnergizedGrid e;
    e.set_width(g.get_width());
    e.set_height(g.get_height());
    
    Beams beams;
    beams.push(initial);

    while (!beams.empty()) {
      auto b = beams.top();
      beams.pop();
      while (g.at(b.pos) != 'X') {
        auto it = visited.emplace((int64_t)b);
        if (!it.second) {
          // we are looping
          break;
        }
        // current position is energized
        e.add(b.pos, true);

        // now move our beam
        const auto c = g.at(b.pos);
        bool terminal = false;
        switch (c) {
          case '.':
          {
            break;
          }
          case '|':
          {
            if (b.dir != aoc::CardinalDirection::South &&
              b.dir != aoc::CardinalDirection::North) {
              // split, this beam goes north, the new beam goes south
              b.dir = aoc::CardinalDirection::North;
              // add a new beam
              beams.emplace(aoc::point{ b.pos.x, b.pos.y + 1}, aoc::CardinalDirection::South);
            }
            break;
          }
          case '-':
          {
            if (b.dir != aoc::CardinalDirection::East &&
              b.dir != aoc::CardinalDirection::West) {
              // split, this beam goes east, the new beam goes west
              b.dir = aoc::CardinalDirection::East;
              // add a new beam
              beams.emplace(aoc::point{ b.pos.x - 1, b.pos.y }, aoc::CardinalDirection::West);
            }
            break;
          }
          case '/':
          {
            switch (b.dir) {
              case aoc::CardinalDirection::North:
                b.dir = aoc::CardinalDirection::East;
                break;
              case aoc::CardinalDirection::East:
                b.dir = aoc::CardinalDirection::North;
                break;
              case aoc::CardinalDirection::South:
                b.dir = aoc::CardinalDirection::West;
                break;
              case aoc::CardinalDirection::West:
                b.dir = aoc::CardinalDirection::South;
                break;
              default:
                throw std::runtime_error("Unknown direction");
            }
            break;
          }
          case '\\':
          {
            switch (b.dir) {
              case aoc::CardinalDirection::North:
                b.dir = aoc::CardinalDirection::West;
                break;
              case aoc::CardinalDirection::East:
                b.dir = aoc::CardinalDirection::South;
                break;
              case aoc::CardinalDirection::South:
                b.dir = aoc::CardinalDirection::East;
                break;
              case aoc::CardinalDirection::West:
                b.dir = aoc::CardinalDirection::North;
                break;
              default:
                throw std::runtime_error("Unknown direction");
            }
            break;
          }
          case 'X':
          {
            // we are done;
            terminal = true;
            break;
          }
          default:
            throw std::runtime_error("Unknown char");
        }

        if (!terminal) {
          b.pos += aoc::point::step(b.dir);
        }
        DEBUG(
          PrintGrid(g, e);
          std::cout << "Beams: " << beams.size() << "                    " << std::endl;
          std::cout << "Current: " << b.pos << " " << b.dir << " visited " << visited.size() << "                    " << std::endl;
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        );
      }
    }

    int64_t energy{};
    for (ssize_t y = 0; y < e.get_height(); ++y) {
      for (ssize_t x = 0; x < e.get_width(); ++x) {
        energy += e.at(x, y);
      }
    }
    return energy;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Grid g;
  if (inTest) {
    g = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    g = LoadInput(f);
  }

  DEBUG(
    aoc::cls(std::cout);
  );

  Visited visited;
  int64_t part1{};
  {
    aoc::AutoTimer t2{"Part 1"};
    part1 = EnergizeGrid(g, Beam{ aoc::point{ 0, 0 }, aoc::CardinalDirection::East }, visited);
  }

  DEBUG(
    exit(1);
  );

  // find ideal start position
  int64_t part2{part1};
  {
    aoc::AutoTimer t2{"Part 2"};
    std::vector<Beam> beams;
    for (int32_t y = 0; y < g.get_height(); ++y) {
      if (y > 0) {
        beams.emplace_back(aoc::point{ 0, y }, aoc::CardinalDirection::East);
      }
      beams.emplace_back(aoc::point{ static_cast<int32_t>(g.get_width()) - 1, y }, aoc::CardinalDirection::West);
    }

    for (int32_t x = 0; x < g.get_width(); ++x) {
      beams.emplace_back(aoc::point{ x, 0 }, aoc::CardinalDirection::South);
      beams.emplace_back(aoc::point{ x, static_cast<int32_t>(g.get_height()) - 1 }, aoc::CardinalDirection::North);
    }

    struct Reducer {
      const Grid& g;
      Visited& v;

      Reducer(const Grid& g, Visited& v) : g(g), v(v) {}
      
      int64_t operator()(const int64_t e, const Beam& b) {
        v.clear();
        return std::max(e, EnergizeGrid(g, b, v));
      }

      int64_t operator()(const int64_t e, const int64_t b) {
        return std::max(e, b);
      }
      int64_t operator()(const Beam& e, const Beam& b) {
        v.clear();
        return std::max(EnergizeGrid(g, e, v), EnergizeGrid(g, b, v));
      }
      int64_t operator()(const Beam& e, const int64_t b) {
        v.clear();
        return std::max(EnergizeGrid(g, e, v), b);
      }
    };

    Reducer r(g, visited);

    part2 = std::accumulate(beams.cbegin(), beams.cend(), part1, r);
  }
  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
