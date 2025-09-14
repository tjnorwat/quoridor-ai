#include "movegen.h"

// Forward declarations for helper functions
inline bool is_wall_between(const Position& pos, Square s1, Square s2);
inline Move* splat_pawn_moves(Move* moveList, Square from, Bitboard to_bb);

Move* generate(const Position& pos, Move* moveList) {
    moveList = generate_pawn_moves(pos, moveList);
    moveList = generate_wall_moves(pos, moveList);
    return moveList;
}

Move* generate_pawn_moves(const Position& pos, Move* moveList) {
    Color us = pos.side_to_move;
    Square us_sq = pos.pawn[us];
    Square them_sq = pos.pawn[~us];
    
    Bitboard moves_bb = Bitboard{0ULL, 0ULL};
    Bitboard all_pawns = square_bb(us_sq) | square_bb(them_sq);

    // 1. Generate standard cardinal moves, excluding squares occupied by pawns.
    Bitboard attacks = PawnAttacks[us_sq] & ~all_pawns;

    // 2. Check for opponent adjacency to handle jumps.
    if (PawnAttacks[us_sq] & them_sq) {
        Direction dir = Direction(them_sq - us_sq);

        // If there's a wall between us and them, we can't jump.
        if (!is_wall_between(pos, us_sq, them_sq)) {
            Square jump_sq = them_sq + dir;

            // 3. Try to generate a straight jump.
            // Check if jump is on board and not blocked by a wall behind the opponent.
            if ((PawnAttacks[them_sq] & jump_sq) && !is_wall_between(pos, them_sq, jump_sq)) {
                moves_bb |= square_bb(jump_sq);
            } 
            // 4. If straight jump is not possible, generate diagonal jumps.
            else {
                // Get directions perpendicular to the opponent's direction
                Direction d1 = (dir == NORTH || dir == SOUTH) ? EAST : NORTH;
                Direction d2 = (dir == NORTH || dir == SOUTH) ? WEST : SOUTH;

                Square diag1 = them_sq + d1;
                Square diag2 = them_sq + d2;

                // Check diagonal 1: must be on board and no wall from opponent
                if ((PawnAttacks[them_sq] & diag1) && !is_wall_between(pos, them_sq, diag1)) {
                    moves_bb |= square_bb(diag1);
                }
                // Check diagonal 2: must be on board and no wall from opponent
                if ((PawnAttacks[them_sq] & diag2) && !is_wall_between(pos, them_sq, diag2)) {
                    moves_bb |= square_bb(diag2);
                }
            }
        }
    }

    // 5. Add standard moves that are not blocked by walls.
    Bitboard potential_moves = attacks;
    while (potential_moves) {
        Square to = pop_lsb(potential_moves);
        if (!is_wall_between(pos, us_sq, to)) {
            moves_bb |= square_bb(to);
        }
    }

    // Remove any moves that would land on the other pawn (can happen with diagonal 'jumps')
    // dont think this is possible
    moves_bb &= ~square_bb(them_sq);

    moveList = splat_pawn_moves(moveList, us_sq, moves_bb);

    return moveList;
}

Move* generate_wall_moves(const Position& pos, Move* moveList) {
    Color us = pos.side_to_move;

    uint16_t our_walls = pos.num_walls[us];
    if (our_walls == 0)
        return moveList;

    // generate horizontal 
    Bitboard h_walls = Bitboard{0ULL, 0ULL};
    Bitboard v_walls = Bitboard{0ULL, 0ULL};
    
    // cant place wall where there is already a wall AND there has to be at least 2 squares of space
    h_walls = ~(pos.h_walls_full | shift<WEST>(pos.h_walls_full));
    // cannot place a wall in between a vertical wall
    // but can place walls after a vertical wall segment ; making a T shape  
    h_walls &= ~(pos.v_walls_idxs);
    h_walls &= ValidWalls;
    
    // cant place wall where there is already a wall
    v_walls = ~(pos.v_walls_full | shift<NORTH>(pos.v_walls_full));
    // cannot place a wall in between a horizontal wall
    // but can place walls after a horizontal wall segment ; making a T shape
    v_walls &= ~(pos.h_walls_idxs);
    v_walls &= ValidWalls;
    
    // TODO ; we also cannot place a wall that completely blocks a player from reaching their goal
    // naive implementation is to just test each wall placement and see if both players can still reach their goal 
    Bitboard h_walls_copy = h_walls;
    while (h_walls_copy) {
        Square wall_sq = pop_lsb(h_walls_copy);
        Position pos_copy = pos;
        pos_copy.h_walls_full |= square_bb(wall_sq) | square_bb(Square(wall_sq + EAST));
        if (!reachable_any_goal(pos_copy, pos_copy.pawn[WHITE], GoalMask[WHITE]) ||
        !reachable_any_goal(pos_copy, pos_copy.pawn[BLACK], GoalMask[BLACK])) {
            h_walls ^= square_bb(wall_sq); // remove this wall placement
        }
    }

    Bitboard v_walls_copy = v_walls;
    while (v_walls_copy) {
        Square wall_sq = pop_lsb(v_walls_copy);
        Position pos_copy = pos;
        pos_copy.v_walls_full |= square_bb(wall_sq) | square_bb(Square(wall_sq + SOUTH));
        if (!reachable_any_goal(pos_copy, pos_copy.pawn[WHITE], GoalMask[WHITE]) ||
            !reachable_any_goal(pos_copy, pos_copy.pawn[BLACK], GoalMask[BLACK])) {
            v_walls ^= square_bb(wall_sq); // remove this wall placement
        }
    }

    moveList = splat_wall_moves(moveList, h_walls, H_WALL);
    moveList = splat_wall_moves(moveList, v_walls, V_WALL);

    return moveList;
}

