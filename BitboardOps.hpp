#pragma once

#include "BitboardNames.hpp"
#include "scan.hpp"

using Bitboard = unsigned long long;

constexpr auto lsb(Bitboard bb) -> int {
    return __builtin_ctzll(bb);
}

constexpr auto msb(Bitboard bb) -> int {
    return __builtin_clzll(bb);
}

constexpr auto popcount(Bitboard bb) -> int {
    return __builtin_popcountll(bb);
}

constexpr auto flip_vertical(Bitboard bb) -> Bitboard {
    // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#FlipVertically
    bb = ((bb >> 8) & 0x00ff00ff00ff00ff) | ((bb & 0x00ff00ff00ff00ff) << 8);
    bb = ((bb >> 16) & 0x0000ffff0000ffff) | ((bb & 0x0000ffff0000ffff) << 16);
    bb = (bb >> 32) | ((bb & 0x00000000ffffffff) << 32);
    return bb;
}

constexpr auto flip_horizontal(Bitboard bb) -> Bitboard {
    // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#MirrorHorizontally
    bb = ((bb >> 1) & 0x5555555555555555) | ((bb & 0x5555555555555555) << 1);
    bb = ((bb >> 2) & 0x3333333333333333) | ((bb & 0x3333333333333333) << 2);
    bb = ((bb >> 4) & 0x0f0f0f0f0f0f0f0f) | ((bb & 0x0f0f0f0f0f0f0f0f) << 4);
    return bb;
}

constexpr auto flip_diagonal(Bitboard bb) -> Bitboard {
    /// https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#FlipabouttheDiagonal
    Bitboard t = (bb ^ (bb << 28)) & 0x0f0f0f0f00000000;
    bb = bb ^ (t ^ (t >> 28));
    t = (bb ^ (bb << 14)) & 0x3333000033330000;
    bb = bb ^ (t ^ (t >> 14));
    t = (bb ^ (bb << 7)) & 0x5500550055005500;
    bb = bb ^ (t ^ (t >> 7));
    return bb;
}

constexpr auto flip_anti_diagonal(Bitboard bb) -> Bitboard {
    // https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#FlipabouttheAntidiagonal
    Bitboard t = bb ^ (bb << 36);
    bb = bb ^ ((t ^ (bb >> 36)) & 0xf0f0f0f00f0f0f0f);
    t = (bb ^ (bb << 18)) & 0xcccc0000cccc0000;
    bb = bb ^ (t ^ (t >> 18));
    t = (bb ^ (bb << 9)) & 0xaa00aa00aa00aa00;
    bb = bb ^ (t ^ (t >> 9));
    return bb;
}

constexpr auto shift_down(Bitboard b) -> Bitboard {
    return b >> 8;
}
constexpr auto shift_2_down(Bitboard b) -> Bitboard {
    return b >> 16;
}
constexpr auto shift_up(Bitboard b) -> Bitboard {
    return (b << 8) & BB_ALL;
}
constexpr auto shift_2_up(Bitboard b) -> Bitboard {
    return (b << 16) & BB_ALL;
}
constexpr auto shift_right(Bitboard b) -> Bitboard {
    return (b << 1) & ~BB_FILE_A & BB_ALL;
}
constexpr auto shift_2_right(Bitboard b) -> Bitboard {
    return (b << 2) & ~BB_FILE_A & ~BB_FILE_B & BB_ALL;
}
constexpr auto shift_left(Bitboard b) -> Bitboard {
    return (b >> 1) & ~BB_FILE_H;
}
constexpr auto shift_2_left(Bitboard b) -> Bitboard {
    return (b >> 2) & ~BB_FILE_G & ~BB_FILE_H;
}
constexpr auto shift_up_left(Bitboard b) -> Bitboard {
    return (b << 7) & ~BB_FILE_H & BB_ALL;
}
constexpr auto shift_up_right(Bitboard b) -> Bitboard {
    return (b << 9) & ~BB_FILE_A & BB_ALL;
}
constexpr auto shift_down_left(Bitboard b) -> Bitboard {
    return (b >> 9) & ~BB_FILE_H;
}
constexpr auto shift_down_right(Bitboard b) -> Bitboard {
    return (b >> 7) & ~BB_FILE_A;
}

constexpr auto square_file(Square square) noexcept -> int {
    // """Gets the file index of the square where ``0`` is the a-file."""
    return (int)square & 7;
}

constexpr auto square_rank(Square square) noexcept -> int {
    // """Gets the rank index of the square where ``0`` is the first rank."""
    return (int)square >> 3;
}

