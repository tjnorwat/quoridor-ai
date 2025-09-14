#pragma once

#include <array>
#include <cstdint>
#include <iostream>

enum Square : int16_t {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1, SQ_I1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2, SQ_I2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3, SQ_I3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4, SQ_I4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5, SQ_I5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6, SQ_I6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7, SQ_I7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8, SQ_I8,
    SQ_A9, SQ_B9, SQ_C9, SQ_D9, SQ_E9, SQ_F9, SQ_G9, SQ_H9, SQ_I9,
    SQ_NB = 81,
    SQ_NONE = 255
};

enum MoveType {
    PAWN,
    H_WALL,
    V_WALL,
    MOVE_TYPE_NB = 3
};

// maybe dont need diagonals
enum Direction : int16_t { 
    NORTH = 9,
    EAST = 1,
    SOUTH = -NORTH,
    WEST = -EAST,
    
    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST
};

enum Color : int16_t {
    WHITE,
    BLACK,
    COLOR_NB = 2
};

constexpr Color operator~(Color c) { return Color(c ^ BLACK); }


enum File : int16_t {
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,
    FILE_I,
    FILE_NB = 9
};


enum Rank : int16_t {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_9,
    RANK_NB = 9
};

#define ENABLE_INCR_OPERATORS_ON(T) \
    constexpr T& operator++(T& d) { return d = T(int(d) + 1); } \
    constexpr T& operator--(T& d) { return d = T(int(d) - 1); }
    
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)

#undef ENABLE_INCR_OPERATORS_ON

constexpr Square make_square(Rank r, File f) { return Square(r * 9 + f); }
constexpr Rank rank_of(Square s) { return Rank(s / 9); }
constexpr File file_of(Square s) { return File(s % 9); }

constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d));}
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d));}

struct Move {
    Square from;
    Square to;
    MoveType type;
    
    Move() = default;
    
    bool operator==(const Move& other) const {
        return from == other.from && to == other.to && type == other.type;
    }
    bool operator!=(const Move& other) const {
        return !(*this == other);
    }

    void print_move() const {
        if (type == PAWN) {
            std::cout << "Pawn move from " << from << " to " << to << "\n";
        } else if (type == H_WALL) {
            std::cout << "Horizontal wall at " << from << "\n";
        } else if (type == V_WALL) {
            std::cout << "Vertical wall at " << from << "\n";
        }
    }
};