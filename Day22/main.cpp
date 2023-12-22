#include "aoc/helpers.h"
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <queue>
#include "aoc/range.h"

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  static int64_t brickId = 0;

  constexpr std::string_view SampleInput(R"(1,0,1~1,2,1
0,0,2~2,0,2
0,2,3~2,2,3
0,0,4~0,2,4
2,0,5~2,2,5
0,1,6~2,1,6
1,1,8~1,1,9)");
  constexpr int64_t SR_Part1 = 5;
  constexpr int64_t SR_Part2 = 7;

  using Range = aoc::Range<int64_t>;
  using Point3D = std::tuple<int64_t, int64_t, int64_t>;

  struct Brick {
    int64_t id;
    Range x;
    Range y;
    Range z;

    Brick() = default;

    Brick(Point3D start, Point3D end)
      : id(brickId++)
      , x(Range::FromFirstAndLast(std::min(std::get<0>(start), std::get<0>(end)), std::max(std::get<0>(start), std::get<0>(end))))
      , y(Range::FromFirstAndLast(std::min(std::get<1>(start), std::get<1>(end)), std::max(std::get<1>(start), std::get<1>(end))))
      , z(Range::FromFirstAndLast(std::min(std::get<2>(start), std::get<2>(end)), std::max(std::get<2>(start), std::get<2>(end))))
    {
    }

    bool operator== (const Brick &other) const = default;

    auto operator <=> (const Brick &other) const
    {
      // Sort by lowest Z first, then by highest Z, then by the other stuff.
      if (auto r = z.First() <=> other.z.First(); r != std::strong_ordering::equal)
        { return r; }

      if (auto r = z.Last() <=> other.z.Last(); r != std::strong_ordering::equal)
        { return r; }

      if (auto r = x.First() <=> other.x.First(); r != std::strong_ordering::equal)
        { return r; }

      if (auto r = y.First() <=> other.y.First(); r != std::strong_ordering::equal)
        { return r; }

      if (auto r = x.Last()  <=> other.x.Last(); r != std::strong_ordering::equal)
        { return r; }

      return y.Last() <=> other.y.Last();
    }

    // Bricks intersect if all their ranges overlap
    bool Intersects(const Brick &other) const
      { return x.Overlaps(other.x) && y.Overlaps(other.y) && z.Overlaps(other.z); }

    static Brick Parse(std::string_view s) {
      std::string_view part;
      std::vector<Point3D> points;
      while (aoc::getline(s, part, '~')) {
        std::string_view coord;
        std::vector<int64_t> coords;
        aoc::parse_as_integers(part, ',', [&coords](const auto p) { coords.push_back(p); });
        assert(coords.size() == 3);
        points.push_back(std::make_tuple( coords[0], coords[1], coords[2] ));
      }
      assert(points.size() == 2);
      return Brick(points[0], points[1]);
    }
  };

  using Input = std::vector<Brick>;

  const auto LoadInput = [](auto f) {
    Input r;
    std::string_view line;
    while (aoc::getline(f, line)) {
      r.push_back(Brick::Parse(line));
    }
    std::sort(r.begin(), r.end());
    return r;
  };


  const auto Stabilize = [](Input &r) {
    int64_t z_max = 1;
    for (size_t i = 0; i < r.size(); ++i) {
      auto& cur = r[i];

      // move it down as far as we can, base on previous z_max
      cur.z = Range::FromStartAndLength(std::min(cur.z.First(), z_max + 1), cur.z.Length());
      while (cur.z.First() > 0) {
        auto prev = cur.z;
        // drop it by one
        cur.z = Range::FromStartAndLength(cur.z.First() - 1, cur.z.Length());

        bool overlaps = false;
        for (size_t j = 0; !overlaps && j < i; ++j) {
          const auto& other = r[j];
          overlaps = cur.Intersects(other);
        }
        if (overlaps) {
          DEBUG_PRINT("Can't drop: " << cur.id << " stabilized at " << prev.First());
          cur.z = prev;
          break;
        }
      }

      z_max = std::max(z_max, cur.z.Last());
    }
    std::sort(r.begin(), r.end());
  };

  struct BrickGraph {
    Brick b;
    std::vector<size_t> supporting;
    std::vector<size_t> supported_by;
  };

  const auto BuildGraph = [](const Input &r) {
    std::vector<BrickGraph> graph{r.size()};

    for (size_t i = 0; const auto& b : r) {
      graph[i].b = b;
      i++;
    }

    for (size_t i = 0; i < r.size(); ++i) {
      auto& cur = graph[i];
      if (cur.b.z.First() == 0) { continue; }

      // if when lowered, it intersects another, then it is supported by it
      auto lowered = cur.b;
      lowered.z = Range::FromStartAndLength(lowered.z.First() - 1, lowered.z.Length());
      for (size_t j = 0; j < i; ++j) {
        auto& other = graph[j];
        if (other.b.Intersects(lowered)) {
          other.supporting.push_back(i);
          cur.supported_by.push_back(j);

          DEBUG_PRINT("Supporting " << cur.b.id << " by " << other.b.id);
        }
      }
    }

    return graph;
  };
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

  Stabilize(r);

  const auto graph = BuildGraph(r);

  int64_t part1 = 0;

  {
    aoc::AutoTimer t1{"Part 1"};

    part1 = std::count_if(graph.begin(), graph.end(), [&graph](const auto& g) {
      return !(std::any_of(g.supporting.begin(), g.supporting.end(), [&graph](const auto& i) {
        const auto& other = graph[i];
        return other.supported_by.size() == 1;
      }));
    });
  }

  int64_t part2 = 0;

  {
    aoc::AutoTimer t2{"Part 2"};

    for (size_t i = 0; i < graph.size(); ++i) {
      std::vector<bool> fallen_bricks;
      fallen_bricks.resize(graph.size());

      std::queue<size_t> q;
      q.push(i);

      while (!q.empty()) {
        auto fi = q.front();
        q.pop();
        fallen_bricks[fi] = true;
        const auto &fg = graph[fi];
        for (auto s : fg.supporting) {
          assert(!fallen_bricks[s]);
          bool u = true;

          auto &sup = graph[s];
          for (auto b : sup.supported_by) {
            if (!fallen_bricks[b]) {
              u = false;
              break;
            }
          }

          if (u) {
            DEBUG_PRINT("Adding " << graph[s].b.id << " to queue");
            q.push(s);
            part2++;
          }
        }
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
