#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "uci.h"
#include "board.h"
#include "io.h"
#include "dynarray.h"

#define ENGINE_READ  to_engine[0]
#define CLIENT_WRITE to_engine[1]
#define CLIENT_READ  from_engine[0]
#define ENGINE_WRITE from_engine[1]

#ifdef DEBUG
#define print_debug(...) fprintf(stderr,__VA_ARGS__)
#else
#define print_debug(...) ((void)0)
#endif

extern const Move default_move;

/* This forks and execs a uci compatible engine, then populates an Engine struct
 * with the file descriptors to read and write commands to the engine. Startup
 * commands are also sent to the engine in this function
 */
void start_engine(Engine* engine, char* engine_exc)
{
    int to_engine[2];
    int from_engine[2];
    int result = pipe(to_engine);
    while (result == -1)
        result = pipe(to_engine);
    result = pipe(from_engine);
    if (result == -1)
        result = pipe(from_engine);
    pid_t child_pid = fork();
    if (child_pid)
    {
        close(ENGINE_READ);
        close(ENGINE_WRITE);
        engine->read = CLIENT_READ;
        engine->write = CLIENT_WRITE;
        engine->pid = child_pid;
        send_uci(CLIENT_WRITE);
        send_isready(CLIENT_WRITE);
        char* message = get_message(CLIENT_READ);
        while (!strstr(message, "readyok"))
        {
            print_debug("%s\n", message);
            if (strstr(message, "id name"))
                memcpy(engine->name, message + 8, strlen(message) - 8);
            engine->name[strlen(message) - 7] = '\0';
            free(message);
            message = get_message(CLIENT_READ);
        }
        print_debug("%s\n", message);
        free(message);
    }
    else
    {
        FILE* err = fopen("/dev/null", "w");
        dup2(ENGINE_READ,  0);
        dup2(ENGINE_WRITE, 1);
        dup2(fileno(err),  2);

        close(CLIENT_READ);
        close(CLIENT_WRITE);

        char** args = calloc(sizeof(char*), 100);

        int i = 0;
        char* save_ptr;
        char* token;
        token = strtok_r(engine_exc, " ", &save_ptr);
        while (token != NULL)
        {
            args[i] = malloc(strlen(token) + 1);
            memcpy(args[i], token, strlen(token) + 1);
            token = strtok_r(NULL, " ", &save_ptr);
            ++i;
        }
        execvp(args[0], args);
    }
}

/* Sends the quit command to the engine, closes its file descriptors and waits
 * on the process 
 */
void stop_engine(Engine* engine)
{
    send_quit(engine->write);
    close(engine->read);
    close(engine->write);
    int ws;
    waitpid(engine->pid, &ws, 0);
    engine->pid = 0;
}

/* Returns the best move returned by an engine. It provides the engine with
 * a fen of the current position and allows the engine to look at most 10 moves
 * deep
 */
Move get_engine_move(Board* board, Engine* engine)
{
    char curr_position[FEN_SIZE];
    export_fen(board, curr_position);
    send_position(engine->write, "fen", curr_position);

    sprintf(curr_position, "depth %d", engine->depth);
    send_go(engine->write, curr_position);

    char* message = get_message(engine->read);
    while (!strstr(message, "bestmove"))
    {
        print_debug("%s\n", message);
        free(message);
        message = get_message(engine->read);
    }
    char* save_ptr;
    char* token;
    token = strtok_r(message, " ", &save_ptr);
    token = strtok_r(NULL, " ", &save_ptr);
    Move move = default_move;
    move.src_file = token[0] - 'a';
    move.src_rank = 8 - (token[1] - '0');
    move.src_piece = board->position[move.src_file + (move.src_rank * 8)];
    int dest = (token[2] - 'a') + (8 - (token[3] - '0')) * 8;
    move.dest = dest;
    move.promotion |= move.src_piece & 0x80;
    print_debug("%s %u %u %u %u\n", token, move.src_file, move.src_rank,
           move.src_piece, move.dest);
    free(message);
    return move;
}

void send_uci(int fd)
{
    const char* message = "uci\n";
    write(fd, message, strlen(message));
}

void send_debug(int fd, int option)
{
    char* message = "debug    \n";
    if (option)
        message = "debug  on\n";
    else
        message = "debug off\n";
    write(fd, message, strlen(message));
}

void send_isready(int fd)
{
    const char* message = "isready\n";
    write(fd, message, strlen(message));
}

void send_setoption(int fd, const char* name, const char* value)
{
    const char* message = "setoption name ";
    write(fd, message, strlen(message));
    write(fd, name, strlen(name));
    if (value)
    {
        write(fd, " value ", 7);
        write(fd, value, strlen(value));
    }
    write(fd, "\n", 1);
}

void send_register(int fd, const char* token)
{
    const char* message = "register ";
    write(fd, message, strlen(message));
    write(fd, token, strlen(token));
    write(fd, "\n", 1);
}

void send_ucinewgame(int fd)
{
    const char* message = "ucinewgame\n";
    write(fd, message, strlen(message));
}

void send_position(int fd, char* mode, char* moves)
{
    const char* message = "position ";
    write(fd, message, strlen(message));
    write(fd, mode, strlen(mode));
    if (moves)
    {
        write(fd, " ", 1);
        write(fd, moves, strlen(moves));
    }
    write (fd, "\n", 1);
}

void send_go(int fd, char* options)
{
    const char* message = "go ";
    write(fd, message, strlen(message));
    if (options)
        write(fd, options, strlen(options));
    write(fd, "\n", 1);
}

void send_stop(int fd)
{
    const char* message = "stop\n";
    write(fd, message, strlen(message));
}

void send_ponderhit(int fd)
{
    const char* message = "ponderhit\n";
    write(fd, message, strlen(message));
}

void send_quit(int fd)
{
    const char* message = "quit\n";
    write(fd, message, strlen(message));
}

/* Gets a single line of output from the engine. */
char* get_message(int fd)
{
    struct dynarray* all_bytes = dynarray_create();
    int bytes_read = 0;
    do 
    {
        char* curr_byte = malloc(1);
        int charsRead = read(fd, curr_byte, 1);
        dynarray_insert(all_bytes, curr_byte);
        if (charsRead < 0)
        {
            perror("Failed to read");
            print_debug("fd: %d\n", fd);
            exit(1);
        }
        bytes_read++;
    }while(*(char*)dynarray_get(all_bytes, bytes_read - 1) != '\n');
    char* buffer = malloc(bytes_read);
    int i;
    for (i = 0; i < bytes_read; ++i)
    {
        char* curr_char = (char*)dynarray_get(all_bytes, i);
        buffer[i] = *curr_char;
        free(curr_char);
    }
    buffer[bytes_read - 1] = '\0';
    dynarray_free(all_bytes);
    return buffer;
}
