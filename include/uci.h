#ifndef UCI_H
#define UCI_H

#include "board.h"
enum
{
    OFF,
    ON
};

void start_engine(char* engine_exc, int* rwfds);
Move get_engine_move(Board* board, int* fds);

void send_uci(int fd);
void send_debug(int fd, int option);
void send_isready(int fd);
void send_setoption(int fd, const char* name, const char* value);
void send_register(int fd, const char* token);
void send_ucinewgame(int fd);
void send_position(int fd, char* mode, char* moves);
void send_go(int fd, char* options);
void send_stop(int fd);
void send_ponderhit(int fd);
void send_quit(int fd);

char* get_message(int fd);

#endif
