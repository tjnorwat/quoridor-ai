#pragma once

#include <algorithm>
#include <limits>
#include "types.h"
#include "position.h"


Move* generate(const Position& pos, Move* moveList);
Move* generate_pawn_moves(const Position& pos, Move* moveList);
Move* generate_wall_moves(const Position& pos, Move* moveList);
bool reachable_any_goal(const Position& pos, Square start, Bitboard goal_mask);
bool reachable_any_goal_slow(const Position& pos, Square start, Bitboard goal_mask);


int distance_to_goal(const Position& pos, Color c);

Move* splat_pawn_moves(Move* moveList, Square from, Bitboard to_bb);
Move* splat_wall_moves(Move* moveList, Bitboard wall_bb, MoveType type);

struct MoveList {
    Move moves[256];
    Move *last;

    explicit MoveList(const Position& pos) : last(generate(pos, moves)) {}
    const Move* begin() const { return moves; }
    const Move* end() const { return last; }
    size_t size() const { return last - moves; }
    bool empty() const { return last == moves; }
    bool contains(Move move) const {return std::find(begin(), end(), move) != end(); }
};