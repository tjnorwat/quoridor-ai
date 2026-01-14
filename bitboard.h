#pragma once

#include "types.h"
#include <iostream>

void init();

struct Bitboard {
    uint64_t lower; // squares 0–63
    uint64_t upper; // squares 64–80

    operator bool() const {
        return lower | upper;
    }
};

// Counts total number of set bits in the board
inline int popcount(const Bitboard& b) {
    return __builtin_popcountll(b.lower) + __builtin_popcountll(b.upper);
}

// Finds and clears the least significant bit
constexpr inline Square pop_lsb(Bitboard& b) {
    if (b.lower) {
        const Square s = Square(__builtin_ctzll(b.lower));
        b.lower &= b.lower - 1;
        return s;
    }

    const Square s = Square(__builtin_ctzll(b.upper) + 64);
    b.upper &= b.upper - 1;
    return s;
}

constexpr inline Bitboard operator|(const Bitboard& b1, const Bitboard& b2) { return Bitboard{b1.lower | b2.lower, b1.upper | b2.upper}; }
constexpr inline Bitboard operator&(const Bitboard& b1, const Bitboard& b2) { return Bitboard{b1.lower & b2.lower, b1.upper & b2.upper}; }
constexpr inline Bitboard operator^(const Bitboard& b1, const Bitboard& b2) { return Bitboard{b1.lower ^ b2.lower, b1.upper ^ b2.upper}; }
constexpr inline Bitboard operator~(const Bitboard& b) { return Bitboard{~b.lower, ~b.upper}; }
constexpr inline bool operator!(const Bitboard &b) { return !(b.lower | b.upper); }

constexpr inline Bitboard& operator|=(Bitboard& b1, const Bitboard& b2) {
    b1.lower |= b2.lower;
    b1.upper |= b2.upper;
    return b1;
}

constexpr inline Bitboard& operator&=(Bitboard& b1, const Bitboard& b2) {
    b1.lower &= b2.lower;
    b1.upper &= b2.upper;
    return b1;
}

constexpr inline Bitboard& operator^=(Bitboard& b1, const Bitboard& b2) {
    b1.lower ^= b2.lower;
    b1.upper ^= b2.upper;
    return b1;
}

// have to account for wrapping lower to upper
constexpr inline Bitboard operator<<(const Bitboard &b, const uint32_t &shift) {
    if (shift >= 64)
        return Bitboard{0ULL, b.lower << (shift - 64)};
    
    const uint64_t temp = shift == 0 ? 0ULL : b.lower >> (64 - shift);
    return Bitboard{b.lower << shift, (b.upper << shift) | temp};
}

constexpr inline Bitboard operator>>(const Bitboard &b, const uint32_t &shift) {
    if (shift >= 64)
        return Bitboard{b.upper >> (shift - 64), 0ULL};
    
    const uint64_t temp = shift == 0 ? 0ULL : b.upper << (64 - shift);
    return Bitboard{(b.lower >> shift) | temp, b.upper >> shift};
}

template<Direction D>
constexpr Bitboard shift(Bitboard b) {
    return D == NORTH ? b << 9 :
           D == SOUTH ? b >> 9 :
           D == EAST  ? b << 1 :
           D == WEST  ? b >> 1 :
           Bitboard{0ULL, 0ULL};
}

constexpr Bitboard square_bb(Square s) {
    return s < 64 ? Bitboard{1ULL << s, 0ULL} : Bitboard{0ULL, 1ULL << (s - 64)};
}

constexpr Square bb_square(Bitboard bb) {
    // making sure that there is only one bit set in the bb
    return pop_lsb(bb);
}

constexpr Bitboard operator&(Bitboard b, Square s) { return b & square_bb(s); }
constexpr Bitboard operator|(Bitboard b, Square s) { return b | square_bb(s); }
constexpr Bitboard operator^(Bitboard b, Square s) { return b ^ square_bb(s); }

constexpr Bitboard& operator&=(Bitboard& b, Square s) { return b &= square_bb(s); }
constexpr Bitboard& operator|=(Bitboard& b, Square s) { return b |= square_bb(s); }
constexpr Bitboard& operator^=(Bitboard& b, Square s) { return b ^= square_bb(s); }

extern Bitboard PawnAttacks[SQ_NB];
extern Bitboard ValidWalls;
extern Bitboard ValidSquares;
extern Bitboard GoalMask[COLOR_NB];

void print_bitboard(Bitboard b);