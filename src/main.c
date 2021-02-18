#include <stdio.h>
#include "board.h"

int main()
{
    struct board board;
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
