#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <argp.h>
#include "chessterm.h"
#include "engine.h"
#include "settings.h"
#include "commands.h"
#include "networking.h"


#ifdef DEBUG
#define print_debug(...) fprintf(stderr,__VA_ARGS__)
#else
#define print_debug(...) ((void)0)
#endif

void play_build_in_engine(char* fen);
void play_engine(Engine* engine);
int engine_v_stockfish(Engine* engine, int silent, FILE* fp);
int engine_v_engine(char* fen, int silent);
void thousand_games(Engine* white_engine, Engine* black_engine);
void initialize_white(char* e_path, int depth, Board* board, Engine* engine);
void initialize_black(char* e_path, int depth, Board* board, Engine* engine);
void prand(Board* board, Engine* white_engine, Engine* black_engine);
void sanity_check(Board* board, Engine* engine);

struct flags { 
    char* fen;
    char* white_engine;
    char* black_engine;
    int white_depth;
    int black_depth;
    int random;
    int is_server;
    char *host;
    int port;
    int swap;
};
static int parse_opt (int key, char *arg, struct argp_state *state)
{
    struct flags *flags = state->input;
    switch (key)
    {
        case 'f': 
            flags->fen = arg;
            break;
        case 'w':
            flags->white_engine = arg;
            break;
        case 'b':
            flags->black_engine = arg;
            break;
        case 'r':
            flags->random = 1;
            break;
        case 500:
            flags->white_depth = 1;
            break;
        case 501:
            flags->black_depth = 1;
            break;
        case 'h':
            flags->is_server = 1;
            break;
        case 'c':
            flags->host = arg;
            break;
        case 'p':
            flags->port = atoi(arg);
        case 502:
            flags->swap = 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    srand(time(0));

    // Switch to alternate buffer
    //printf("\e[?1049h\e[2J");

    /* Each bit will correlate to a boolean:
     * 0x80000000 stop, can check bools < 0
     * 0x01 is flipped
     * 0x02 is checkmate
     * 0x04 is stalemate
     * 0x08 is 50 move rule
     * 0x10 is is threefold repitition
     * 0x20 is MAX_HISTORY
     * 0x40 is MAX_STORED_POSITIONS
     * 
     * 0x100 is autoflipping
     * 0x200 was an engine assigned a side randomly
     * 0x400 is set when a command should be processed rather than a move,
     *       for example, when there are two engines, process user input
     *       before letting the engines play
     * 
     * Stop bit should be set to stop the program, should be 0 while running
     */
    int bools = 0;
    #ifdef AUTOFLIP
        if (AUTOFLIP)
            bools |= AUTOFLIP;
    #endif

    char notes[NOTES_LENGTH];
    Board board;
    board.notes = notes;
    default_board(&board); 
    Engine white_engine;
    white_engine.pid = 0;
    Engine black_engine;
    black_engine.pid = 0;

    char* last_pgn = NULL;

    /* Flags processing */
    struct flags flags;
    flags.fen = NULL;
    flags.black_engine = NULL;
    flags.white_engine = NULL;
    flags.white_depth = 5;
    flags.black_depth = 5;
    flags.random = 0;
    flags.is_server = 0;
    flags.host = NULL;
    flags.port = 5000;
    flags.swap = 0;


    struct argp_option options[] = {
        {"fen", 'f', "STRING", 0, 
            "Supply a FEN to determine the starting position of the game."},
        {0, 0, 0, 0, "Engines:", 1},
        {"white_engine", 'w', "PATH", 0, 
            "The path of the engine that will play as white."},
        {"black_engine", 'b', "PATH", 0, 
            "The path of the engine that will play as black."},
        {"white_depth", 500, 0, 0, 
            "The depth that the white engine should look."},
        {"black_depth", 501, 0, 0, 
            "The depth that the white engine should look."},
        {"random", 'r', 0, 0, 
            "Randomly assign engine(s) to black or white."},
        {0,0,0,0, "Networking:", 7},
        { "host", 'h', 0, 0, "Allow connections from another player", 0},
        { "connect", 'c', "IP", 0, "Connect to a host", 0},
        { "port", 'p', "PORT", 0, "Port for connecting to host", 0},
        { "swap", 502, 0, 0, "Swap colors for host and client", 0},
        {0}};
    struct argp argp = { options, parse_opt };
    int r = argp_parse(&argp, argc, argv, 0, 0, &flags);
    if (flags.fen)
        load_fen(&board, flags.fen);
    if (flags.random)
    {
        int temp = rand() % 2;
        if (flags.white_engine && flags.black_engine && temp)
        {
            char* temp_e = flags.white_engine;
            int temp_d = flags.white_depth;
            flags.white_engine = flags.black_engine;
            flags.white_depth = flags.black_depth;
            flags.black_engine = temp_e;
            flags.black_depth = temp_d;
        }
        else if (flags.white_engine && temp)
        {
            flags.black_engine = flags.black_engine;
            flags.black_depth = flags.black_depth;
            flags.white_engine = NULL;
        }
        else if (flags.black_engine && temp)
        {
            flags.white_engine = flags.black_engine;
            flags.white_depth = flags.black_depth;
            flags.black_engine = NULL;
        }
    }
    if (flags.white_engine)
    {
        initialize_white(flags.white_engine, flags.white_depth, &board,
                         &white_engine);
    }
    if (flags.black_engine)
    {
        initialize_black(flags.black_engine, flags.black_depth, &board,
                         &black_engine);
    }
    if (black_engine.pid && white_engine.pid)
        bools |= COMMAND;

    /* Set up networking */
    int client = -1;
    int host = -1;
    int host_col = 1;
    int client_col = 0;
    if (flags.is_server && flags.port)
    {
        client = SetupServer(flags.port);
        if (flags.swap)
        {
            int t = host_col;
            host_col = client_col;
            client_col = t; 
            SendCommand(client, "swap");
        }
        else
            bools |= FLIPPED;
        if (flags.fen)
        {
            SendCommand(client, "loadfen");
            SendCommand(client, flags.fen);
        }
        SendCommand(client, "ready");
    }
    else if (flags.host && flags.port)
    {
        host = SetupClient(flags.host, flags.port);
        int setup_finished = 0;
        while (!setup_finished)
        {
            char* resp = RecvCommand(host);
            if (resp== NULL) {
                printf("Connection to other user broken\n");
                exit(0);
            }
            int rv = ProcessCommand(&board, resp);
            if (rv == SWAP)
            {
                int t = host_col;
                host_col = client_col;
                client_col = t; 
                bools |= FLIPPED;
            }
            if (!strcmp(resp, "loadfen"))
            {
                free(resp);
                resp = RecvCommand(host);
                load_fen(&board, resp);
            }
            if (!strcmp(resp, "ready"))
                setup_finished = 1;
            free(resp);
        }
    }

    printf("\n");
    if (!(bools & STOP))
        print_fancy(&board);

    bools |= is_gameover(&board);
    while (!(bools & STOP))
    {
        printf("\e[2J\e[H");
        if (bools & FLIPPED)
            print_fancy_flipped(&board);
        else
            print_fancy(&board);
        printf("%s", board.notes);
        board.notes[0] = '\0';

        /* If human move */
        if (( board.to_move && !black_engine.pid) || 
            (!board.to_move && !white_engine.pid) || 
            (bools & COMMAND))
        {
            char move[256];
            int res = 0;

            if ((host   >= 0 && board.to_move == host_col) ||
                (client >= 0 && board.to_move == client_col)) 
            {
              int user = (host > 0) ? host : client;
              printf("Waiting on opponent...\n");
              char *response = RecvCommand(user);
              if (response == NULL) {
                printf("Connection to other user broken\n");
                break;
              }
              //snprintf(board.notes, NOTES_LENGTH, "%s\n", response);
              res = ProcessCommand(&board, response);
              strcpy(move, response);
              free(response);
            } else {
              printf(": ");
              scanf("%30s", move);
              if (host >= 0 && is_networked_command(move))
                SendCommand(host, move);
              else if (client >= 0 && is_networked_command(move))
                SendCommand(client, move);
              res = ProcessCommand(&board, move);
            }

            if (res == NEWGAME)
            {
                if (white_engine.pid){
                    send_ucinewgame(white_engine.write);
                    memcpy(board.white_name, white_engine.name, 
                           strlen(white_engine.name) + 1);
                }
                if (black_engine.pid){
                    send_ucinewgame(black_engine.write);
                    memcpy(board.black_name, black_engine.name, 
                           strlen(black_engine.name) + 1);
                }
            }
            else if (res == SWAP)
            {
                int t = host_col;
                host_col = client_col;
                client_col = t;
                bools ^= FLIPPED;
                continue;
            }
            else if (res != MOVE)
            {
                bools ^= res;
                continue;
            }

            /* Autoflip */
            if (!move_san(&board, move) && bools & AUTOFLIP)
            {
                bools ^= 1;
            }

        }
        else
        {
            Move engine_move;
            if (board.to_move)
            {
                engine_move = get_engine_move(&board, &black_engine);
            }
            else
            {
                engine_move = get_engine_move(&board, &white_engine);
            }
            
            int i;
            i = move_piece(&board, &engine_move);
            if (i == -1)
            {
                print_debug("%d to play\n", board.to_move);
                print_debug("Move: %d Trying %c%d to %c%d\n", board.moves,
                        engine_move.src_file + 'a' , engine_move.src_rank + 1, 
                        engine_move.dest%8+'a', 8-engine_move.dest/8);
            }

            /* Autoflip */
            if (!i && bools & AUTOFLIP)
            {
                bools ^= 1;
            }

        }


        bools |= is_gameover(&board);
        
        if (bools & (CHECKMATE | FIFTY | STALEMATE | THREEFOLD | MAXHIST | MAXPOS))
        {
            int idx = 0;
            bools |= COMMAND;
            if (bools & CHECKMATE)
            {
                idx += snprintf(board.notes + idx,
                                NOTES_LENGTH - strlen(board.notes),
                                "Checkmate!\n");
            }
            else
            {
                if (bools & FIFTY)
                {
                    idx += snprintf(board.notes + idx,
                                    NOTES_LENGTH - strlen(board.notes),
                                    "50 move rule\n");
                }
                else if (bools & THREEFOLD)
                {
                    idx += snprintf(board.notes + idx,
                                    NOTES_LENGTH - strlen(board.notes),
                                    "Three fold repetition\n");
                }
                idx += snprintf(board.notes + idx,
                                NOTES_LENGTH - strlen(board.notes),
                                "Stalemate!\n");
            }
            char* pgn = export_pgn(&board);
            idx += snprintf(board.notes + idx,
                            NOTES_LENGTH - strlen(board.notes), "%s\n", pgn);
            if (last_pgn)
                free(last_pgn);
            last_pgn = malloc(strlen(pgn) + 1);
            memcpy(last_pgn, pgn, strlen(pgn) + 1);
            free(pgn);

            idx += snprintf(board.notes + idx, 
                    NOTES_LENGTH - strlen(board.notes), 
                    "\nTo play again, use \e[38;5;40m: new\e[m \n");
        }
    }
    printf("\n");
    if (white_engine.pid)
        stop_engine(&white_engine);
    if (black_engine.pid)
        stop_engine(&black_engine);
    if (host >= 0)
        CloseClient(host);
    if (client >= 0)
        CloseServer(client);
    if (last_pgn)
        free(last_pgn);

    // Switch to regular buffer
    //printf("\e[?1049l");

    return 0;
}

/* Prints the most recent move to the screen as if it were a part of a PGN */
void print_last_move(Board* board)
{
    Move record = board->history[board->history_count - 1];
    if (board->history_count % 2)
        dprintf(2 , "%d. ", board->history_count / 2 + 1);
    if (record.src_piece & KING)
        dprintf(2 , "K");
    if (record.src_piece & QUEEN)
        dprintf(2 , "Q");
    if (record.src_piece & ROOK)
        dprintf(2 , "R");
    if (record.src_piece & KNIGHT)
        dprintf(2 , "N");
    if (record.src_piece & BISHOP)
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
        if (record.promotion & QUEEN)
            dprintf(2 , "Q");
        if (record.promotion & ROOK)
            dprintf(2 , "R");
        if (record.promotion & KNIGHT)
            dprintf(2 , "N");
        if (record.promotion & BISHOP)
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

/* Runs a game between my custom engine and itself. If silent is non-zero, there
 * will be no output regarding the game
 */
int engine_v_engine(char* fen, int silent)
{
    int running = 1;
    Board board;
    if (fen != NULL)
        load_fen(&board, fen);
    else
        default_board(&board);
    memcpy(board.black_name, "My Engine\0", 10);
    memcpy(board.white_name, "My Engine\0", 10);
    int game_win = -2;
    while (running)
    {
        Move engine_move;
        if (board.to_move)
            engine_move = Emateinone(&board);
        else
            engine_move = Econdensed(&board, 4);

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
        if (game_win == 2)
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
        else if (game_win > 2)
        {
            if (!silent)
            {
                if (game_win == 8)
                    printf("50 move rule\n");
                else if (game_win == 0x10)
                    printf("Three fold repetition\n");
                printf("Stalemate!\n");
                char* pgn = export_pgn(&board);
                printf("%s\n", pgn);
                free(pgn);
            }
            running = 0;
        }
    }
    if (game_win == 2 && board.to_move)
        return 1;
    else if (game_win == 2 && !board.to_move)
        return -1;
    else if (game_win > 2)
        return 0;
    else 
        return game_win;
}

/* Play a game against my custom engine */
void play_build_in_engine(char* fen)
{
    int running = 1;
    int flipped = 0;
    Board board;
    if (fen != NULL)
        load_fen(&board, fen);
    else
        default_board(&board);
    memcpy(board.black_name, "My Engine\0", 10);
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
            Move engine_move = Econdensed(&board, 3);

            move_piece(&board, &engine_move);
        }
        else
        {
            char move[50];

            printf(": ");
            scanf("%30s", move);
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
        if (game_win == 2)
        {
            printf("Checkmate!\n");
            running = 0;
            char* pgn = export_pgn(&board);
            printf("%s\n", pgn);
            free(pgn);
        }
        else if (game_win > 2)
        {
            printf("Stalemate!\n");
            running = 0;
            char* pgn = export_pgn(&board);
            printf("%s\n", pgn);
            free(pgn);
        }
    }
}

/* Play a game against the loaded engine. It does not have to be stockfish, even
 * though that is what I named this function. I will probably rename it at some
 * point.
 */
int engine_v_stockfish(Engine* engine, int silent, FILE* fp)
{
    int running = 1;
    Board board;
    default_board(&board);
    send_ucinewgame(engine->write);
    memcpy(board.black_name, engine->name, strlen(engine->name) + 1);
    memcpy(board.white_name, "My Engine\0", 10);
    int game_win = -2;
    while (running)
    {
        Move engine_move;
        if (board.to_move)
            engine_move = get_engine_move(&board, engine);
        else
            engine_move = Econdensed(&board, 4);

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
        print_fancy(&board);
        char fen[FEN_SIZE];
        export_fen(&board, fen);
        printf("%s\n", fen);

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
        if (game_win == 2)
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
        else if (game_win > 2)
        {
            if (!silent)
            {
                if (game_win == 8)
                    printf("50 move rule\n");
                else if (game_win == 0x10)
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
    if (game_win == 2 && board.to_move)
        return 1;
    else if (game_win == 2 && !board.to_move)
        return -1;
    else if (game_win > 2)
        return 0;
    else 
        return game_win;
}

/* Play a game against the loaded uci engine */
void play_engine(Engine* engine)
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
            scanf("%30s", move);
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
        if (game_win == 2)
        {
            printf("Checkmate!\n");
            running = 0;
            char* pgn = export_pgn(&board);
            printf("%s\n", pgn);
            free(pgn);
        }
        else if (game_win > 2)
        {
            printf("Stalemate!\n");
            running = 0;
            char* pgn = export_pgn(&board);
            printf("%s\n", pgn);
            free(pgn);
        }
    }
}

void thousand_games(Engine* white_engine, Engine* black_engine)
{
    int i;
    int valid_move;
    int black_win = 0;
    int white_win = 0;
    int draw = 0;
    int result = 0;
    clock_t t = clock();
    FILE* games = fopen("thousand_games.txt", "w");
    Board board;
    for (i = 0; i < 1000; ++i)
    {
        clock_t in_t = clock();
        default_board(&board);
        send_ucinewgame(white_engine->write);
        send_ucinewgame(black_engine->write);
        while (result == 0)
        {
            Move engine_move;
            if (board.to_move)
            {
                engine_move = get_engine_move(&board, black_engine);
            }
            else
            {
                engine_move = get_engine_move(&board, white_engine);
            }
            valid_move = move_piece(&board, &engine_move);
            if (valid_move == -1)
            {
                print_debug("%d to play\n", board.to_move);
                print_debug("Move: %d Trying %c%d to %c%d\n", board.moves,
                        engine_move.src_file + 'a' , engine_move.src_rank + 1, 
                        engine_move.dest%8+'a', 8-engine_move.dest/8);
            }
            result = is_gameover(&board);
        }

        in_t = clock() - in_t;
        double t_taken = ((double)in_t)/CLOCKS_PER_SEC;
        printf("Time taken for game %d: %f seconds\n", i + 1, t_taken);
        if (result == 2 && !board.to_move)
            black_win++;
        else if (result == 2 && board.to_move)
            white_win++;
        else if (result & 0x1C)
            draw++;
        if (games)
        {
            char* pgn = export_pgn(&board);
            if (fprintf(games, "%s\n\n", pgn) < 0)
            {
                perror("Couldn't write to file");
                exit(1);
            }
            free(pgn);
        }
    }
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
    printf("Time taken: %f seconds\nWhite: %d\nBlack: %d\nDraw: %d\n", 
            time_taken, white_win, black_win, draw);
    fclose(games);
    stop_engine(white_engine);
    stop_engine(black_engine);
}

void initialize_black(char* e_path, int depth, Board* board, Engine* engine)
{
    start_engine(engine, e_path);
    memcpy(board->black_name, engine->name, strlen(engine->name) + 1);
    send_ucinewgame(engine->write);
    engine->depth = depth;
}

void initialize_white(char* e_path, int depth, Board* board, Engine* engine)
{
    start_engine(engine, e_path);
    memcpy(board->white_name, engine->name, strlen(engine->name) + 1);
    send_ucinewgame(engine->write);
    engine->depth = depth;
}

void prand(Board *board, Engine* white_engine, Engine* black_engine)
{
    /* Threshold is how much better an engine must be before being 
     * nerfed. 
     *
     * Precision is how many sig figs in the percentage. */
    const float threshold = .25;
    const int min_games = 10;
    const int num_games = 100;
    const int precision = 100;
    int i;
    int valid_move;
    int black_win = 0;
    int white_win = 0;
    int draw = 0;
    int result = 0;
    int high = precision;
    int low = 0;
    int mid = precision;
    int nerf = -1;
    FILE* games = fopen("prand.txt", "w");
    long t_out;
    struct timeval timecheck_out;
    gettimeofday(&timecheck_out, NULL);
    t_out = (long)timecheck_out.tv_sec * 1000 + (long)timecheck_out.tv_usec / 1000;
    for (i = 0; i < num_games && high > low + 1; ++i)
    {
        default_board(board);
        if (i & 1)
        {
            memcpy(&board->white_name, black_engine->name, 
                   strlen(black_engine->name) + 1);
            memcpy(&board->black_name, white_engine->name, 
                   strlen(white_engine->name) + 1);
        }
        else 
        {
            memcpy(&board->black_name, black_engine->name, 
                   strlen(black_engine->name) + 1);
            memcpy(&board->white_name, white_engine->name, 
                   strlen(white_engine->name) + 1);
        }
        send_ucinewgame(white_engine->write);
        send_ucinewgame(black_engine->write);
        result = 0;
        long t_in;
        struct timeval timecheck_in;
        gettimeofday(&timecheck_in, NULL);
        t_in = (long)timecheck_in.tv_sec * 1000 + (long)timecheck_in.tv_usec / 1000;
        while (result == 0)
        {
            Move engine_move;
            /* xor i & 1 swaps sides every game */
            if (board->to_move ^ (i & 1))
            {
                if (nerf == 1 && rand()%precision > mid)
                {
                    engine_move = Erandom_move(board);
                } 
                else 
                {
                    engine_move = get_engine_move(board, white_engine);
                }
            }
            else
            {
                if (nerf == 0 && (rand()%precision) > mid)
                {
                    engine_move = Erandom_move(board);
                }
                else
                {
                    engine_move = get_engine_move(board, black_engine);
                }
            }

            valid_move = move_piece(board, &engine_move);
            if (valid_move == -1)
            {
                if (board->to_move ^ (i & 1))
                {
                    dprintf(2, "%s to play\n", white_engine->name);
                }
                else 
                {
                    dprintf(2, "%s to play\n", black_engine->name);
                }
                char fen[FEN_SIZE];
                export_fen(board, fen);
                printf("%s\n", fen);
                dprintf(2, "Move: %d Trying %c%d to %c%d\n", board->moves,
                        engine_move.src_file + 'a' , 8 - engine_move.src_rank, 
                        engine_move.dest%8+'a', 8-engine_move.dest/8);
            }
            result = is_gameover(board);
        }
        gettimeofday(&timecheck_in, NULL);
        t_in = (long)timecheck_in.tv_sec * 1000 + (long)timecheck_in.tv_usec / 1000 - t_in;
        double t_taken = ((double)t_in) / 1000;
        printf("Time taken for game %d: %f seconds\n", i + 1, t_taken);
        if (result == 2 && board->to_move ^ (i & 1))
            black_win++;
        else if (result == 2 && !(board->to_move ^ (i & 1)))
            white_win++;
        else if (result & 0x1C)
            draw++;
        if (games)
        {
            char* pgn = export_pgn(board);
            if (fprintf(games, "%s\n\n", pgn) < 0)
            {
                perror("Couldn't write to file");
                exit(1);
            }
            free(pgn);
        }

        int score_diff = black_win - white_win;
        /* abs(score_diff) */
        score_diff = score_diff > 0 ? score_diff : -1*score_diff;
        /* Should an engine be nerfed? */
        if ((float)i * threshold < score_diff && i > min_games)
        {
            if (nerf < 0) 
            {
                if (white_win > black_win)
                {
                    nerf = 1;
                }
                else 
                {
                    nerf = 0;
                }
            }
            if ((white_win > black_win) ^ nerf)
            {
                low = mid;
            }
            else
            {
                high = mid;
            }
            mid = (high + low) >> 1;
            i=-1;
            black_win = 0;
            white_win = 0;
            draw = 0;
        }
    }
    gettimeofday(&timecheck_out, NULL);
    t_out = (long)timecheck_out.tv_sec * 1000 + (long)timecheck_out.tv_usec / 1000 - t_out;
    double t_taken = ((double)t_out) / 1000;
    if (nerf == 0)
    {
        printf("%s at depth %d equates to %s at depth %d performing at %f%%.", 
               white_engine->name, white_engine->depth, black_engine->name, 
               black_engine->depth, 100*(float)mid/(float)precision);
    } 
    else if (nerf == 1)
    {
        printf("%s at depth %d equates to %s at depth %d performing at %f%%.", 
               black_engine->name, black_engine->depth, white_engine->name, 
               white_engine->depth, 100*(float)mid/(float)precision);
    }
    else 
    {
        printf("%s and %s are evenly matched.", black_engine->name, 
               white_engine->name);
    }
    printf("\nTime taken: %f seconds\n", t_taken);
    printf("Enter a command to continue\n");
    fclose(games);
    stop_engine(white_engine);
    stop_engine(black_engine);
}

void sanity_check(Board* board, Engine* engine)
{
    Board copy;
    int i;
    for (i = 0; i < 1000; ++i)
    {
        memcpy(&copy, board, sizeof(Board));
        Move move = get_engine_move(&copy, engine);
        int valid_move = move_piece(&copy, &move);
        if (valid_move == -1)
        {
            char fen[FEN_SIZE];
            export_fen(board, fen);
            printf("%s\n ----------------------------------------------", fen);
            dprintf(2, "Move: %d Trying %c%d to %c%d\n", board->moves,
                    move.src_file + 'a' , 8 - move.src_rank, 
                    move.dest%8+'a', 8-move.dest/8);
        }
        else
            dprintf(2, "Y\n");
    }
}
