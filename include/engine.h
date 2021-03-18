#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"
#define MOVES_PER_POSITION 218

Move Erandom_move(Board* board);
Move Eaggressive_move(Board* board);
Move Eape_move(Board* board);
Move Eideal(Board* board, int protecc);
Move Emateinone(Board* board);

#endif
