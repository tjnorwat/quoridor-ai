#include "position.h"
#include "bitboard.h"
#include "movegen.h"
#include "search.h"


void ai_vs_ai() {
    Position pos;
    pos.print_board();
    while (!pos.is_terminal()) {
        Move best{};
        int score = iterative_deepening(pos, 5, 10000, best);
        std::cout << "Best move score: " << score << "\n";
        best.print_move();
        pos.do_move(best);
        pos.print_board();
    }
}


void testing() {
    Position pos;
    pos.print_board();
    
    pos.do_move(Move{SQ_E1, SQ_E2, PAWN});
    pos.print_board();
    
    pos.do_move(Move{SQ_E3, SQ_NONE, V_WALL});
    pos.print_board();

    pos.do_move(Move{SQ_E9, SQ_NONE, H_WALL});
    pos.print_board();

    Move best{};
    int score = iterative_deepening(pos, 5, 10000, best);
    std::cout << "Best move score: " << score << "\n";
    best.print_move();
}

int main() {
    init();

    ai_vs_ai();
    // testing();

    return 0;
}