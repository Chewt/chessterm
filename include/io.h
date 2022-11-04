#ifndef IO_H
#define IO_H

#include "board.h"

#define FEN_SIZE 100

void board_stats(Board* board);
void print_board(Board* board);
void print_fancy(Board* board);
void print_flipped(Board* board);
void print_fancy_flipped(Board* board);
void load_fen(Board* board, char* fen);
void export_fen(Board* board, char* fen);
char* export_pgn(Board* board);
char* export_moves(Board* board);

#endif
