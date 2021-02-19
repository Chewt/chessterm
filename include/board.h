#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

enum Pieces
{
    pawn   = 1,
    bishop = 2,
    knight = 4,
    rook   = 8,
    queen  = 16,
    king   = 32

};

enum Color
{
    white = 0,
    black = 128
};

typedef struct 
{
    uint8_t position[64];
    uint8_t to_move;
    uint8_t castling;
    int8_t en_p;
    uint8_t halfmoves;
    uint8_t moves;
    uint8_t wking_pos;
    uint8_t bking_pos;
} Board;

void default_board(Board* board);
void print_board(Board* board);
void load_fen(Board* board, char* fen);
void board_stats(Board* board);
void move_piece(Board* board, int src, int dest);
void move_verbose(Board* board, char* dest, char* src);
void move_san(Board* board, char* move);

#endif
