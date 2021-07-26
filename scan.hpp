#pragma once

#include "SquareIterator.hpp"
#include "BitboardOps.hpp"

using Bitboard = unsigned long long;

auto scan_forward(Bitboard bb) {
    return sqgen::Range<Square, sqgen::Forward>(bb);
}

auto scan_reversed(Bitboard bb) {
    return sqgen::Range<Square, sqgen::Reverse>(bb);
}

auto _carry_rippler(Bitboard mask) {
    return sqgen::CRRange(mask);
}

constexpr Bitboard _edges(Square square);

template<class IterableOfInt> constexpr auto _sliding_attacks(Square square, Bitboard occupied, IterableOfInt deltas)->Bitboard;

template <typename IterableOfInt>
constexpr auto _attack_table(IterableOfInt deltas) {
    std::vector<Bitboard> mask_table;
    std::vector<std::unordered_map<Bitboard, Bitboard>> attack_table;

    for (auto square : SQUARES) {
        std::unordered_map<Bitboard, Bitboard> attacks;

        auto mask = _sliding_attacks(square, 0, deltas) & ~_edges(square);
        for (auto subset : _carry_rippler(mask))
            attacks[subset] = _sliding_attacks(square, subset, deltas);

        attack_table.push_back(attacks);
        mask_table.push_back(mask);
    }
    return std::make_pair(mask_table, attack_table);
}