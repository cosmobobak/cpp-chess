#pragma once

#include <sstream>

// #include "names.hpp"
#include "BitboardNames.hpp"
#include "BitboardOps.hpp"
#include "scan.hpp"

using namespace std::literals;

class SquareSet {
    // """
    // A set of squares.

    // >>> import chess
    // >>>
    // >>> squares = chess.SquareSet([chess.A8, chess.A1])
    // >>> squares
    // SquareSet(0x0100_0000_0000_0001)

    // >>> squares = chess.SquareSet(chess.BB_A8 | chess.BB_RANK_1)
    // >>> squares
    // SquareSet(0x0100_0000_0000_00ff)

    // >>> print(squares)
    // 1 . . . . . . .
    // . . . . . . . .
    // . . . . . . . .
    // . . . . . . . .
    // . . . . . . . .
    // . . . . . . . .
    // . . . . . . . .
    // 1 1 1 1 1 1 1 1

    // >>> len(squares)
    // 9

    // >>> bool(squares)
    // True

    // >>> chess.B1 in squares
    // True

    // >>> for square in squares:
    // ...     # 0 -- chess.A1
    // ...     # 1 -- chess.B1
    // ...     # 2 -- chess.C1
    // ...     # 3 -- chess.D1
    // ...     # 4 -- chess.E1
    // ...     # 5 -- chess.F1
    // ...     # 6 -- chess.G1
    // ...     # 7 -- chess.H1
    // ...     # 56 -- chess.A8
    // ...     print(square)
    // ...
    // 0
    // 1
    // 2
    // 3
    // 4
    // 5
    // 6
    // 7
    // 56

    // >>> list(squares)
    // [0, 1, 2, 3, 4, 5, 6, 7, 56]

    // Square sets are internally represented by 64-bit integer masks of the
    // included squares. Bitwise operations can be used to compute unions,
    // intersections and shifts.

    // >>> int(squares)
    // 72057594037928191

    // Also supports common set operations like
    // :func:`~chess.SquareSet.issubset()`, :func:`~chess.SquareSet.issuperset()`,
    // :func:`~chess.SquareSet.union()`, :func:`~chess.SquareSet.intersection()`,
    // :func:`~chess.SquareSet.difference()`,
    // :func:`~chess.SquareSet.symmetric_difference()` and
    // :func:`~chess.SquareSet.copy()` as well as
    // :func:`~chess.SquareSet.update()`,
    // :func:`~chess.SquareSet.intersection_update()`,
    // :func:`~chess.SquareSet.difference_update()`,
    // :func:`~chess.SquareSet.symmetric_difference_update()` and
    // :func:`~chess.SquareSet.clear()`.
    // """

    using Bitboard = unsigned long long;

    Bitboard mask;

   public:
    SquareSet(Bitboard squares = BB_EMPTY) {
        mask = squares;
    }
    template <typename SquareRange>
    SquareSet(SquareRange squares) {
        // # Try squares as an iterable. Not under except clause for nicer
        // # backtraces.
        for (Square square : squares)  // type: ignore
            add(square);
    }
    // # Set

    auto __contains__(Square square) -> bool {
        return (bool)(BB_SQUARES[square] & mask);
    }
    auto contains(Square square) -> bool {
        return (bool)(BB_SQUARES[square] & mask);
    }
    auto __iter__() const {
        return scan_forward(mask);
    }
    auto begin() const {
        return scan_forward(mask).begin();
    }
    auto end() const {
        return scan_forward(mask).end();
    }
    auto cbegin() const {
        return scan_forward(mask).cbegin();
    }
    auto cend() const {
        return scan_forward(mask).cend();
    }
    auto __reversed__() const {
        return scan_reversed(mask);
    }
    auto rbegin() const {
        return scan_reversed(mask).begin();
    }
    auto rend() const {
        return scan_reversed(mask).end();
    }
    auto crbegin() const {
        return scan_reversed(mask).cbegin();
    }
    auto crend() const {
        return scan_reversed(mask).cend();
    }
    auto __len__() -> int {
        return popcount(mask);
    }
    auto size() -> int {
        return popcount(mask);
    }
    // # MutableSet

    void add(Square square) {
        // """Adds a square to the set."""
        mask |= BB_SQUARES[square];
    }

    void discard(Square square) {
        // """Discards a square from the set."""
        mask &= ~BB_SQUARES[square];
    }

    // # frozenset

    auto isdisjoint(const SquareSet& other) const -> bool {
        // """Tests if the square sets are disjoint."""
        return !(bool)(*this & other);
    }

    auto issubset(const SquareSet& other) const -> bool {
        // """Tests if this square set is a subset of another."""
        return !(bool)(~*this & other);
    }

    auto issuperset(const SquareSet& other) const -> bool {
        // """Tests if this square set is a superset of another."""
        return !(bool)(*this & ~other);
    }

    auto _union(const SquareSet& other) const -> SquareSet {
        return *this | other;
    }

    auto operator|(const SquareSet& other) const -> SquareSet {
        auto r = other;
        r.mask |= mask;
        return r;
    }

    auto intersection(const SquareSet& other) const -> SquareSet {
        return *this & other;
    }

    auto operator&(const SquareSet& other) const -> SquareSet {
        auto r = other;
        r.mask &= mask;
        return r;
    }

    auto difference(const SquareSet& other) const -> SquareSet {
        return *this - other;
    }

