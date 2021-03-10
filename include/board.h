#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#define MAX_HISTORY 1000
#define MAX_STORED_POSITIONS 102
#define MAX_POSITION_STRING 82

enum Pieces
{
    pawn   = 1,
    bishop = 2,
    knight = 4,
    rook   = 8,
    queen  = 16,
    king   = 32,
    all_pieces = 63
};

enum Color
{
    white = 0,
    black = 128
};

typedef struct
{
    int8_t dest;
    int8_t src_rank;
    int8_t src_file;
    int8_t castle;
    uint8_t src_piece;
    uint8_t piece_taken;
    uint8_t gave_check;
    uint8_t game_over;
    uint8_t promotion;
} Move;

typedef struct 
{
    char* white_name;
    char* black_name;
    uint8_t position[64];
    uint8_t to_move;
    uint8_t castling;
    int8_t en_p;
    uint8_t halfmoves;
    uint8_t moves;
    uint8_t wking_pos;
    uint8_t bking_pos;
    uint16_t history_count;
    Move history[MAX_HISTORY];
    char position_hist[MAX_STORED_POSITIONS][MAX_POSITION_STRING];
    uint16_t pos_count;
} Board;

void default_board(Board* board);
void empty_board(Board* board);
void move_square(Board* board, int src, int dest);
void move_verbose(Board* board, char* dest, char* src);
void move_san(Board* board, char* move);
int is_gameover(Board* board);
int move_piece(Board* board, Move* move);
struct found* find_attacker(Board* board, int square, uint8_t piece);
int is_legal(Board* board, int dest, int src);
int is_attacked(Board* board, int square);
int is_checkmate(Board* board, int which_color);

void stress_test(Board* board, int times);

#endif
