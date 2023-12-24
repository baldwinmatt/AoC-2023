#include "aoc/helpers.h"
#include "aoc/point3d.h"

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"()");
  constexpr int SR_Part1 = 0;
  constexpr int SR_Part2 = 0;

  constexpr int64_t min_c = 200'000'000'000'000ll;
  constexpr int64_t max_c = 400'000'000'000'000ll;

  struct Hail {
    aoc::point3d64 pos;
    aoc::point3d64 vel;
  };

  using HailStones = std::vector<Hail>;

  const auto LoadInput = [](auto f) {
    HailStones r;
    std::string_view line;
    while (aoc::getline(f, line)) {
      const auto parts = aoc::split(line, " ,@");
      assert(parts.size() == 6);
      r.push_back(Hail{
        { aoc::stoi(parts[0]), aoc::stoi(parts[1]), aoc::stoi(parts[2]) },
        { aoc::stoi(parts[3]), aoc::stoi(parts[4]), aoc::stoi(parts[5]) }}
      );
    }
    return r;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  HailStones r;
  if (inTest) {
    r = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    r = LoadInput(f);
  }

  int64_t part1{};
  int64_t part2{};

  {
    aoc::AutoTimer t1{"Part 1"};

    for (size_t i = 0; i < r.size(); i++) {
      for (size_t j = i + 1; j < r.size(); j++) {
        auto &si = r[i];
        auto &sj = r[j];

        double cx = double(si.pos.x);
        double cy = double(si.pos.y);
        double dx = double(si.vel.x);
        double dy = double(si.vel.y);

        double ax = double(sj.pos.x);
        double ay = double(sj.pos.y);
        double bx = double(sj.vel.x);
        double by = double(sj.vel.y);

        double det = (dx*by - dy*bx);
        if (det == 0)
        { continue; }

        double u = (bx*(cy-ay) + by*(ax-cx))/det;
        double t = (dx*(cy-ay) + dy*(ax-cx))/det;

        double x = ax + bx * t;
        double y = ay + by * t;

        if (u < 0 || t < 0)
        { continue; }

        if (x >= double{min_c} && x <= double{max_c} && y >= double{min_c} && y <= double{max_c}) {
          part1++;
          DEBUG_PRINT("found " << i << " " << j);
        }
      }
    }
  }

  {
    aoc::AutoTimer t1{"Part 2"};

    int32_t s0Index = 0;
    int32_t s1Index = 1;
    while (r[s0Index].vel == r[s1Index].vel )
      { s1Index++; }

    int32_t s2Index = s1Index + 1;
    while (r[s0Index].vel == r[s2Index].vel || r[s1Index].vel == r[s2Index].vel)
      { s2Index++; }

    double A0x = double(r[s0Index].pos.x);
    double A0y = double(r[s0Index].pos.y);
    double A0z = double(r[s0Index].pos.z);
    double Avx = double(r[s0Index].vel.x);
    double Avy = double(r[s0Index].vel.y);
    double Avz = double(r[s0Index].vel.z);

    // B0x + Bvx*u = Px + Qx*u
    // B0y + Bvy*u = Py + Qy*u
    // B0z + Bvz*u = Pz + Qz*u
    double B0x = double(r[s1Index].pos.x);
    double B0y = double(r[s1Index].pos.y);
    double B0z = double(r[s1Index].pos.z);
    double Bvx = double(r[s1Index].vel.x);
    double Bvy = double(r[s1Index].vel.y);
    double Bvz = double(r[s1Index].vel.z);

    // C0x + Cvx*v = Px + Qx*v
    // C0y + Cvy*v = Py + Qy*v
    // C0z + Cvz*v = Pz + Qz*v
    double C0x = double(r[s2Index].pos.x);
    double C0y = double(r[s2Index].pos.y);
    double C0z = double(r[s2Index].pos.z);
    double Cvx = double(r[s2Index].vel.x);
    double Cvy = double(r[s2Index].vel.y);
    double Cvz = double(r[s2Index].vel.z);

    double abvx = Avx - Bvx;
    double abvy = Avy - Bvy;
    double abvz = Avz - Bvz;

    double acvx = Avx - Cvx;
    double acvy = Avy - Cvy;
    double acvz = Avz - Cvz;

    double ab0x = A0x - B0x;
    double ab0y = A0y - B0y;
    double ab0z = A0z - B0z;

    double ac0x = A0x - C0x;
    double ac0y = A0y - C0y;
    double ac0z = A0z - C0z;

    double h0 = (B0y * Bvx - B0x * Bvy) - (A0y * Avx - A0x * Avy);
    double h1 = (C0y * Cvx - C0x * Cvy) - (A0y * Avx - A0x * Avy);
    double h2 = (B0x * Bvz - B0z * Bvx) - (A0x * Avz - A0z * Avx);
    double h3 = (C0x * Cvz - C0z * Cvx) - (A0x * Avz - A0z * Avx);
    double h4 = (B0z * Bvy - B0y * Bvz) - (A0z * Avy - A0y * Avz);
    double h5 = (C0z * Cvy - C0y * Cvz) - (A0z * Avy - A0y * Avz);

    auto Pxx = acvx*ab0z - abvx*ac0z;
    auto Pyy = acvy*ab0x - abvy*ac0x;
    auto Pzz = acvz*ab0y - abvz*ac0y;

    auto Pxz = abvx*ac0x - acvx*ab0x;
    auto Pzy = abvz*ac0z - acvz*ab0z;
    auto Pyx = abvy*ac0y - acvy*ab0y;

    auto Pxc = abvx*h3 - acvx*h2;
    auto Pyc = abvy*h1 - acvy*h0;
    auto Pzc = abvz*h5 - acvz*h4;

    auto Pxd = acvx*abvz - abvx*acvz;
    auto Pyd = acvy*abvx - abvy*acvx;
    auto Pzd = acvz*abvy - abvz*acvy;

    auto Qz0 = (abvy/Pxd)*Pxz;
    auto Qx0 = (abvy/Pxd)*Pxx - (abvx/Pyd)*Pyx - ab0y;
    auto Qy0 = ab0x - (abvx/Pyd)*Pyy;
    auto r0 = h0 - (abvy/Pxd)*Pxc + (abvx/Pyd)*Pyc;

    auto Qy1 = (abvx/Pzd)*Pzy;
    auto Qz1 = (abvx/Pzd)*Pzz - (abvz/Pxd)*Pxz - ab0x;
    auto Qx1 = ab0z - (abvz/Pxd)*Pxx;
    auto r1 = h2 - (abvx/Pzd)*Pzc + (abvz/Pxd)*Pxc;

    auto Qx2 = (abvz/Pyd)*Pyx;
    auto Qy2 = (abvz/Pyd)*Pyy - (abvy/Pzd)*Pzy - ab0z;
    auto Qz2 = ab0y - (abvy/Pzd)*Pzz;
    auto r2 = h4 - (abvz/Pyd)*Pyc + (abvy/Pzd)*Pzc;

    double Qz = ((Qx1*Qy0 - Qx0*Qy1)*(Qx2*r0 - Qx0*r2) - (Qx2*Qy0 - Qx0*Qy2)*(Qx1*r0 - Qx0*r1))
      / ((Qx2*Qy0 - Qx0*Qy2)*(Qx0*Qz1 - Qx1*Qz0) - (Qx1*Qy0 - Qx0*Qy1)*(Qx0*Qz2 - Qx2*Qz0));

    double Qy = ((Qx0*Qz1 - Qx1*Qz0)*Qz + (Qx1*r0 - Qx0*r1))/(Qx1*Qy0 - Qx0*Qy1);

    double Qx = (r0 - Qy0*Qy - Qz0*Qz)/Qx0;

    double Px = (Pxz*Qz + Pxx*Qx + Pxc)/Pxd;
    double Py = (Pyx*Qx + Pyy*Qy + Pyc)/Pyd;
    double Pz = (Pzy*Qy + Pzz*Qz + Pzc)/Pzd;

    part2 = int64_t(std::round(Px)) + int64_t(std::round(Py)) + int64_t(std::round(Pz));
  }


  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
