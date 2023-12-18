#pragma once

#include "aoc/helpers.h"
#include <algorithm>
#include <unordered_set>
#include <iostream>

namespace aoc {

    struct point {
        int32_t x{0};
        int32_t y{0};

        constexpr inline point() noexcept = default;

        constexpr inline point(int32_t x, int32_t y) noexcept
            : x{x}
            , y{y}
        { }

        constexpr inline point(point const&) noexcept = default;
        constexpr inline point(point &&) noexcept = default;
        constexpr inline point &operator=(point const&) noexcept = default;
        constexpr inline point &operator=(point &&) noexcept = default;

        constexpr inline point &operator+=(point const& o) noexcept {
            x += o.x;
            y += o.y;
            return *this;
        }

        constexpr inline point &operator-=(point const& o) noexcept {
            x -= o.x;
            y -= o.y;
            return *this;
        }

        constexpr inline point &operator*=(int32_t v) noexcept {
            x *= v;
            y *= v;
            return *this;
        }

        constexpr inline point &operator/=(int32_t v) noexcept {
            x /= v;
            y /= v;
            return *this;
        }

        constexpr inline point operator+(point const& o) const noexcept {
            return point{x + o.x, y + o.y};
        }

        constexpr inline point operator-(point const& o) const noexcept {
            return point{x - o.x, y - o.y};
        }

        constexpr inline point operator*(int32_t v) const noexcept {
            return point{x * v, y * v};
        }

        constexpr inline point operator/(int32_t v) const noexcept {
            return point{x / v, y / v};
        }

        friend constexpr inline bool operator==(aoc::point const& l, aoc::point const& r);
        friend constexpr inline bool operator!=(aoc::point const& l, aoc::point const& r);
        friend constexpr inline bool operator<(aoc::point const& l, aoc::point const& r);
        friend constexpr inline bool operator>(aoc::point const& l, aoc::point const& r);
        friend constexpr inline bool operator<=(aoc::point const& l, aoc::point const& r);
        friend constexpr inline bool operator>=(aoc::point const& l, aoc::point const& r);
        friend constexpr inline bool operator>=(aoc::point const& l, aoc::point const& r);
        friend constexpr inline int cmp(aoc::point const& l, aoc::point const& r);
        friend std::ostream& operator<<(std::ostream& os, aoc::point const &p);

        constexpr inline point sgn() const noexcept {
            return point{aoc::sgn(x), aoc::sgn(y)};
        }

        inline int32_t abs() const noexcept {
            return std::abs(x) + std::abs(y);
        }

        inline int32_t manhattan(point const& o) const noexcept {
            return (*this - o).abs();
        }

        static constexpr inline point right() noexcept {
            return {1, 0};
        }

        static constexpr inline point up() noexcept {
            return {0, -1};
        }

        static constexpr inline point left() noexcept {
            return {-1, 0};
        }

        static constexpr inline point down() noexcept {
            return {0, 1};
        }

        static constexpr inline point origin() noexcept {
            return {0, 0};
        }

        static constexpr inline point step(CardinalDirection dir) noexcept {
            switch (dir) {
                case CardinalDirection::North:
                    return up();
                case CardinalDirection::NorthEast:
                    return up() + right();
                case CardinalDirection::East:
                    return right();
                case CardinalDirection::SouthEast:
                    return down() + right();
                case CardinalDirection::South:
                    return down();
                case CardinalDirection::SouthWest:
                    return down() + left();
                case CardinalDirection::West:
                    return left();
                case CardinalDirection::NorthWest:
                    return up() + left();
            }
            return origin();
        }
    };

    struct point_hash {
        std::size_t operator() (point const& v) const {
            std::size_t seed = 0;
            hash_combine(seed, v.x);
            hash_combine(seed, v.y);
            return seed;
        }
    };

    using PointSet = std::unordered_set<point, point_hash>;

    // Same comparison semantics as a std::pair
    constexpr inline bool operator<(aoc::point const& l, aoc::point const& r) {
        return l.x < r.x ? true :
            (r.x < l.x ? false :
            (l.y < r.y ? true : false));
    }
    constexpr inline bool operator<=(aoc::point const& l, aoc::point const& r) {
        return !(r < l);
    }
    constexpr inline bool operator>(aoc::point const& l, aoc::point const& r) {
        return r < l;
    }
    constexpr inline bool operator>=(aoc::point const& l, aoc::point const& r) {
        return !(l < r);
    }
    constexpr inline bool operator==(aoc::point const& l, aoc::point const& r) {
        return l.x == r.x && l.y == r.y;
    }
    constexpr inline bool operator!=(aoc::point const& l, aoc::point const& r) {
        return l.x != r.x || l.y != r.y;
    }
    constexpr inline int cmp(aoc::point const& l, aoc::point const& r) {
        return l < r ? -1 : // less
            (r < l ? 1 :    // greater
            0);             // equivalent
    }
    std::ostream& operator<<(std::ostream& os, aoc::point const &p) {
        os << "{ " << p.x << ", " << p.y << " }";
        return os;
    }

    enum class PerimiterMode {
        None,
        Include,
        Exclude,
    };

    template<class InputIt>
    int64_t area(InputIt start, InputIt end, PerimiterMode mode = PerimiterMode::None) {
        // calculate the area of the enclosed path.
        int64_t area{};
        int64_t perimeter{};
        while (start != end && (start + 1) != end) {
            auto& a = *start;
            auto& b = *(start + 1);
            area += a.x * b.y - a.y * b.x;
            perimeter += a.manhattan(b);
            start++;
        }

        switch (mode) {
            case PerimiterMode::None:
                break;
            case PerimiterMode::Include:
                area += perimeter;
                break;
            case PerimiterMode::Exclude:
                area -= perimeter;
                break;
        }
        area = (std::abs(area)) / 2;
        return area + 1;
    }
}


