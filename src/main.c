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
        char move[50];
        printf(": ");
        scanf("%30s", &move);
        if (!strcmp(move, "exit"))
        {
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
            continue;
        }
        move_san(&board, move);
        print_board(&board);
        int game_win = is_gameover(&board);
        if (game_win == 1)
        {
            printf("Checkmate!\n");
            running = 0;
        }
        else if (game_win == 2)
        {
            printf("Stalemate!\n");
            running = 0;
        }
    }

    return 0;
}
