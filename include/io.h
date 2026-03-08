#ifndef IO_H
#define IO_H

#include "board.h"
#include "config.h"

#define FEN_SIZE 100

void board_stats(Board* board);
void print_board(Board* board);
void print_fancy(Board* board, Config* config);
void print_flipped(Board* board);
void print_fancy_flipped(Board* board, Config* config);
void load_fen(Board* board, char* fen);
void export_fen(Board* board, char* fen);
char* export_pgn(Board* board);
char* export_moves(Board* board);

enum art_styles {
    ASCII,
    UNICODE,
    NUM_ART_STYLES
};

enum chess_pieces {
    CHESS_PAWN,
    CHESS_BISHOP,
    CHESS_KNIGHT,
    CHESS_ROOK,
    CHESS_KING,
    CHESS_QUEEN,
    NUM_PIECES
};

#endif
