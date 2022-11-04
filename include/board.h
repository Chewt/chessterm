#ifndef BOARD_H
#define BOARD_H
#include <stdint.h>
#define MAX_HISTORY 1000
#define MAX_STORED_POSITIONS 102
#define MAX_POSITION_STRING 82

enum Pieces
{
    PAWN   = 1,
    BISHOP = 2,
    KNIGHT = 4,
    ROOK   = 8,
    QUEEN  = 16,
    KING   = 32,
    ALL_PIECES = 63
};

enum Color
{
    WHITE = 0,
    BLACK = 128
};

/* Holds information about squares found by find_attacker() */
typedef struct
{
    int num_found;
    int squares[16];
    int en_p_taken;
    int made_en_p;
    int promotion;
    int castle;
} Found;

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
    char white_name[100];
    char black_name[100];
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

enum
{
    FLIPPED   = 0x001,
    CHECKMATE = 0x002,
    STALEMATE = 0x004,
    FIFTY     = 0x008,
    THREEFOLD = 0x010,
    MAXHIST   = 0x020,
    MAXPOS    = 0x040,
    AUTOFLIP  = 0x100,
    RANDOMSIDE= 0x200,
    COMMAND   = 0x400,
    STOP      = 0x80000000
};

extern const Move default_move;

void default_board(Board* board);
void empty_board(Board* board);
void move_square(Board* board, int src, int dest);
void move_verbose(Board* board, char* dest, char* src);
int move_san(Board* board, char* move);
int is_gameover(Board* board);
int move_piece(Board* board, Move* move);
void find_attacker(Board* board, int square, uint8_t piece, Found* founds);
int is_legal(Board* board, int dest, int src);
int is_attacked(Board* board, int square);
int is_checkmate(Board* board, int which_color);
int check_stalemate(Board* board, int which_color);
int get_value(Board* board, int square);
void get_material_scores(Board* board, int* white, int* black);
void UndoMove(Board* board);

void stress_test(Board* board, int times);

#endif
