#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "chessterm.h"

int main(int argc, char** argv)
{
    Board board;
    default_board(&board);
    if (argc == 2)
        load_fen(&board, argv[1]);
    printf("\n");
    print_board(&board);
    int running = 1;
    int flipped = 0;
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
            free(fen);
            continue;
        }
        else if (!strcmp(move, "pgn"))
        {
            char* pgn = export_pgn(&board);
            printf("%s\n", pgn);
            free(pgn);
            continue;
        }
        else if (!strcmp(move, "flip"))
        {
            flipped = !flipped;
            if (flipped)
                print_flipped(&board);
            else
                print_board(&board);
            continue;
        }
        move_san(&board, move);
        if (flipped)
            print_flipped(&board);
        else
            print_board(&board);
        
#ifdef STRESSTEST
        clock_t t;
        t = clock();
        int i;
        for (i = 0; i < 1000000; ++i)
            is_gameover(&board);
        t = clock() - t;
        double time_taken = ((double)t)/CLOCKS_PER_SEC;
        printf("Stress test took %f seconds\n", time_taken);
#endif


        int game_win = is_gameover(&board);
        if (game_win == 1)
        {
            printf("Checkmate!\n");
            running = 0;
            char* pgn = export_pgn(&board);
            printf("%s\n", pgn);
            free(pgn);
        }
        else if (game_win == 2)
        {
            printf("Stalemate!\n");
            running = 0;
            char* pgn = export_pgn(&board);
            printf("%s\n", pgn);
            free(pgn);
        }
    }
    return 0;
}
