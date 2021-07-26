#pragma once

#include "ConstexprArrayGenerator.hpp"

enum Square : int {
    A1, A2, A3, A4, A5, A6, A7, A8,
    B1, B2, B3, B4, B5, B6, B7, B8,
    C1, C2, C3, C4, C5, C6, C7, C8,
    D1, D2, D3, D4, D5, D6, D7, D8,
    E1, E2, E3, E4, E5, E6, E7, E8,
    F1, F2, F3, F4, F5, F6, F7, F8,
    G1, G2, G3, G4, G5, G6, G7, G8,
    H1, H2, H3, H4, H5, H6, H7, H8,
};

constexpr std::array SQUARES = {
    Square::A1, Square::A2, Square::A3, Square::A4, Square::A5, Square::A6, Square::A7, Square::A8,
    Square::B1, Square::B2, Square::B3, Square::B4, Square::B5, Square::B6, Square::B7, Square::B8,
    Square::C1, Square::C2, Square::C3, Square::C4, Square::C5, Square::C6, Square::C7, Square::C8,
    Square::D1, Square::D2, Square::D3, Square::D4, Square::D5, Square::D6, Square::D7, Square::D8,
    Square::E1, Square::E2, Square::E3, Square::E4, Square::E5, Square::E6, Square::E7, Square::E8,
    Square::F1, Square::F2, Square::F3, Square::F4, Square::F5, Square::F6, Square::F7, Square::F8,
    Square::G1, Square::G2, Square::G3, Square::G4, Square::G5, Square::G6, Square::G7, Square::G8,
    Square::H1, Square::H2, Square::H3, Square::H4, Square::H5, Square::H6, Square::H7, Square::H8,
};

using U64 = unsigned long long;

constexpr U64 BB_EMPTY = 0ULL;
constexpr U64 BB_ALL = 0xffffffffffffffffULL;

constexpr auto BB_SQUARES = cag::make_array<64>([](int i) { return 1ULL << i; });

constexpr auto _square_mirror_INTHACK(int square) noexcept -> int {
    // """Mirrors the square vertically."""
    return (square ^ 0x38);
}
constexpr auto SQUARES_180 = cag::make_array<64>(_square_mirror_INTHACK);

constexpr U64 BB_A1 = 1ULL << 0;
constexpr U64 BB_B1 = 1ULL << 1;
constexpr U64 BB_C1 = 1ULL << 2;
constexpr U64 BB_D1 = 1ULL << 3;
constexpr U64 BB_E1 = 1ULL << 4;
constexpr U64 BB_F1 = 1ULL << 5;
constexpr U64 BB_G1 = 1ULL << 6;
constexpr U64 BB_H1 = 1ULL << 7;
constexpr U64 BB_A2 = 1ULL << 8;
constexpr U64 BB_B2 = 1ULL << 9;
constexpr U64 BB_C2 = 1ULL << 10;
constexpr U64 BB_D2 = 1ULL << 11;
constexpr U64 BB_E2 = 1ULL << 12;
constexpr U64 BB_F2 = 1ULL << 13;
constexpr U64 BB_G2 = 1ULL << 14;
constexpr U64 BB_H2 = 1ULL << 15;
constexpr U64 BB_A3 = 1ULL << 16;
constexpr U64 BB_B3 = 1ULL << 17;
constexpr U64 BB_C3 = 1ULL << 18;
constexpr U64 BB_D3 = 1ULL << 19;
constexpr U64 BB_E3 = 1ULL << 20;
constexpr U64 BB_F3 = 1ULL << 21;
constexpr U64 BB_G3 = 1ULL << 22;
constexpr U64 BB_H3 = 1ULL << 23;
constexpr U64 BB_A4 = 1ULL << 24;
constexpr U64 BB_B4 = 1ULL << 25;
constexpr U64 BB_C4 = 1ULL << 26;
constexpr U64 BB_D4 = 1ULL << 27;
constexpr U64 BB_E4 = 1ULL << 28;
constexpr U64 BB_F4 = 1ULL << 29;
constexpr U64 BB_G4 = 1ULL << 30;
constexpr U64 BB_H4 = 1ULL << 31;
constexpr U64 BB_A5 = 1ULL << 32;
constexpr U64 BB_B5 = 1ULL << 33;
constexpr U64 BB_C5 = 1ULL << 34;
constexpr U64 BB_D5 = 1ULL << 35;
constexpr U64 BB_E5 = 1ULL << 36;
constexpr U64 BB_F5 = 1ULL << 37;
constexpr U64 BB_G5 = 1ULL << 38;
constexpr U64 BB_H5 = 1ULL << 39;
constexpr U64 BB_A6 = 1ULL << 40;
constexpr U64 BB_B6 = 1ULL << 41;
constexpr U64 BB_C6 = 1ULL << 42;
constexpr U64 BB_D6 = 1ULL << 43;
constexpr U64 BB_E6 = 1ULL << 44;
constexpr U64 BB_F6 = 1ULL << 45;
constexpr U64 BB_G6 = 1ULL << 46;
constexpr U64 BB_H6 = 1ULL << 47;
constexpr U64 BB_A7 = 1ULL << 48;
constexpr U64 BB_B7 = 1ULL << 49;
constexpr U64 BB_C7 = 1ULL << 50;
constexpr U64 BB_D7 = 1ULL << 51;
constexpr U64 BB_E7 = 1ULL << 52;
constexpr U64 BB_F7 = 1ULL << 53;
constexpr U64 BB_G7 = 1ULL << 54;
constexpr U64 BB_H7 = 1ULL << 55;
constexpr U64 BB_A8 = 1ULL << 56;
constexpr U64 BB_B8 = 1ULL << 57;
constexpr U64 BB_C8 = 1ULL << 58;
constexpr U64 BB_D8 = 1ULL << 59;
constexpr U64 BB_E8 = 1ULL << 60;
constexpr U64 BB_F8 = 1ULL << 61;
constexpr U64 BB_G8 = 1ULL << 62;
constexpr U64 BB_H8 = 1ULL << 63;

