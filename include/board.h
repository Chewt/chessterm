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

struct board
{
    uint8_t position[64];
    uint8_t to_move;
    uint8_t castling;
    int8_t en_p;
    uint8_t halfmoves;
    uint8_t moves;
};

void default_board(struct board* board);
void print_board(struct board* board);
void load_fen(struct board* board, char* fen);
void board_stats(struct board* board);
void move_piece(struct board* board, int src, int dest);
void move_verbose(struct board* board, char* dest, char* src);
void move_san(struct board* board, char* move);

#endif
