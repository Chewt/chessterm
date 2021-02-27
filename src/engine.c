#include <stdlib.h>
#include "board.h"

const Move default_move = 
{
    .dest = -1,
    .castle = -1,
    .src_piece = pawn,
    .piece_taken = 0,
    .gave_check = 0,
    .game_over = 0,
    .promotion = 0
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
    int start_square = rand() % 64;
    int i;
    uint8_t pieces = all_pieces;
    pieces |= (board->to_move) ? black : white;
    for (i = 0; i < 64; ++i)
    {
        struct found* found_moves = find_attacker(board, 
                (i + start_square) % 63, pieces);
        if (found_moves->num_found)
        {
            move.dest = (i + start_square) % 63;
            move.src_piece = board->position[found_moves->squares[0]];
            free(found_moves);
            return move;
        }
    }
    return move;
}