constexpr auto square_distance(Square a, Square b) noexcept -> int {
    // """
    // Gets the distance (i.e., the number of king steps) from square *a* to *b*.
    // """
    return std::max(
        std::abs(square_file(a) - square_file(b)),
        std::abs(square_rank(a) - square_rank(b)));
}

template <typename IterableOfInt>
constexpr auto _sliding_attacks(Square square, Bitboard occupied, IterableOfInt deltas) -> Bitboard {
    Bitboard attacks = BB_EMPTY;

    for (int delta : deltas) {
        int sq = (int)square;

        while (true) {
            sq += delta;
            if (!(0 <= sq && sq < 64) || square_distance((Square)sq, (Square)(sq - delta)) > 2)
                break;

            attacks |= BB_SQUARES[sq];

            if (occupied & BB_SQUARES[sq])
                break;
        }
    }
    return attacks;
}

template <typename IterableOfInt>
constexpr auto _step_attacks(Square square, IterableOfInt deltas) -> Bitboard {
    return _sliding_attacks(square, BB_ALL, deltas);
}

constexpr std::array _KNIGHT_OFFSETS = {17, 15, 10, 6, -17, -15, -10, -6};
constexpr auto _make_knight_move(int sq) {
    return _step_attacks((Square)sq, _KNIGHT_OFFSETS);
}
constexpr auto BB_KNIGHT_ATTACKS = cag::make_array<64>(_make_knight_move);
constexpr std::array _KING_OFFSETS = {9, 8, 7, 1, -9, -8, -7, -1};
constexpr auto _make_king_move(int sq) {
    return _step_attacks((Square)sq, _KING_OFFSETS);
}
constexpr auto BB_KING_ATTACKS = cag::make_array<64>(
    _make_king_move);

// BB_PAWN_ATTACKS is done as a literal in "BitboardNames.hpp"

constexpr auto square(int file_index, int rank_index) noexcept -> Square {
    // """Gets a square number by file and rank index."""
    return (Square)(rank_index * 8 + file_index);
}

constexpr auto square_mirror(Square square) noexcept -> Square {
    // """Mirrors the square vertically."""
    return (Square)((int)square ^ 0x38);
}

constexpr auto _edges(Square square) -> Bitboard {
    return (((BB_RANK_1 | BB_RANK_8) & ~BB_RANKS[square_rank(square)]) |
            ((BB_FILE_A | BB_FILE_H) & ~BB_FILES[square_file(square)]));
}

constexpr std::array _DIAG = {-9, -7, 7, 9};
const auto _DIAG_PAIR = _attack_table(_DIAG);
const auto BB_DIAG_MASKS = _DIAG_PAIR.first;
const auto BB_DIAG_ATTACKS = _DIAG_PAIR.second;

constexpr std::array _FILE = {-8, 8};
const auto _FILE_PAIR = _attack_table(_FILE);
const auto BB_FILE_MASKS = _FILE_PAIR.first;
const auto BB_FILE_ATTACKS = _FILE_PAIR.second;

constexpr std::array _RANK = {-1, 1};
const auto _RANK_PAIR = _attack_table(_RANK);
const auto BB_RANK_MASKS = _RANK_PAIR.first;
const auto BB_RANK_ATTACKS = _RANK_PAIR.second;

auto _rays() {
    std::array<std::array<Bitboard, 64>, 64> rays;
    int a = 0;
    for (auto bb_a : BB_SQUARES) {
        auto& rays_row = rays[a];
        int b = 0;
        for (auto bb_b : BB_SQUARES) {
            if (BB_DIAG_ATTACKS[a].at(0) & bb_b) {
                rays_row[b] = ((BB_DIAG_ATTACKS[a].at(0) & BB_DIAG_ATTACKS[b].at(0)) | bb_a | bb_b);
            } else if (BB_RANK_ATTACKS[a].at(0) & bb_b) {
                rays_row[b] = (BB_RANK_ATTACKS[a].at(0) | bb_a);
            } else if (BB_FILE_ATTACKS[a].at(0) & bb_b) {
                rays_row[b] = (BB_FILE_ATTACKS[a].at(0) | bb_a);
            } else {
                rays_row[b] = (BB_EMPTY);
            }
            ++b;
        }
        ++a;
    }
    return rays;
}

const auto BB_RAYS = _rays();

constexpr auto ray(Square a, Square b) -> Bitboard {
    return BB_RAYS[a][b];
}

constexpr auto between(Square a, Square b) -> Bitboard {
    auto bb = BB_RAYS[a][b] & ((BB_ALL << a) ^ (BB_ALL << b));
    return bb & (bb - 1);
}