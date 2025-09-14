#include "bitboard.h"

Bitboard PawnAttacks[SQ_NB];
Bitboard ValidWalls;
Bitboard ValidSquares;
Bitboard GoalMask[COLOR_NB];


void init() {
    for (Rank rank = RANK_1; rank <= RANK_9; ++rank) {
        for (File file = FILE_A; file <= FILE_I; ++file) {
            Square sq = make_square(rank, file);
            Bitboard attacks = Bitboard{0ULL, 0ULL};
            // i think we should just add the cardinal directions first 
            // then if a cardinal direction is special, aka opponent pawn, then add the correct diagonals
            if (rank < RANK_9)
                attacks |= square_bb(sq + NORTH);
            if (rank > RANK_1)
                attacks |= square_bb(sq + SOUTH);
            if (file < FILE_I)
                attacks |= square_bb(sq + EAST);
            if (file > FILE_A)
                attacks |= square_bb(sq + WEST);

            PawnAttacks[sq] = attacks;
        }
    }

    // Horizontal and Vertical walls can both be represented by the same mask
    ValidWalls = Bitboard{0ULL, 0ULL};
    for (Rank rank = RANK_2; rank <= RANK_9; ++rank) {
        for (File file = FILE_A; file <= FILE_H; ++file) {
            Square sq = make_square(rank, file);
            ValidWalls |= sq;
        }
    }

    ValidSquares = Bitboard{0ULL, 0ULL};
    for (Square sq = SQ_A1; sq < SQ_NB; ++sq) {
        ValidSquares |= sq;
    }

    GoalMask[WHITE] = Bitboard{0ULL, 0ULL};
    GoalMask[BLACK] = Bitboard{0ULL, 0ULL};
    for (File file = FILE_A; file <= FILE_I; ++file) {
        GoalMask[WHITE] |= make_square(RANK_9, file);
        GoalMask[BLACK] |= make_square(RANK_1, file);
    }
}


void print_bitboard(Bitboard b) {
    for (Rank rank = RANK_9; rank >= RANK_1; --rank) {
        for (File file = FILE_A; file <= FILE_I; ++file) {
            Square sq = make_square(rank, file);
            if (b & sq)
                std::cout << "1";
            else
                std::cout << ".";

            std::cout << " ";
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }
    std::cout << std::endl;
}