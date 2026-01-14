#include "search.h"

// Combined function: Handles both root behavior (tracking best_move) and recursive behavior
int negamax(Position& pos, int depth, int alpha, int beta, int& nodes_searched, 
            Move* best_move, std::chrono::steady_clock::time_point end_time, bool& time_up) {
    
    ++nodes_searched;

    // Check time every 2048 nodes to avoid system call overhead
    if ((nodes_searched & 2047) == 0) {
        if (end_time < std::chrono::steady_clock::time_point::max() && 
            std::chrono::steady_clock::now() >= end_time) {
            time_up = true;
            return 0; // Return dummy value
        }
    }

    if (depth == 0 || pos.is_terminal()) {
        int score = eval(pos);
        if (score == WIN_SCORE) return WIN_SCORE + depth;
        if (score == LOSS_SCORE) return LOSS_SCORE - depth;
        return score;
    }

    int best_val = -INF;
    MoveList moves(pos);

    // add move ordering heuristics here later
    for (const Move& m : moves) {
        pos.do_move(m);
        // Pass nullptr for inner nodes so we don't track moves for them
        // dont care about the best move except at root
        // only thing we care about is the score for recursive calls
        int score = -negamax(pos, depth - 1, -beta, -alpha, nodes_searched, nullptr, end_time, time_up);
        pos.undo_move(m);

        if (time_up) return 0;

        if (score > best_val) {
            best_val = score;
            
            // Only update best_move if this is the root
            // Move var is passed from iterative_deepening
            if (best_move != nullptr) {
                *best_move = m;
            }
        }

        alpha = std::max(alpha, score);
        if (alpha >= beta) break;
    }
    return best_val;
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
        Move current_iteration_best{};
        time_up = false;
        
        // Pass &current_iteration_best to capture the move at the root
        // nullptr would be passed inside the recursion automatically
        int score = negamax(pos, depth, -INF, INF, nodes_searched, &current_iteration_best, end_time, time_up);
        
        if (time_up) {
            break; // Discard results of incomplete search
        }

        best_score = score;
        best_move = current_iteration_best;

        // Optional: early exit on decisive result
        if (best_score >= WIN_SCORE - 1 || best_score <= LOSS_SCORE + 1)
            break;
    }

    // (Optional) expose nodes_searched somewhere (return via reference or global/log)
    std::cout << "Nodes searched: " << nodes_searched << "\n";
    return best_score;
}

int eval(const Position& pos) {
    Color us = pos.side_to_move;
    Color opp = ~us;

    int my_dist = distance_to_goal(pos, us);
    int opp_dist = distance_to_goal(pos, opp);

    // 1. Immediate Terminal Detection
    if (my_dist == 0) return WIN_SCORE;
    if (opp_dist == 0) return LOSS_SCORE;

    int score = 0;

    // 2. Linear Distance Weighting
    // We want to minimize our distance and maximize opponent distance.
    // Scaling factor ensures distance is the primary driver.
    // Max distance is 81 squares (roughly), though path can be longer.
    // 50 per step is substantial compared to walls.
    score += (opp_dist - my_dist) * 50; 

    // 3. Tempo Bonus
    // In a racing game, being the one whose turn it is is a massive advantage.
    // This breaks "ties" where both are the same distance away.
    score += 10; 

    // 4. Wall Value Scaling
    // Walls are worth more when you have many and the opponent has few.
    // We also value walls slightly more if the game is still early (long paths).
    int wall_diff = pos.num_walls[us] - pos.num_walls[opp];
    int wall_multiplier = (my_dist > 4) ? 10 : 2; // Walls lose value as we approach goal
    score += wall_diff * wall_multiplier;

    // 5. Centrality Bonus (Heuristic)
    // Pawns in the center are harder to block than pawns on the edges.
    // Square coordinates usually range 0-8 for x and y.
    int my_file = file_of(pos.pawn[us]);
    int centrality = 4 - std::abs(4 - my_file); // 0 at edges, 4 at center
    score += centrality * 2;

    return score;
}