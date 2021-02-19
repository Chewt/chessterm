#include <stdio.h>
#include <string.h>
#include "board.h"

int main(int argc, char** argv)
{
    struct board board;
    default_board(&board);
    if (argc == 2)
        load_fen(&board, argv[1]);
    printf("\n");
    print_board(&board);
    int running = 1;
    while (running)
    {
        char* move = NULL;
        printf(": ");
        scanf("%ms", &move);
        if (!strcmp(move, "exit"))
            break;
        else if (!strcmp(move, "status"))
        {
            board_stats(&board);
            continue;
        }
        move_san(&board, move);
        print_board(&board);
    }

    return 0;
}
