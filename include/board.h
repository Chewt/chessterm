#ifndef BOARD_H
#define BOARD_H

#include "dynarray.h"

struct board
{
    dynarray* white;
    dynarray* black;
    int turn;
}

struct board* create_board();

void free_board(struct board* board);

int movepiece(struct board* board, int piece, char file, int rank);

#endif
