#ifndef COMMANDS_H
#define COMMANDS_H
#include "board.h"

#define COMMAND_LENGTH 256

typedef struct
{
    char name[COMMAND_LENGTH];
    void* function;

} Command;

int is_networked_command(char input[COMMAND_LENGTH]);
int ProcessCommand(Board* board, char input[COMMAND_LENGTH]);

#endif