    auto operator-(const SquareSet& other) const -> SquareSet {
        auto r = other;
        r.mask = mask & ~r.mask;
        return r;
    }

    auto symmetric_difference(SquareSet other) const -> SquareSet {
        return *this ^ other;
    }

    auto operator^(const SquareSet& other) const -> SquareSet {
        auto r = other;
        r.mask ^= mask;
        return r;
    }

    auto copy() const -> SquareSet {
        return *this;
    }

    // # set
    auto update(const SquareSet& others) -> void {
        // for other in others
        //     *this |= other
        mask |= others.mask;
    }

    void operator|=(const SquareSet& other) {
        mask |= other.mask;
    }

    void intersection_update(const SquareSet& others) {
        // for other in others:
        //     *this &= other
        mask &= others.mask;
    }

    void operator&=(const SquareSet& other) {
        mask &= other.mask;
        // return *this;
    }

    void difference_update(const SquareSet& other) {
        *this -= other;
    }

    auto operator-=(const SquareSet& other) -> SquareSet {
        mask &= ~other.mask;
        return *this;
    }

    void symmetric_difference_update(SquareSet other) {
        *this ^= other;
    }

    auto operator^=(SquareSet other) -> void {
        mask ^= other.mask;
        // return *this
    }

    auto remove(Square square) -> void {
        // """
        // Removes a square from the set.

        // :raises: :exc:`KeyError` if the given *square* was not in the set.
        // """
        auto removal_mask = BB_SQUARES[square];
        if (mask & removal_mask) {
            mask ^= removal_mask;
        } else {
            throw std::invalid_argument(std::to_string(square));
        }
    }

    auto pop() -> Square {
        // """
        // Removes and returns a square from the set.

        // :raises: :exc:`KeyError` if the set is empty.
        // """
        if (!mask)
            std::invalid_argument("pop from empty SquareSet");

        auto square = (Square)lsb(mask);
        mask &= (mask - 1);
        return square;
    }

    auto clear() -> void {
        // """Removes all elements from this set."""
        mask = BB_EMPTY;
    }

    // # SquareSet

    auto carry_rippler() const {
        // """Iterator over the subsets of this set."""
        return _carry_rippler(mask);
    }

    auto mirror() const -> SquareSet {
        // """Returns a vertically mirrored copy of this square set."""
        return SquareSet(flip_vertical(mask));
    }

    auto tolist() const -> std::array<bool, 64> {
        // """Converts the set to a list of 64 bools."""
        auto result = std::array<bool, 64>({0});
        for (auto square : *this) {
            result[square] = true;
        }
        return result;
    }

    explicit operator bool() const {
        return (bool)(mask);
    }

    friend bool operator==(const SquareSet& a, const SquareSet& b) {
        return a.mask == b.mask;
    }
    friend bool operator!=(const SquareSet& a, const SquareSet& b) {
        return a.mask != b.mask;
    }

    auto operator<<(int shift) const -> SquareSet {
        return SquareSet(mask << shift);  // safe not to & with BB_ALL because we have fixedlen ints here
    }

    auto operator>>(int shift) const -> SquareSet {
        return SquareSet(mask >> shift);
    }

    auto operator<<=(int shift) -> void {
        mask <<= shift;
        // return *this;
    }

    auto operator>>=(int shift) -> void {
        mask >>= shift;
        // return *this;
    }

    auto operator~() const -> SquareSet {
        return SquareSet(~mask);
    }

    explicit operator Bitboard() {
        return mask;
    }

    // auto __index__() -> int:
    //     return *this.mask

    auto __repr__() -> std::string {
        return "SquareSet("s + std::to_string(mask) + ")"s;
    }

    auto __str__() -> std::string {
        std::stringstream builder;

        for (auto square : SQUARES_180) {
            auto fmask = BB_SQUARES[square];
            builder << (mask & fmask) ? "1"s : "."s;

            if (!(fmask & BB_FILE_H))
                builder << " "s;
            else if ((Square)square != H1)
                builder << "\n"s;
        }
        return builder.str();
    }

    // auto _repr_svg_(*this) -> str:
    //     import chess.svg
    //     return chess.svg.board(squares=*this, size=390)

    static auto ray(Square a, Square b) -> SquareSet {
        // """
        // All squares on the rank, file or diagonal with the two squares, if they
        // are aligned.

        // >>> import chess
        // >>>
        // >>> print(chess.SquareSet.ray(chess.E2, chess.B5))
        // . . . . . . . .
        // . . . . . . . .
        // 1 . . . . . . .
        // . 1 . . . . . .
        // . . 1 . . . . .
        // . . . 1 . . . .
        // . . . . 1 . . .
        // . . . . . 1 . .
        // """
        return SquareSet(ray(a, b));
    }

    static auto between(Square a, Square b) -> SquareSet {
        // """
        // All squares on the rank, file or diagonal between the two squares
        // (bounds not included), if they are aligned.

        // >>> import chess
        // >>>
        // >>> print(chess.SquareSet.between(chess.E2, chess.B5))
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // . . 1 . . . . .
        // . . . 1 . . . .
        // . . . . . . . .
        // . . . . . . . .
        // """
        return SquareSet(between(a, b));
    }

    static auto from_square(Square square) -> SquareSet {
        // """
        // Creates a :class:`~chess.SquareSet` from a single square.

        // >>> import chess
        // >>>
        // >>> chess.SquareSet.from_square(chess.A1) == chess.BB_A1
        // True
        // """
        return SquareSet(BB_SQUARES[square]);
    }
};