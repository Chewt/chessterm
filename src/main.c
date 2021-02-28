#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "chessterm.h"
#include "engine.h"

void play_engine();
void engine_v_engine();


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
            engine_v_engine();
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

void engine_v_engine()
{
    int running = 1;
    Board board;
    default_board(&board);
    while (running)
    {
        Move engine_move = Erandom_move(&board);
        move_piece(&board, &engine_move);
        char* curr_pgn = export_pgn(&board);
        printf("%s\n", curr_pgn);
        Move last_hist = board.history[board.history_count - 1];
        if (board.history_count)
            printf("src_piece: %x, dest: %d\n", last_hist.src_piece, last_hist.dest);
        free(curr_pgn);
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

void play_engine()
{
    int running = 1;
    int flipped = 0;
    Board board;
    default_board(&board);
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
