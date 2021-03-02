#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "chessterm.h"
#include "engine.h"

void play_engine();
int engine_v_engine(int silent);


int main(int argc, char** argv)
{
    srand(time(0));
    Board board;
    default_board(&board);
    if (argc == 2)
        load_fen(&board, argv[1]);
    printf("\n");
    print_board(&board);
    int running = 1;
    int flipped = 0;
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
        else if (!strcmp(move, "engine"))
        {
            play_engine();
            continue;
        }
        else if (!strcmp(move, "itself"))
        {
            engine_v_engine(0);
            continue;
        }
        else if (!strcmp(move, "thousand"))
        {
            int i;
            int black_win = 0;
            int white_win = 0;
            int draw = 0;
            int result;
            clock_t t = clock();
            for (i = 0; i < 100000; ++i)
            {
                clock_t in_t = clock();
                result = engine_v_engine(1);
                in_t = clock() - in_t;
                double t_taken = ((double)in_t)/CLOCKS_PER_SEC;
                printf("Time taken for game %d: %f seconds\n", i + 1, t_taken);
                if (result == -1)
                    black_win++;
                else if (result == 1)
                    white_win++;
                else if (result == 0)
                    draw++;
            }
            t = clock() - t;
            double time_taken = ((double)t)/CLOCKS_PER_SEC;
            printf("Time taken: %f seconds\nWhite: %d\nBlack: %d\nDraw: %d\n", 
                    time_taken, white_win, black_win, draw);
            continue;
        }
        move_san(&board, move);
        if (flipped)
            print_flipped(&board);
        else
            print_board(&board);

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

int engine_v_engine(int silent)
{
    int running = 1;
    Board board;
    default_board(&board);
    board.white_name = "Random Engine";
    board.black_name = "Random Engine";
    int game_win = -2;
    while (running)
    {
        Move engine_move = Erandom_move(&board);
        move_piece(&board, &engine_move);
        game_win = is_gameover(&board);

        if (game_win && !silent)
            print_board(&board);
        if (game_win == 1)
        {
            if (!silent)
            {
                printf("Checkmate!\n");
                char* pgn = export_pgn(&board);
                printf("%s\n", pgn);
                free(pgn);
            }
            running = 0;
        }
        else if (game_win == 2)
        {
            if (!silent)
            {
                printf("Stalemate!\n");
                char* pgn = export_pgn(&board);
                printf("%s\n", pgn);
                free(pgn);
            }
            running = 0;
        }
    }
    if (game_win == 1 && board.to_move)
        return 1;
    else if (game_win == 1 && !board.to_move)
        return -1;
    else if (game_win == 2)
        return 0;
    else 
        return game_win;
}

void play_engine()
{
    int running = 1;
    int flipped = 0;
    Board board;
    default_board(&board);
    board.white_name = "User";
    board.black_name = "Random Engine";
    printf("\n");
    print_board(&board);
    while (running)
    {
        if (board.to_move)
        {
            Move engine_move = Erandom_move(&board);
            move_piece(&board, &engine_move);
        }
        else
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

        }
        if (flipped)
            print_flipped(&board);
        else
            print_board(&board);
        int game_win = is_gameover(&board);
        if (game_win)
            print_board(&board);
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
}