constexpr U64 BB_CORNERS = BB_A1 | BB_H1 | BB_A8 | BB_H8;
constexpr U64 BB_CENTER = BB_D4 | BB_E4 | BB_D5 | BB_E5;

constexpr U64 BB_LIGHT_SQUARES = 0x55aa55aa55aa55aaULL;
constexpr U64 BB_DARK_SQUARES = 0xaa55aa55aa55aa55ULL;

constexpr auto BB_FILES = cag::make_array<8>([](int i) { return 0x0101010101010101ULL << i; });

constexpr U64 BB_FILE_A = BB_FILES.at(0);
constexpr U64 BB_FILE_B = BB_FILES.at(1);
constexpr U64 BB_FILE_C = BB_FILES.at(2);
constexpr U64 BB_FILE_D = BB_FILES.at(3);
constexpr U64 BB_FILE_E = BB_FILES.at(4);
constexpr U64 BB_FILE_F = BB_FILES.at(5);
constexpr U64 BB_FILE_G = BB_FILES.at(6);
constexpr U64 BB_FILE_H = BB_FILES.at(7);

constexpr auto BB_RANKS = cag::make_array<8>([](int i) { return 0xffULL << (8 * i); });

constexpr U64 BB_RANK_1 = BB_RANKS.at(0);
constexpr U64 BB_RANK_2 = BB_RANKS.at(1);
constexpr U64 BB_RANK_3 = BB_RANKS.at(2);
constexpr U64 BB_RANK_4 = BB_RANKS.at(3);
constexpr U64 BB_RANK_5 = BB_RANKS.at(4);
constexpr U64 BB_RANK_6 = BB_RANKS.at(5);
constexpr U64 BB_RANK_7 = BB_RANKS.at(6);
constexpr U64 BB_RANK_8 = BB_RANKS.at(7);

constexpr U64 BB_BACKRANKS = BB_RANK_1 | BB_RANK_8;
constexpr U64 BB_SECOND_RANKS = BB_RANK_2 | BB_RANK_7;
constexpr U64 BB_MIDDLE_RANKS = BB_RANK_4 | BB_RANK_5;

// this is kind of a bullshit way round to do it
constexpr U64 BB_PAWN_ATTACKS[2][64] = {
    {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 2ULL, 5ULL, 10ULL, 20ULL, 40ULL, 80ULL, 160ULL, 64ULL, 512ULL, 1280ULL, 2560ULL, 5120ULL, 10240ULL, 20480ULL, 40960ULL, 16384ULL, 131072ULL, 327680ULL, 655360ULL, 1310720ULL, 2621440ULL, 5242880ULL, 10485760ULL, 4194304ULL, 33554432ULL, 83886080ULL, 167772160ULL, 335544320ULL, 671088640ULL, 1342177280ULL, 2684354560ULL, 1073741824ULL, 8589934592ULL, 21474836480ULL, 42949672960ULL, 85899345920ULL, 171798691840ULL, 343597383680ULL, 687194767360ULL, 274877906944ULL, 2199023255552ULL, 5497558138880ULL, 10995116277760ULL, 21990232555520ULL, 43980465111040ULL, 87960930222080ULL, 175921860444160ULL, 70368744177664ULL, 562949953421312ULL, 1407374883553280ULL, 2814749767106560ULL, 5629499534213120ULL, 11258999068426240ULL, 22517998136852480ULL, 45035996273704960ULL, 18014398509481984ULL},
    {512ULL, 1280ULL, 2560ULL, 5120ULL, 10240ULL, 20480ULL, 40960ULL, 16384ULL, 131072ULL, 327680ULL, 655360ULL, 1310720ULL, 2621440ULL, 5242880ULL, 10485760ULL, 4194304ULL, 33554432ULL, 83886080ULL, 167772160ULL, 335544320ULL, 671088640ULL, 1342177280ULL, 2684354560ULL, 1073741824ULL, 8589934592ULL, 21474836480ULL, 42949672960ULL, 85899345920ULL, 171798691840ULL, 343597383680ULL, 687194767360ULL, 274877906944ULL, 2199023255552ULL, 5497558138880ULL, 10995116277760ULL, 21990232555520ULL, 43980465111040ULL, 87960930222080ULL, 175921860444160ULL, 70368744177664ULL, 562949953421312ULL, 1407374883553280ULL, 2814749767106560ULL, 5629499534213120ULL, 11258999068426240ULL, 22517998136852480ULL, 45035996273704960ULL, 18014398509481984ULL, 144115188075855872ULL, 360287970189639680ULL, 720575940379279360ULL, 1441151880758558720ULL, 2882303761517117440ULL, 5764607523034234880ULL, 11529215046068469760ULL, 4611686018427387904ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL}};
