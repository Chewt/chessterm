#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "chessterm.h"
#include "uci.h"
#include "engine.h"

#ifdef DEBUG
#include "io.h"
#define print_debug(...) fprintf(stderr,__VA_ARGS__)
#else
#define print_debug(...) ((void)0)
#endif

void play_engine();
void play_stockfish(Engine* engine);
int engine_v_stockfish(Engine* engine, int silent, FILE* fp);
int engine_v_engine(char* fen, int silent);

int main(int argc, char** argv)
{
    srand(time(0));
    Board board;
    default_board(&board);
    if (argc == 2)
        load_fen(&board, argv[1]);
    printf("\n");
    print_fancy(&board);
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
            char fen[FEN_SIZE];
            export_fen(&board, fen);
            printf("%s\n", fen);
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
                print_fancy_flipped(&board);
            else
                print_fancy(&board);
            continue;
        }
        else if (!strcmp(move, "engine"))
        {
            if (argc == 1)
                play_engine(NULL);
            else if (argc == 2)
                play_engine(argv[1]);
            else if (argc == 3)
            {
                Engine engine;
                start_engine(&engine, argv[2]);
                play_stockfish(&engine);
                stop_engine(&engine);
            }
            continue;
        }
        else if (!strcmp(move, "itself"))
        {
            if (argc == 3)
            {
                Engine engine;
                start_engine(&engine, argv[2]);
                engine_v_stockfish(&engine, 0, NULL);
                stop_engine(&engine);
            }
            else
                engine_v_engine(NULL, 0);
            continue;
        }
        else if (!strcmp(move, "thousand"))
        {
            int i;
            int black_win = 0;
            int white_win = 0;
            int draw = 0;
            int result;
            Engine engine;
            if (argc == 3)
                start_engine(&engine, argv[2]);
            clock_t t = clock();
            FILE* games = fopen("thousand_games.txt", "w");
            for (i = 0; i < 1000; ++i)
            {
                clock_t in_t = clock();
                if(argc == 1)
                    result = engine_v_engine(NULL, 1);
                else if (argc == 2)
                    result = engine_v_engine(argv[1], 1);
                else if (argc == 3)
                    result = engine_v_stockfish(&engine, 1, games);
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
            fclose(games);
            if (argc == 3)
                stop_engine(&engine);
            continue;
        }
        move_san(&board, move);
        if (flipped)
            print_fancy_flipped(&board);
        else
            print_fancy(&board);

        int game_win = is_gameover(&board);
        if (game_win == 1)
        {
            printf("Checkmate!\n");
            running = 0;
            char* pgn = export_pgn(&board);
            printf("%s\n", pgn);
            free(pgn);
        }
        else if (game_win > 1)
        {
            if (game_win == 3)
                printf("50 move rule\n");
            else if (game_win == 4)
                printf("Three fold repetition\n");
            printf("Stalemate!\n");
            running = 0;
            char* pgn = export_pgn(&board);
            printf("%s\n", pgn);
            free(pgn);
        }
    }
    return 0;
}

void print_last_move(Board* board)
{
    Move record = board->history[board->history_count - 1];
    if (board->history_count % 2)
        dprintf(2 , "%d. ", board->history_count / 2 + 1);
    if (record.src_piece & king)
        dprintf(2 , "K");
    if (record.src_piece & queen)
        dprintf(2 , "Q");
    if (record.src_piece & rook)
        dprintf(2 , "R");
    if (record.src_piece & knight)
        dprintf(2 , "N");
    if (record.src_piece & bishop)
        dprintf(2 , "B");
    if (record.src_file != -1)
        dprintf(2 , "%c", 'a' + record.src_file);
    if (record.src_rank != -1)
        dprintf(2 , "%c", '0' + 8 - record.src_rank);
    if (record.piece_taken)
        dprintf(2 , "x");
    if (record.dest != -1)
    {
        dprintf(2 , "%c", record.dest % 8 + 'a');
        dprintf(2 , "%c", 8 - record.dest / 8 + '0');
    }
    if (record.promotion)
    {
        dprintf(2 , "=");
        if (record.promotion & queen)
            dprintf(2 , "Q");
        if (record.promotion & rook)
            dprintf(2 , "R");
        if (record.promotion & knight)
            dprintf(2 , "N");
        if (record.promotion & bishop)
            dprintf(2 , "B");
    }
    if (record.castle == 0)
        dprintf(2 , "O-O");
    if (record.castle == 1)
        dprintf(2 , "O-O-O");
    if (record.gave_check && !record.game_over)
        dprintf(2 , "+");
    if (record.game_over == 1)
    {
        dprintf(2 , "#");
        if (board->to_move)
            dprintf(2 , " 1-0");
        else
            dprintf(2 , " 0-1");
    }
    if (record.game_over == 2)
        dprintf(2 , " 1/2-1/2");
    dprintf(2 , " ");
    fflush(stderr);
}

