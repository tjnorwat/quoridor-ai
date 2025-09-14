#include "position.h"


Position::Position() {
    pawn[WHITE] = SQ_E1;
    pawn[BLACK] = SQ_E9;

    num_walls[WHITE] = 10;
    num_walls[BLACK] = 10;

    h_walls_idxs = Bitboard{0ULL, 0ULL};
    v_walls_idxs = Bitboard{0ULL, 0ULL};

    h_walls_full = Bitboard{0ULL, 0ULL};
    v_walls_full = Bitboard{0ULL, 0ULL};

    side_to_move = WHITE;
}

// assumes move is legal
void Position::do_move(Move move) {
    if (move.type == PAWN)
        pawn[side_to_move] = move.to;
    else if (move.type == H_WALL) {
        h_walls_idxs |= square_bb(move.from);
        h_walls_full |= square_bb(move.from) | square_bb(Square(move.from + EAST));
        num_walls[side_to_move]--;
    } 
    else {
        v_walls_idxs |= square_bb(move.from);
        v_walls_full |= square_bb(move.from) | square_bb(Square(move.from + SOUTH));    
        num_walls[side_to_move]--;
    }
    side_to_move = ~side_to_move;
}


void Position::undo_move(Move move) {
    side_to_move = ~side_to_move;
    if (move.type == PAWN)
        pawn[side_to_move] = move.from;
    else if (move.type == H_WALL) {
        h_walls_idxs ^= square_bb(move.from);
        h_walls_full ^= square_bb(move.from) | square_bb(Square(move.from + EAST));
        num_walls[side_to_move]++;
    } 
    else {
        v_walls_idxs ^= square_bb(move.from);
        v_walls_full ^= square_bb(move.from) | square_bb(Square(move.from + SOUTH));    
        num_walls[side_to_move]++;
    }
}


bool Position::is_terminal() const {
    if (GoalMask[WHITE] & pawn[WHITE])
        return true;
    if (GoalMask[BLACK] & pawn[BLACK])
        return true;

    return false;
}

// this makes it so that horizontal walls are between the square and the square south of it 
// and vertical walls are the the square and the square east of it
void Position::print_board() const { 
    for (Rank rank = RANK_9; rank >= RANK_1; --rank) {
        for (File file = FILE_A; file <= FILE_I; ++file) {
            Square sq = make_square(rank, file);
            if (pawn[WHITE] == sq) 
                std::cout << "W";
            else if (pawn[BLACK] == sq)
                std::cout << "B";
            else
                std::cout << ".";
        
            if (file < FILE_I) {
                if (v_walls_full & sq)
                    std::cout << "|";
                else
                    std::cout << " ";
            }
        }

        std::cout << std::endl;

        for (File file = FILE_A; file <= FILE_I; ++file) {
            Square sq = make_square(rank, file);
            if (rank > RANK_1) {
                if (h_walls_full & sq)
                    std:: cout << "--";
                else
                    std::cout << "  ";
            }
        }
        std::cout << std::endl;
    }

    std::cout << "Side to move: " << (side_to_move == WHITE ? "White" : "Black") << std::endl;
    std::cout << "White walls: " << num_walls[WHITE] << ", Black walls: " << num_walls[BLACK] << std::endl;
    std::cout << std::endl;
}