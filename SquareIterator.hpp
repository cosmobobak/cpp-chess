#pragma once

#include <iterator>
#include "BitboardNames.hpp"
#include "ConstexprArrayGenerator.hpp"

namespace sqgen {

constexpr auto Forward = true;
constexpr auto Reverse = false;

template <typename Square, bool Direction>
class SquareIterator {
    unsigned long long bb;
   public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Square;
    using pointer = value_type*;
    using reference = value_type&;

    SquareIterator(unsigned long long bb) {
        this->bb = bb;
    }
    SquareIterator() {
        this->bb = BB_EMPTY;
    }

    auto operator*() -> value_type {
        if constexpr (Direction == Forward) {
            return (Square)__builtin_ctzll(bb);
        } else {
            return (Square)(63 - __builtin_clzll(bb));
        }
    }

    auto peek() -> value_type {
        return operator*();
    }

    // Prefix increment
    inline SquareIterator& operator++() {
        if constexpr (Direction == Forward) {
            bb &= bb - 1;
        } else {
            bb ^= BB_SQUARES[63 - __builtin_clzll(bb)];
        }
        
        return *this;
    }

    // Postfix increment
    inline SquareIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    auto next() {
        ++(*this);
        return peek();
    }

    friend bool operator==(const SquareIterator& a, const SquareIterator& b) {
        return a.bb == b.bb;
    };

    friend bool operator!=(const SquareIterator& a, const SquareIterator& b) {
        return a.bb != b.bb;
    };

    static SquareIterator sentinel() {
        return SquareIterator(BB_EMPTY);
    }
};

template <typename Square, bool Direction>
class Range {
    unsigned long long bb;
   public:
    Range(unsigned long long bb) {
        this->bb = bb;
    }

    auto begin() {
        return SquareIterator<Square, Direction>(bb);
    }

    auto end() {
        return SquareIterator<Square, Direction>::sentinel();
    }

    auto cbegin() const {
        return SquareIterator<Square, Direction>(bb);
    }

    auto cend() const {
        return SquareIterator<Square, Direction>::sentinel();
    }
};

class CRGenerator {
    unsigned long long subset;
    unsigned long long mask;
    bool END_SENTINEL = false;
   public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = unsigned long long;
    using pointer = value_type*;
    using reference = value_type&;

    CRGenerator(unsigned long long mask) {
        this->subset = BB_EMPTY;
        this->mask = mask;
    }

    auto operator*() -> value_type {
        return subset;
    }

    auto peek() -> value_type {
        return operator*();
    }

    // Prefix increment
    inline CRGenerator& operator++() {
        subset = (subset - mask) & mask;
        if (!subset)
            END_SENTINEL = true;
        return *this;
    }

    // Postfix increment
    inline CRGenerator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    auto next() {
        ++(*this);
        return peek();
    }

    static auto end_sentinel() {
        auto out = CRGenerator(BB_EMPTY);
        out.END_SENTINEL = true;
        return out;
    }

    friend bool operator==(const CRGenerator& a, const CRGenerator& b) {
        return a.END_SENTINEL == b.END_SENTINEL;
    };

    friend bool operator!=(const CRGenerator& a, const CRGenerator& b) {
        return a.END_SENTINEL != b.END_SENTINEL;
    };
};

class CRRange {
    unsigned long long mask;
   public:
    CRRange(unsigned long long mask) {
        this->mask = mask;
    }

    auto begin() {
        return CRGenerator(mask);
    }

    auto end() {
        return CRGenerator::end_sentinel();
    }
};
}  // namespace sqgen