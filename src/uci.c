#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "uci.h"

#define ENGINE_READ  to_engine[0]
#define CLIENT_WRITE to_engine[1]
#define CLIENT_READ  from_engine[0]
#define ENGINE_WRITE from_engine[1]

void start_engine(const char* engine_exc)
{
    int to_engine[2];
    int from_engine[2];
    if (pipe(to_engine) == -1)
        printf("Uh oh, pipe no worky\n");
    if (pipe(from_engine) == -1)
        printf("Uh oh, pipe no worky\n");
    pid_t child_pid = fork();
    if (child_pid)
    {
        close(ENGINE_READ);
        close(ENGINE_WRITE);
        send_uci(to_engine[1]);
        send_quit(to_engine[1]);
    }
    else
    {
        dup2(ENGINE_READ,  0);
        //dup2(ENGINE_WRITE, 1);

        close(CLIENT_READ);
        close(CLIENT_WRITE);
        close(ENGINE_READ);
        close(ENGINE_WRITE);

        char* args[] = {engine_exc, NULL};
        execvp(engine_exc, args);
    }
}

void send_uci(int fd)
{
    const char* message = "uci\n";
    write(fd, message, strlen(message) + 1);
}

void send_debug(int fd, int option)
{
    char* message = "debug    \n";
    if (option)
        message = "debug  on\n";
    else
        message = "debug off\n";
    write(fd, message, strlen(message) + 1);
}

void send_isready(int fd)
{
    const char* message = "isready\n";
    write(fd, message, strlen(message) + 1);
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
    write(fd, "\n", 2);
}

void send_register(int fd, const char* token)
{
    const char* message = "register ";
    write(fd, message, strlen(message));
    write(fd, token, strlen(token));
    write(fd, "\n", 2);
}

void send_ucinewgame(int fd)
{
    const char* message = "ucinewgame\n";
    write(fd, message, strlen(message) + 1);
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
    write (fd, "\n", 2);
}

void send_go(int fd, char* options)
{
    const char* message = "go ";
    write(fd, message, strlen(message));
    if (options)
        write(fd, options, strlen(options));
    write(fd, "\n", 2);
}

void send_stop(int fd)
{
    const char* message = "stop\n";
    write(fd, message, strlen(message) + 1);
}

void send_ponderhit(int fd)
{
    const char* message = "ponderhit\n";
    write(fd, message, strlen(message) + 1);
}

void send_quit(int fd)
{
    const char* message = "quit\n";
    write(fd, message, strlen(message) + 1);
}
