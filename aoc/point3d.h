#pragma once

#include <numeric>
#include <cstdint>
#include <cmath>

namespace aoc {

    template <typename T>
    class point3d
    {
    public:
    constexpr point3d() noexcept = default;
    constexpr point3d(T xv, T yv, T zv)
        : x(xv)
        , y(yv)
        , z(zv)
        { }

    explicit constexpr point3d(T v)
        : x(v)
        , y(v)
        , z(v)
        { }

    T LengthSqr() const
        { return x * x + y * y + z * z; }

    T Length() const requires (std::floating_point<T>)
        { return std::sqrt(LengthSqr()); }

    T MinComponent() const
        { return std::min({x, y, z}); }

    T MaxComponent() const
        { return std::max({x, y, z}); }

    T &operator[](size_t i)
    {
        return (&x)[i];
    }

    T operator[](size_t i) const
    {
        return (&x)[i];
    }

    constexpr bool operator==(const point3d &) const = default;
    constexpr auto operator<=>(const point3d &) const = default;

    point3d operator-() const requires (std::signed_integral<T> || std::floating_point<T>)
        { return { -x, -y, -z }; }

    point3d operator+(point3d other) const
        { return { x + other.x, y + other.y, z + other.z }; }

    point3d operator-(point3d other) const
        { return { x - other.x, y - other.y, z - other.z }; }

    point3d operator*(T v) const
        { return { x * v, y * v, z * v }; }

    point3d operator*(point3d v) const
        { return { x * v.x, y * v.y, z * v.z }; }

    point3d operator/(T v) const
        { return { x / v, y / v, z / v }; }

    point3d operator/(point3d v) const
        { return { x / v.x, y / v.y, z / v.z }; }

    point3d &operator+=(point3d v)
        { *this = *this + v; return *this; }

    point3d &operator-=(point3d v)
        { *this = *this - v; return *this; }

    point3d &operator*=(T v)
        { *this = *this * v; return *this; }

    point3d &operator*=(point3d v)
        { *this = *this * v; return *this; }

    point3d &operator/=(T v)
        { *this = *this / v; return *this; }

    point3d &operator/=(point3d v)
        { *this = *this / v; return *this; }

    T x = T(0);
    T y = T(0);
    T z = T(0);
    };

    using point3d64 = point3d<int64_t>;
}
