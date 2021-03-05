#ifndef IO_H
#define IO_H

void board_stats(Board* board);
void print_board(Board* board);
void print_fancy(Board* board);
void print_flipped(Board* board);
void load_fen(Board* board, char* fen);
char* export_fen(Board* board);
char* export_pgn(Board* board);

#endif
