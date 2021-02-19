#include <stdio.h>
#include "board.h"

int main()
{
    struct board board;
    //load_fen(&board, "rnbq1bnr/1ppp1ppp/6k1/4Q3/4P3/p7/PPP2PPP/RNBQKBNR w KQ - 2 8");
    default_board(&board);
    printf("\n");
    print_board(&board);
    int running = 1;
    while (running)
    {
        char* move = NULL;
        printf(": ");
        scanf("%ms", &move);
        move_san(&board, move);
        print_board(&board);
    }

    return 0;
}
