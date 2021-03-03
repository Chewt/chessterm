#include <stdio.h>
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

Move Eaggressive_move(Board* board)
{
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    uint8_t opp_color = (board->to_move) ? white : black;
    pieces |= (board->to_move) ? black : white;
    int start_square = rand() % 64;
    for (i = 0; i < 64; ++i)
    {
        int curr_square = (start_square + i) % 64;
        if (board->position[curr_square] && 
                (board->position[curr_square] & 0x80) == opp_color)
        {
            struct found* found_moves = find_attacker(board, curr_square,
                                                             pieces);
            if (found_moves->num_found)
            {
                int choice = rand() % found_moves->num_found;
                move.dest = curr_square;
                move.src_piece = board->position[found_moves->squares[choice]];
                move.src_rank = found_moves->squares[choice] / 8;
                move.src_file = found_moves->squares[choice] % 8;
                free(found_moves);
                break;
            }
            free(found_moves);
        }
    }
    if (move.dest == -1)
        return Erandom_move(board);
    else
        return move;
}

Move Eape_move(Board* board)
{
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    pieces |= (board->to_move) ? black : white;
    int start_square = rand() % 64;
    for (i = 0; i < 64; ++i)
    {
        struct found* found_moves = find_attacker(board, i, pieces);
        int j;
        board->to_move = !board->to_move;
        for (j = 0; j < found_moves->num_found; ++j)
            if (!is_legal(board, i, found_moves->squares[j]))
            {
                move.dest = i;
                move.src_piece = board->position[found_moves->squares[j]];
                move.src_rank = found_moves->squares[j] / 8;
                move.src_file = found_moves->squares[j] % 8;
                break;
            }
        board->to_move = !board->to_move;
        free(found_moves);
    }
    if (move.dest == -1)
        return Eaggressive_move(board);
    else
        return move;
}
