#pragma once

#include "aoc/helpers.h"
#include "aoc/point.h"

namespace aoc {

    template<typename T, ssize_t PadBy = 8>
    class PaddedMatrix {
        const ssize_t padding{PadBy};
        std::vector<T> data;
        ssize_t width{0};
        ssize_t height{0};

        ssize_t padded_width() const { return width + 2 * padding; }
        ssize_t padded_height() const { return height + 2 * padding; }

    public:

        void fill_pddding(const T& value) {
            for (ssize_t y = 0; y < padded_height(); y++) {
                for (ssize_t x = 0; x < padded_width(); x++) {
                    if (y < padding || y >= padded_height() - padding || x < padding || x >= padded_width() - padding) {
                        data.at(raw_offset(x, y)) = value;
                    }
                }
            }
        }

        const T& at(ssize_t x, ssize_t y) const {
            return data.at(offset(x, y));
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

        ssize_t add_row() {
            height++;
            data.resize(padded_width() * padded_height());
            return height - 1;
        }

        void add(ssize_t x, ssize_t y, const T& value) {
            const auto p = offset(x, y);
            data.at(p) = value;
        }

        ssize_t offset(ssize_t x, ssize_t y) const {
            return (y + padding) * padded_width() + x + padding;
        }

        ssize_t raw_offset(ssize_t x, ssize_t y) const {
            return y * padded_width() + x;
        }
    };

} // namespace aoc