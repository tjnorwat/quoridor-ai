#include "search.h"

int negamax(Position& pos, int depth, int alpha, int beta, int& nodes_searched) {
    ++nodes_searched; // Count this node

    if (depth == 0 || pos.is_terminal())
        return eval(pos);

    int best = INT32_MIN;
    MoveList moves(pos);
    for (const Move& m : moves) {
        pos.do_move(m);
        int score = -negamax(pos, depth - 1, -beta, -alpha, nodes_searched);
        pos.undo_move(m);

        best = std::max(best, score);
        alpha = std::max(alpha, score);
        if (alpha >= beta) break;
    }
    return best;
}

// Root negamax that tracks best move and observes time
int negamax_root(Position& pos, int depth, int alpha, int beta, Move& best_move,
                 std::chrono::steady_clock::time_point end_time, bool& time_up,
                 int& nodes_searched) {

    ++nodes_searched; // Count root (once per depth iteration)

    if (depth == 0 || pos.is_terminal())
        return eval(pos);

    int best_score = INT32_MIN;
    MoveList moves(pos);

    for (const Move& m : moves) {
        if (std::chrono::steady_clock::now() >= end_time) {
            time_up = true;
            break;
        }
        pos.do_move(m);
        int score = -negamax(pos, depth - 1, -beta, -alpha, nodes_searched);
        pos.undo_move(m);

        if (score >= best_score) {
            best_score = score;
            best_move = m;
        }
        if (score > alpha) alpha = score;
        if (alpha >= beta) break;
    }

    return best_score;
}

int iterative_deepening(Position& pos, int max_depth, int time_limit_ms, Move& best_move) {
    using clock = std::chrono::steady_clock;
    auto start = clock::now();
    auto end_time = time_limit_ms > 0
        ? start + std::chrono::milliseconds(time_limit_ms)
        : clock::time_point::max();

    int best_score = eval(pos);
    bool time_up = false;

    int nodes_searched = 0;

    for (int depth = 1; depth <= max_depth; ++depth) {
        time_up = false;
        int alpha = INT32_MIN;
        int beta  = INT32_MAX;

        Move current_best{};
        int score = negamax_root(pos, depth, alpha, beta, current_best, end_time, time_up, nodes_searched);

        if (time_up) {
            break;
        }

        best_score = score;
        best_move = current_best;

        // Optional: early exit on decisive result
        if (best_score >= WIN_SCORE - 1 || best_score <= LOSS_SCORE + 1)
            break;
    }

    // (Optional) expose nodes_searched somewhere (return via reference or global/log)
    std::cout << "Nodes searched: " << nodes_searched << "\n";
    return best_score;
}

int eval(const Position& pos) {
    // Perspective: score is for side to move (negamax requirement)
    Color us = pos.side_to_move;
    Color opp = ~us;

    int my_dist  = distance_to_goal(pos, us);
    int opp_dist = distance_to_goal(pos, opp);

    // If someone already at goal (defensive check)
    if (my_dist == 0)  return  WIN_SCORE;
    if (opp_dist == 0) return LOSS_SCORE;

    // Base: the smaller my_dist, the larger (opp_dist - my_dist)
    int score = (opp_dist - my_dist) * 100; // scale so walls matter less than a full tempo

    // Walls: more remaining walls is an asset
    int my_walls  = pos.num_walls[us];
    int opp_walls = pos.num_walls[opp];
    score += WALL_VALUE * (my_walls - opp_walls);

    return score;
}