// Helper to check for walls between two adjacent squares.
// Your wall representation:
// - horizontal_walls at 's' block movement between 's' and 's - NORTH' (s + SOUTH)
// - vertical_walls at 's' block movement between 's' and 's - EAST' (s + WEST)
inline bool is_wall_between(const Position& pos, Square from, Square to) {    
    if (from == to) return false;

    // Vertical wall check (East/West move)
    if (rank_of(from) == rank_of(to)) {
        // Square west_sq = std::min(from, to);
        Square west_sq = file_of(from) < file_of(to) ? from : to;
        return static_cast<bool>(pos.v_walls_full & west_sq);
    }
    // Horizontal wall check (North/South move)
    else if (file_of(from) == file_of(to)) {
        // Square south_sq = std::min(from, to);
        Square south_sq = rank_of(from) < rank_of(to) ? from : to;
        return static_cast<bool>(pos.h_walls_full & (south_sq + NORTH));
    }
    
    return true; // Not cardinally adjacent
}


inline Move* splat_wall_moves(Move* moveList, Bitboard bb, MoveType type) {
    while (bb) {
        Square from = pop_lsb(bb);
        *moveList++ = Move{from, SQ_NONE, type};
    }
    return moveList;
}


inline Move* splat_pawn_moves(Move* moveList, Square from, Bitboard to_bb) {
    while (to_bb) {
        Square to = pop_lsb(to_bb);
        *moveList++ = Move{from, to, PAWN};
    }
    return moveList;
}


bool reachable_any_goal_slow(const Position& pos, Square start, Bitboard goal_mask) {
    Bitboard visited = Bitboard{0ULL, 0ULL};
    Bitboard to_visit = square_bb(start);

    while (to_visit) {
        Square sq = pop_lsb(to_visit);
        if (goal_mask & sq) return true;

        visited |= square_bb(sq);

        // Explore neighbors
        for (Direction dir : {NORTH, EAST, SOUTH, WEST}) {
            Square neighbor = sq + dir;

            if (ValidSquares & neighbor &&               // on board ; protects from going up too far
                !(visited & neighbor)   &&               // not visited
                !is_wall_between(pos, sq, neighbor)) {   // no wall in between
                to_visit |= neighbor;
            }
        }
    }

    return false;
}

bool reachable_any_goal(const Position& pos, Square start, Bitboard goal_mask) {
    Bitboard visited = Bitboard{0ULL, 0ULL};
    Bitboard to_visit = square_bb(start);

    while (to_visit) {
        Square sq = pop_lsb(to_visit);
        if (goal_mask & sq) return true;

        visited |= square_bb(sq);

        // Explore neighbors
        Bitboard neighbors = PawnAttacks[sq] & ~visited & ValidSquares;
        while (neighbors) {
            Square neighbor = pop_lsb(neighbors);
            if (!is_wall_between(pos, sq, neighbor)) {
                to_visit |= neighbor;
            }
        }
    }

    return false;
}

int distance_to_goal(const Position& pos, Color c) {
    // Using BFS to find the shortest path to the goal
    Bitboard visited = Bitboard{0ULL, 0ULL};
    Bitboard to_visit = square_bb(pos.pawn[c]);
    int distance = 0;

    while (to_visit) {
        Bitboard next_to_visit = Bitboard{0ULL, 0ULL};

        while (to_visit) {
            Square sq = pop_lsb(to_visit);
            if (GoalMask[c] & sq) return distance;

            visited |= square_bb(sq);

            Bitboard neighbors = PawnAttacks[sq] & ~visited & ValidSquares;
            while (neighbors) {
                Square neighbor = pop_lsb(neighbors);
                if (!is_wall_between(pos, sq, neighbor)) {
                    next_to_visit |= neighbor;
                }
            }
        }

        to_visit = next_to_visit;
        distance++;
    }

    return std::numeric_limits<int>::max(); // No path found
}