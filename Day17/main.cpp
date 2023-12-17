#include "aoc/helpers.h"
#include "aoc/padded_matrix.h"
#include <queue>
#include <unordered_map>

namespace {
  using Result = std::pair<int64_t, int64_t>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(2413432311323
3215453535623
3255245654254
3446585845452
4546657867536
1438598798454
4457876987766
3637877979653
4654967986887
4564679986453
1224686865563
2546548887735
4322674655533)");
  constexpr int64_t SR_Part1 = 102;
  constexpr int64_t SR_Part2 = 71;

  using Grid = aoc::PaddedMatrix<int32_t, 1>;

  const auto LoadInput = [](auto f) {
    std::string_view line;
    Grid r;
    while (aoc::getline(f, line)) {
      r.set_width(line.size());
      const auto y = r.add_row();
      ssize_t x = 0;
      for (const auto c : line) {
        r.add(x++, y, aoc::ctoi(c));
      }
    }
    r.fill_pddding('X');
    return r;
  };

  struct Pos {
    aoc::point p; // position
    aoc::CardinalDirection dir; // current direction
    int steps; // number of steps taken in current direction

    Pos()
      : p{ 0, 0 }
      , dir{ aoc::CardinalDirection::East }
      , steps{ 0 }
    { }

    Pos(aoc::point p, aoc::CardinalDirection dir, int steps)
      : p{ p }
      , dir{ dir }
      , steps{ steps }
    { }

    operator int64_t() const {
      int64_t v = p.y;
      v <<= 16;
      v |= p.x;
      v <<= 16;
      v |= static_cast<int64_t>(dir);
      v <<= 16;
      v |= steps;
      return v;
    }
  };

  using PosPriority = std::pair<Pos, int64_t>;
  struct PriorityComparator {
    bool operator()(const PosPriority& l, const PosPriority& r) const {
      return l.second > r.second;
    }
  };

  using PriorityQueue = std::priority_queue<PosPriority, std::vector<PosPriority>, PriorityComparator>;
  using VisitedMap = std::unordered_map<int64_t, int64_t>;

  struct StepBounds {
    const int min;
    const int max;
    StepBounds(int min, int max)
      : min{ min }
      , max{ max }
    { }

    static StepBounds Part1() {
      return StepBounds{ 1, 3 };
    }

    static StepBounds Part2() {
      return StepBounds{ 4, 10 };
    }
  };

  const auto FindShortestPath = [](const Grid& r, StepBounds bounds) {
    int64_t cost{INT64_MAX};
    PriorityQueue q;
    VisitedMap visited;

    const aoc::point End{ static_cast<int32_t>(r.get_width()) - 1, static_cast<int32_t>(r.get_height()) - 1 };
    const aoc::point Start{ 0, 0 };

    q.push(PosPriority{ Pos{ Start, aoc::CardinalDirection::East, 0 }, 0 });
    q.push(PosPriority{ Pos{ Start, aoc::CardinalDirection::South, 0 }, 0 });

    while (!q.empty()) {
      auto [p, loss] = q.top();
      q.pop();
      if (p.p == End) {
        DEBUG_PRINT("Found end: " << p.p << " loss: " << loss << " dir: " << p.dir << " steps: " << p.steps << " prev: " << cost);
        cost = std::min(cost, loss);
        return cost;
      }

      DEBUG_PRINT("Checking position: " << p.p << " loss: " << loss << " dir: " << p.dir << " steps: " << p.steps);

      const auto EnqueueNext = [&r, &visited, &q](Pos p, int64_t loss, aoc::CardinalDirection dir, int steps) {
        auto next = p;
        const auto max_p = next.p + (aoc::point::step(dir) * steps);
        if (max_p.x < 0 || max_p.y < 0 || max_p.x >= r.get_width() || max_p.y >= r.get_height()) {
          return;
        }
        next.p += aoc::point::step(dir);
        if (r.at(next.p) != 'X') {
          if (dir == next.dir) {
            next.steps ++;
          } else {
            next.steps = 1; // this is the first step
            next.dir = dir; // this is the new direction
          }
          auto next_loss = loss + r.at(next.p);
          auto vit = visited.emplace(next, next_loss);
          if (vit.second || vit.first->second > next_loss) {
            q.push(PosPriority{ next, next_loss });
          }
        }
      };

      if (p.steps < bounds.max) {
        EnqueueNext(p, loss, p.dir, 1);
      }

      // try turning in each direction (except our opposite)
      switch (p.dir) {
        case aoc::CardinalDirection::East:
          EnqueueNext(p, loss, aoc::CardinalDirection::North, bounds.min);
          EnqueueNext(p, loss, aoc::CardinalDirection::South, bounds.min);
          break;
        case aoc::CardinalDirection::North:
          EnqueueNext(p, loss, aoc::CardinalDirection::West, bounds.min);
          EnqueueNext(p, loss, aoc::CardinalDirection::East, bounds.min);
          break;
        case aoc::CardinalDirection::South:
          EnqueueNext(p, loss, aoc::CardinalDirection::West, bounds.min);
          EnqueueNext(p, loss, aoc::CardinalDirection::East, bounds.min);
          break;
        case aoc::CardinalDirection::West:
          EnqueueNext(p, loss, aoc::CardinalDirection::North, bounds.min);
          EnqueueNext(p, loss, aoc::CardinalDirection::South, bounds.min);
          break;
        default:
          throw std::runtime_error("Unknown direction");
      }
    }

    return cost;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Grid r;
  if (inTest) {
    r = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f);
  }

  int64_t part1 = INT64_MAX;

  {
    aoc::AutoTimer t1{"Part 1"};
    const auto bounds = StepBounds::Part1();

    part1 = FindShortestPath(r, bounds);
  }

  int64_t part2 = INT64_MAX;
  {
    aoc::AutoTimer t2{"Part 2"};
    const auto bounds = StepBounds::Part2();

    part2 = FindShortestPath(r, bounds);
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
