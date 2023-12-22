#pragma once
#include <optional>

namespace aoc {

template <typename T>
class Range
{
private:
  Range(T s, T e)
    : start(s)
    , end(e)
    { }


  T start = 0;
  T end = 0;

public:
  Range() = default;

  static Range FromStartAndLength(T start, T length)
    { return { start, start + length }; }

  static Range FromStartAndEnd(T start, T end)
    { return { start, end }; }

  static Range FromFirstAndLast(T first, T last)
    { return { first, last + 1 }; }

  T Start() const
    { return start; }

  T End() const
    { return end; }

  T Length() const
    { return end - start; }

  T First() const
    { return start; }

  T Last() const
    { return end - 1; }

  bool Contains(T v) const
    { return v >= start && v < end; }

  bool Overlaps(Range b) const
    { return end > b.start && start < b.end; }

  std::optional<Range> PartBefore(Range b) const
  {
    if (start >= b.start)
      { return std::nullopt; }

    return Range::FromStartAndEnd(start, std::min(end, b.start));
  }

  std::optional<Range> PartAfter(Range b) const
  {
    if (end <= b.end)
      { return std::nullopt; }

    return Range::FromStartAndEnd(std::max(start, b.end), end);
  }

  std::optional<Range> Intersection(Range b) const
  {
    if (end <= b.start || start >= b.end)
      { return std::nullopt; }

    return Range::FromStartAndEnd(std::max(start, b.start), std::min(end, b.end));
  }

  std::optional<Range> Union(Range b) const
  {
    if (end >= b.start && start <= b.end)
      { return Range::FromStartAndEnd(std::min(start, b.start), std::max(end, b.end)); }

    return std::nullopt;
  }

  bool operator==(const Range &) const = default;
  auto operator<=>(const Range &) const = default;

};

}