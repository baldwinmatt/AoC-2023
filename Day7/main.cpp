#include "aoc/helpers.h"

#include <array>
#include <map>
#include <set>

namespace {
  using Result = std::pair<int, int>;
  using MappedFileSource = aoc::MappedFileSource<char>;

  constexpr std::string_view SampleInput(R"(32T3K 765
T55J5 684
KK677 28
KTJJT 220
QQQJA 483)");
  constexpr int SR_Part1 = 6440;
  constexpr int SR_Part2 = 5905;

  const auto GetCardValue = [](char c, bool joker) -> int {
    switch(c) {
      case 'A': return 12;
      case 'K': return 11; 
      case 'Q': return 10;
      case 'J': return (joker) ? -1 : 9;
      case 'T': return 8;
      case '9': return 7;
      case '8': return 6; 
      case '7': return 5;
      case '6': return 4;
      case '5': return 3;
      case '4': return 2;
      case '3': return 1;
      case '2': return 0;
      default: break;
    }
    throw std::runtime_error("Invalid card");
  };

  enum class HandType {
    Unranked,
    HighCard,
    OnePair,
    TwoPair,
    ThreeOfAKind,
    FullHouse,
    FourOfAKind,
    FiveOfAKind,
  };

  using Hand = std::string_view;

  struct RankedHand {
    Hand hand;
    int64_t bid;
    HandType type;

    RankedHand(Hand h, int64_t b, HandType t) : hand(h), bid(b), type(t) { }
  };

  using CardCounts = std::map<char, uint_fast8_t>;
  using CardGroups = std::vector<uint_fast8_t>;

  const auto GetCardCounts = [](const std::string_view h) {
    CardCounts counts;
    for (const auto c : h) {
      auto it = counts.emplace(c, 0);
      it.first->second++;
    }
    return counts;
  };

  const auto GroupCardCountsFromCounts = [](const CardCounts& counts) {
    CardGroups groups;
    groups.reserve(counts.size());
    for (const auto& [c, count] : counts) {
      (void)c;
      groups.push_back(count);
    }

    std::sort(groups.begin(), groups.end(), std::greater<uint_fast8_t>());

    return groups;
  };

  const auto GroupCardCounts = [](const auto& h) {
    const auto counts = GetCardCounts(h);
    return GroupCardCountsFromCounts(counts);
  };

  const auto GetHandRankFromGroups = [](const CardGroups& g) {
    switch (g.size()) {
      case 1:
        return HandType::FiveOfAKind;
      case 2:
        // Either 4 of a kind or a full house
        if (g[0] == 4) {
          return HandType::FourOfAKind;
        }
        return HandType::FullHouse;
      case 3:
        // Either 3 of a kind or a two pair
        if (g[0] == 3) {
          return HandType::ThreeOfAKind;
        }
        return HandType::TwoPair;
      case 4:
        return HandType::OnePair;
      case 5:
        return HandType::HighCard;
    }
    throw std::runtime_error("Invalid hand");
  };

  const auto GetHandRank = [](const Hand& h) {
    const auto groups = GroupCardCounts(h);
    return GetHandRankFromGroups(groups);
  };

  const auto GetHandRankJoker = [](const Hand& h) {
    // remove jokers
    std::array<char, 5> cards;
    size_t i = 0;
    std::for_each(h.begin(), h.end(), [&cards, &i](const char c) { if (c != 'J') { cards[i++] = c; } });
    const auto num_jokers = h.size() - i;
    if (num_jokers == 0) {
      return GetHandRank(h);
    }
    auto groups = GroupCardCounts(std::string_view{cards.data(), i});
    if (groups.empty()) {
      groups.push_back(0);
    }
    groups[0] += num_jokers;
    return GetHandRankFromGroups(groups);
  };

  const auto RankHand = [](const Hand& h, int64_t bid) {
    return RankedHand(h, bid, GetHandRank(h));
  };

  const auto RankHandJoker = [](const Hand& h, int64_t bid) {
    return RankedHand(h, bid, GetHandRankJoker(h));
  };

  const auto HandComparatorImpl = [](const RankedHand& lhs, const RankedHand& rhs, bool joker) {
    if ( rhs.type != lhs.type) {
      return rhs.type < lhs.type;
    }

    // tie breaker
    for (size_t i = 0; i < rhs.hand.size(); ++i) {
      if (rhs.hand[i] == lhs.hand[i]) {
        continue;
      }
      return GetCardValue(rhs.hand[i], joker) < GetCardValue(lhs.hand[i], joker);
    }

    throw std::runtime_error("Invalid hand");
  };
  const auto HandComparatorJoker = [](const RankedHand& lhs, const RankedHand& rhs) {
    return HandComparatorImpl(lhs, rhs, true);
  };
  const auto HandComparator = [](const RankedHand& lhs, const RankedHand& rhs) {
    return HandComparatorImpl(lhs, rhs, false);
  };

  using Part1Games = std::set<RankedHand, decltype(HandComparator)>;
  using Part2Games = std::set<RankedHand, decltype(HandComparatorJoker)>;

  const auto ScoreGames = [](const auto& games) {
    int64_t i = 0;
    int64_t score = 0;
    for (auto it = games.rbegin(); it != games.rend(); ++it) {
      const auto bid = it->bid;
      DEBUG_PRINT("Rank: " << i << " Hand: " << it->hand << " Type: " << (int)it->type << " Bid: " << bid);
      score += ++i * bid;
    }
    return score;
  };

  const auto LoadInput = [](auto f) {
    Part1Games g1(HandComparator);
    Part2Games g2(HandComparatorJoker);
    std::string_view line;
    while (aoc::getline(f, line)) {
      const auto idx = line.find(' ');
      const auto h = line.substr(0, idx);
      const auto bid = aoc::stoi(line.substr(idx + 1));
      g1.emplace(RankHand(h, bid));
      g2.emplace(RankHandJoker(h, bid));
    }

    return std::make_pair(ScoreGames(g1), ScoreGames(g2));    
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
