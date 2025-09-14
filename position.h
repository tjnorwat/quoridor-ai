#pragma once

#include "types.h"
#include "bitboard.h"
#include <iostream>

struct Position {
    Square pawn[COLOR_NB];
    uint16_t num_walls[COLOR_NB];

    Color side_to_move;

    Bitboard h_walls_idxs;
    Bitboard v_walls_idxs;

    Bitboard h_walls_full;
    Bitboard v_walls_full;

    Position();

    void do_move(Move move);
    void undo_move(Move move);
    bool is_terminal() const;
    void print_board() const;
};
