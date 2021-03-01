#include <stdlib.h>
#include "board.h"

const Move default_move = 
{
    .dest = -1,
    .src_rank = -1,
    .src_file = -1,
    .castle = -1,
    .src_piece = pawn,
    .piece_taken = 0,
    .gave_check = 0,
    .game_over = 0,
    .promotion = queen
};

struct found 
{
    int num_found;
    int squares[16];
    int en_p_taken;
    int made_en_p;
    int promotion;
};

Move Erandom_move(Board* board)
{
    Move move = default_move;
    move.promotion = pawn << (rand() % 4 + 1);
    move.promotion |= (board->to_move) ? black : white;
    while (move.dest == -1)
    {
        int start_square = rand() % 64;
        uint8_t pieces = all_pieces;
        pieces |= (board->to_move) ? black : white;
        struct found* found_moves = find_attacker(board, start_square, pieces);
        if (found_moves->num_found)
        {
            int choice = rand() % found_moves->num_found;
            move.dest = start_square;
            move.src_piece = board->position[found_moves->squares[choice]];
            move.src_rank = found_moves->squares[choice] / 8;
            move.src_file = found_moves->squares[choice] % 8;
        }
        free(found_moves);
    }
    return move;
}
