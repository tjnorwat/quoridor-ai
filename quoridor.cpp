#include "position.h"
#include "bitboard.h"
#include "movegen.h"
#include "search.h"


void ai_vs_ai() {
    Position pos;
    pos.print_board();
    while (!pos.is_terminal()) {
        Move best{};
        int score = iterative_deepening(pos, 4, 0, best);
        std::cout << "Best move score: " << score << "\n";
        best.print_move();
        pos.do_move(best);
        pos.print_board();
    }
}


void ai_takeover(Position& pos) {
    while (!pos.is_terminal()) {
        Move best{};
        int score = iterative_deepening(pos, 4, 0, best);
        std::cout << "Best move score: " << score << "\n";
        best.print_move();
        pos.do_move(best);
        pos.print_board();
    }
}


void testing() {
    Position pos;
    // pos.print_board();

    // set white to be idx 69
    pos.pawn[WHITE] = SQ_G8;
    pos.pawn[BLACK] = SQ_A7;

    pos.do_move(Move{SQ_F9, SQ_NONE, H_WALL});
    pos.do_move(Move{SQ_D9, SQ_NONE, H_WALL});

    pos.num_walls[WHITE] = 3;
    pos.num_walls[BLACK] = 3;

    pos.print_board();


    ai_takeover(pos);

    // Move best{};
    // int score = iterative_deepening(pos, 5, 10000, best);
    // std::cout << "Best move score: " << score << "\n";
    // best.print_move();
}

int main() {
    init();

    ai_vs_ai();
    // testing();

    return 0;
}