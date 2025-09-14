#pragma once

#include "position.h"
#include "movegen.h"
#include <limits>
#include <chrono>

constexpr int WIN_SCORE = 100000;
constexpr int LOSS_SCORE = -WIN_SCORE;

constexpr int WALL_VALUE = 10; // tune experimentally

int negamax(Position& pos, int depth, int alpha, int beta, int& nodes_searched);

int negamax_root(Position& pos, int depth, int alpha, int beta, Move& best_move,
                 std::chrono::steady_clock::time_point end_time, bool& time_up, int& nodes_searched);

int iterative_deepening(Position& pos, int max_depth, int time_limit_ms, Move& best_move);

// Convenience overload if caller does not need the move
inline int iterative_deepening(Position& pos, int max_depth, int time_limit_ms) {
    Move dummy;
    return iterative_deepening(pos, max_depth, time_limit_ms, dummy);
}

int eval(const Position& pos);