#pragma once

#include "aoc/helpers.h"
#include "aoc/point.h"

namespace aoc {

    template<typename T, ssize_t PadBy = 8>
    class PaddedMatrix {
        const ssize_t padding;
        std::vector<T> data;
        ssize_t width;
        ssize_t height;
        ssize_t padded_width() const { return width + 2 * padding; }
        ssize_t padded_height() const { return height + 2 * padding; }

    public:

        PaddedMatrix()
            : padding(PadBy)
            , width(0)
            , height(0)
        { }
        PaddedMatrix(ssize_t w, ssize_t h)
            : PaddedMatrix()
        { 
            set_width(w);
            for (ssize_t i = 0; i < h; i++) {
                add_row();
            }
            fill_pddding(T());
        }

        PaddedMatrix(const PaddedMatrix<T, PadBy>& other) = default;

        PaddedMatrix<T, PadBy>& operator=(const PaddedMatrix<T, PadBy>& other) {
            if (this == &other) {
                return *this;
            }
            width = other.width;
            height = other.height;
            data = other.data;
            return *this;
        }

        void fill_pddding(const T& value) {
            for (ssize_t y = 0; y < padded_height(); y++) {
                for (ssize_t x = 0; x < padded_width(); x++) {
                    if (y < padding || y >= padded_height() - padding || x < padding || x >= padded_width() - padding) {
                        data.at(raw_offset(x, y)) = value;
                    }
                }
            }
        }

        typename std::vector<T>::const_reference at(ssize_t x, ssize_t y) const {
            return data[offset(x, y)];
        }

        typename std::vector<T>::const_reference at(const point& p) const {
            return at(p.x, p.y);
        }

        typename std::vector<T>::const_reference at(const Point& p) const {
            return at(p.first, p.second);
        }

        ssize_t get_width() const {
            return width;
        }
        ssize_t get_height() const {
            return height;
        }

        void set_width(ssize_t w) {
            width = w;
        }

        void set_height(ssize_t h) {
            height = h;
            data.resize(padded_width() * padded_height());
            fill_pddding(T());
        }

        ssize_t add_row() {
            height++;
            data.resize(padded_width() * padded_height());
            return height - 1;
        }

        void add(ssize_t x, ssize_t y, const T& value) {
            const auto p = offset(x, y);
            data.at(p) = value;
        }

        void add(aoc::point p, const T& value) {
            add(p.x, p.y, value);
        }

        ssize_t offset(ssize_t x, ssize_t y) const {
            return (y + padding) * padded_width() + x + padding;
        }

        ssize_t raw_offset(ssize_t x, ssize_t y) const {
            return y * padded_width() + x;
        }
    };

} // namespace aoc