int engine_v_engine(char* fen, int silent)
{
    int running = 1;
    Board board;
    if (fen != NULL)
        load_fen(&board, fen);
    else
        default_board(&board);
    int game_win = -2;
    while (running)
    {
        Move engine_move;
        if (board.to_move)
            engine_move = Emateinone(&board);
        else
            engine_move = Emateinone(&board);

        /*
        if (board.history_count%2 == 0)
            print_debug("%d.\n", board.moves);
        else
            print_debug("%d. ...\n", board.moves);
        print_board(&board);
        */

        int valid = move_piece(&board, &engine_move);

        if (valid == -1)
        {

            print_debug("%d to play\n", board.to_move);
            print_debug("Move: %d Trying %c%d to %c%d\n", board.moves,
                    engine_move.src_file + 'a' , engine_move.src_rank + 1, 
                    engine_move.dest%8+'a', 8-engine_move.dest/8);
        }
        game_win = is_gameover(&board);

        /*
        if (!valid)
        {
            if (silent)
                print_last_move(&board);
            if (game_win && silent)
                dprintf(2, "\n\n");
        }
        */

        if (game_win && !silent)
            print_fancy(&board);
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
        else if (game_win > 1)
        {
            if (!silent)
            {
                if (game_win == 3)
                    printf("50 move rule\n");
                else if (game_win == 4)
                    printf("Three fold repetition\n");
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
    else if (game_win > 1)
        return 0;
    else 
        return game_win;
}

void play_engine(char* fen)
{
    int running = 1;
    int flipped = 0;
    Board board;
    if (fen != NULL)
        load_fen(&board, fen);
    else
        default_board(&board);
    printf("\n");
    print_fancy(&board);
    while (running)
    {
        if (board.to_move)
        {
            //Move engine_move = Erandom_move(&board);
            //Move engine_move = Eaggressive_move(&board);
            //Move engine_move = Eape_move(&board);
            //Move engine_move = Eideal(&board);
            Move engine_move = Emateinone(&board);
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
                char fen[FEN_SIZE];
                export_fen(&board, fen);
                printf("%s\n", fen);
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
                    print_fancy_flipped(&board);
                else
                    print_fancy(&board);
                continue;
            }
            move_san(&board, move);

        }
        if (flipped)
            print_fancy_flipped(&board);
        else
            print_fancy(&board);
        int game_win = is_gameover(&board);
        if (game_win)
            print_fancy(&board);
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

int engine_v_stockfish(Engine* engine, int silent, FILE* fp)
{
    int running = 1;
    Board board;
    default_board(&board);
    send_ucinewgame(engine->write);
    memcpy(board.black_name, engine->name, strlen(engine->name) + 1);
    memcpy(board.white_name, "My Engine\0", 6);
    int game_win = -2;
    while (running)
    {
        Move engine_move;
        if (board.to_move)
            engine_move = get_engine_move(&board, engine);
        else
            engine_move = Emateinone(&board);

        /*
        char* pgn = export_pgn(&board);
        printf("%s\n", pgn);
        free(pgn);
        */

        /*
        if (board.history_count%2 == 0)
            print_debug("%d.\n", board.moves);
        else
            print_debug("%d. ...\n", board.moves);
        print_board(&board);
        */

        int valid = move_piece(&board, &engine_move);

        if (valid == -1)
        {

            print_debug("%d to play\n", board.to_move);
            print_debug("Move: %d Trying %c%d to %c%d\n", board.moves,
                    engine_move.src_file + 'a' , engine_move.src_rank + 1, 
                    engine_move.dest%8+'a', 8-engine_move.dest/8);
        }
        game_win = is_gameover(&board);

        /*
        if (!valid)
        {
            if (silent)
                print_last_move(&board);
            if (game_win && silent)
                dprintf(2, "\n\n");
        }
        */

        if (game_win && !silent)
            print_fancy(&board);
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
        else if (game_win > 1)
        {
            if (!silent)
            {
                if (game_win == 3)
                    printf("50 move rule\n");
                else if (game_win == 4)
                    printf("Three fold repetition\n");
                printf("Stalemate!\n");
                char* pgn = export_pgn(&board);
                printf("%s\n", pgn);
                free(pgn);
            }
            running = 0;
        }
    }
    if (silent && fp)
    {
        char* pgn = export_pgn(&board);
        if (fprintf(fp, "%s\n\n", pgn) < 0)
        {
            perror("Couldn't write to file");
            exit(1);
        }
        free(pgn);
    }
    if (game_win == 1 && board.to_move)
        return 1;
    else if (game_win == 1 && !board.to_move)
        return -1;
    else if (game_win > 1)
        return 0;
    else 
        return game_win;
}

void play_stockfish(Engine* engine)
{
    int running = 1;
    int flipped = 0;
    Board board;
    default_board(&board);
    memcpy(board.black_name, engine->name, strlen(engine->name) + 1);
    printf("\n");
    print_fancy(&board);
    while (running)
    {
        if (board.to_move)
        {
            Move engine_move = get_engine_move(&board, engine);
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
                char fen[FEN_SIZE];
                export_fen(&board, fen);
                printf("%s\n", fen);
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
                    print_fancy_flipped(&board);
                else
                    print_fancy(&board);
                continue;
            }
            move_san(&board, move);

        }
        if (flipped)
            print_fancy_flipped(&board);
        else
            print_fancy(&board);
        int game_win = is_gameover(&board);
        if (game_win)
            print_fancy(&board);
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
