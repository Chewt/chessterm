#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"

int main(int argc, char** argv)
{
    Board board;
    init_board(&board);
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
        {
            free(move);
            break;
        }
        else if (!strcmp(move, "status"))
        {
            board_stats(&board);
            continue;
        }
        else if (!strcmp(move, "fen"))
        {
            char* fen = export_fen(&board);
            printf("%s\n", fen);
            free(fen);
            continue;
        }
        move_san(&board, move);
        print_board(&board);
        free(move);
        if (is_checkmate(&board, 0))
        {
            printf("Checkmate!\n");
            running = 0;
        }
        board.to_move = !board.to_move;
        if (is_checkmate(&board, 1))
        {
            printf("Checkmate!\n");
            running = 0;
        }
        board.to_move = !board.to_move;
    }

    return 0;
}
