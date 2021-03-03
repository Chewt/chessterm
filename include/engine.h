#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"
#define MOVE_MAX 274

Move Erandom_move(Board* board);
Move Eaggressive_move(Board* board);
Move Eape_move(Board* board);
Move Eideal(Board* board);

#endif
