#include "aoc/helpers.h"

#include <list>

namespace {
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(rn=1,cm-,qp=3,cm=2,qp-,pc=4,ot=9,ab=5,pc-,pc=6,ot=7)");
  constexpr int SR_Part1 = 1320;
  constexpr int SR_Part2 = 145;

  const auto ElfHash = [](std::string_view s) {
    int sum = 0;
    for (const auto c: s) {
      sum += static_cast<int>(c);
      sum *= 17;
      sum %= 256;
    }

    DEBUG_PRINT("ElfHash(" << s << "): " << sum);

    return sum;
  };

  enum class Operator {
    EQUALS = 0,
    SUBTRACT = 1,
  };

  struct Lens {
    std::string label;
    Operator op;
    int operand;
    const int hash;
    int label_hash;

    Lens(std::string_view s)
      : hash(ElfHash(s)) {
      const auto it = s.find('=');
      if (it != std::string_view::npos) {
        op = Operator::EQUALS;
        operand = aoc::stoi(s.substr(it + 1));
        s.remove_suffix(s.size() - it);
      } else {
        op = Operator::SUBTRACT;
        s.remove_suffix(1);
        operand = 0;
      }
      label = std::string(s.data(), s.size());
      label_hash = ElfHash(label);
    }
  };

  using Input = std::vector<Lens>;
  using Box = std::list<const Lens*>;
  using BoxMap = Box[256];

  const auto FindInBox = [](const Box& box, const std::string_view label) {
    for (auto it = box.begin(); it != box.end(); ++it) {
      if ((*it)->label == label) {
        return it;
      }
    }
    return box.end();
  };

  const auto RemoveFromBox = [](Box& box, const Lens* l) {
    auto it = FindInBox(box, l->label);
    if (it != box.end()) {
      box.erase(it);
    }
  };

  const auto ReplaceOrAddToBox = [](Box& box, const Lens* l) {
    auto it = FindInBox(box, l->label);
    if (it != box.end()) {
      // replace the existing one
      box.insert(it, l);
      box.erase(it);
    } else {
      box.push_back(l);
    }
  };

  const auto PlaceLensInBox = [](BoxMap& m, const Lens *l) {
    // determine the box, using the hash
    const auto box_id = l->label_hash;
    // get the box
    auto& box = m[box_id];

    switch (l->op) {
      case Operator::EQUALS:
        {
          // if the box contains a lens with this label, replace it with the new one
          // else add this to the back of the box
          ReplaceOrAddToBox(box, l);
        }
        break;
      case Operator::SUBTRACT:
        {
          RemoveFromBox(box, l);
        }
        break;
    }
  };

  const auto GetBoxLoad = [](const Box& b, int64_t idx) {
    int64_t pos = 1;
    int64_t load = 0;
    if (b.empty()) {
      return load;
    }
    DEBUG(
      std::cout << "Box " << idx << ":";
    );
    for (const auto l : b) {
      DEBUG(
        std::cout << " [" << l->label << " " << l->operand << "]"
      );
      load += (idx * pos * l->operand);
      pos++;
    }
    DEBUG(
      std::cout << std::endl;
    );
    return load;
  };

  const auto LoadInput = [](auto f) {
    Input i;
    std::string_view line;
    while (aoc::getline(f, line)) {
      std::string_view s;
      while (aoc::getline(line, s, ',')) {
        i.emplace_back(s);
      }
    }
    return i;
  };
}

int main(int argc, char** argv) {
  aoc::AutoTimer t;
  const bool inTest = argc < 2;

  Input input;
  if (inTest) {
    input = LoadInput(SampleInput);
  } else {
    std::unique_ptr<MappedFileSource>m(new MappedFileSource(argc, argv));
    std::string_view f(m->data(), m->size());
    input = LoadInput(f);
  }

  int part1 = 0;
  int part2 = 0;

  {
    aoc::AutoTimer t1{"Part 1"};

    BoxMap map;
    for (const auto& l : input) {
      part1 += l.hash;
      PlaceLensInBox(map, &l);
    }
  }

  {
    aoc::AutoTimer t2{"Part 2"};

    BoxMap map;

    for (const auto& l : input) {
      PlaceLensInBox(map, &l);
    }

    for (size_t i = 0; i < std::size(map); i++) {
      const auto& box = map[i];
      part2 += GetBoxLoad(box, i + 1);
    }
  }

  aoc::print_results(part1, part2);

  if (inTest) {
    aoc::assert_result(part1, SR_Part1);
    aoc::assert_result(part2, SR_Part2);
  }

  return 0;
}
