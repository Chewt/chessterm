#include <stdio.h>
#include <stdlib.h>
#include "board.h"

#define DEBUG

#ifdef DEBUG
#include "io.h"
#define print_debug(...) fprintf(stderr,__VA_ARGS__)
#else
#define print_debug(...) ((void)0)
#endif

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

void print_square(int i)
{
    printf("%c", i/8+'a');
    printf("%d", 8-i/8);
}

Move Erandom_move(Board* board)
{
    print_debug("Playing Random Move\n");
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
    print_debug("Playing Aggressive Move\n");
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
                move.src_piece = board->position[found_moves->squares [choice]];
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

int gives_check(Board* board, int dest, int src)
{
    board->to_move = !board->to_move;
    int result = !is_legal(board, dest, src);
    board->to_move = !board->to_move;
    return result;
}

int gives_checkmate(Board* board, int dest, int src)
{
    uint8_t original_piece = board->position[dest];
    move_square(board, dest, src);
    uint8_t opp_color = (board->position[src] & 0x80) ^ 0x80;
    int result = is_checkmate(board, opp_color);
    move_square(board, src, dest);
    board->position[dest] = original_piece;
    return result;
}

int is_protected(Board* board, int src)
{
    board->to_move = !board->to_move;
    int result = is_attacked(board, src);
    board->to_move = !board->to_move;
    return result;
}

Move Eape_move(Board* board)
{
    print_debug("Playing Ape Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    pieces |= (board->to_move) ? black : white;
    int start_square = rand() % 64;
    for (i = 0; i < 64; ++i)
    {
        int curr_square = (start_square + i) % 64;
        struct found* found_moves = find_attacker(board, curr_square, pieces);
        int j;
        for (j = 0; j < found_moves->num_found; ++j)
            if (gives_check(board, curr_square, found_moves->squares[j]))
            {
                move.dest = curr_square;
                move.src_piece = board->position[found_moves->squares[j]];
                move.src_rank = found_moves->squares[j] / 8;
                move.src_file = found_moves->squares[j] % 8;
                break;
            }
        free(found_moves);
    }
    if (move.dest == -1)
        return Eaggressive_move(board);
    else
        return move;
}

Move Esafe(Board* board)
{
    print_debug("Playing safe Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    pieces |= (board->to_move) ? black : white;
    uint8_t color = pieces & 0x80;
    int start_square = rand() % 64;
    int hanging_squares[16] = {-1};
    int hang_count = 0;
    for (i = 0; i < 64; ++i)
        if (board->position[i] && (board->position[i] & 0x80) == color &&
                is_attacked(board, i))
            if (!is_protected(board, i))
            {
                hanging_squares[hang_count++] = i;
            }
    for (i = 0; i < 128; ++i)
    {
        int curr_square = (rand() % 64 + i) % 64;
        struct found* found_moves = find_attacker(board, curr_square, pieces);
        int j;
        for (j = 0; j < found_moves->num_found; ++j)
        {
            int target = found_moves->squares[j];
            if (curr_square == 42)
                print_debug("%c%d to c3 found\n", target%8+'a',8-target/8);
            if (!is_attacked(board, curr_square))
            {
                if (hang_count)
                {
                    int k;
                    int found = 0;
                    for (k = 0; k < hang_count; ++k)
                        if (hanging_squares[k] == found_moves->squares[j])
                        {
                            target = hanging_squares[k];
                            found = 1;
                            break;
                        }
                    if (!found)
                        break;
                }
                move.dest = curr_square;
                move.src_piece = board->position[target];
                move.src_rank = target / 8;
                move.src_file = target % 8;
                break;
            }
        }
        free(found_moves);
    }
    if (move.dest == -1)
        return Eape_move(board);
    else
        return move;
}

Move Esafeaggro(Board* board)
{
    print_debug("Playing safeaggro Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    uint8_t opp_color = (board->to_move) ? white : black;
    pieces |= (board->to_move) ? black : white;
    uint8_t color = pieces & 0x80;
    int start_square = rand() % 64;
    int hanging_squares[16] = {-1};
    int hang_count = 0;
    for (i = 0; i < 64; ++i)
        if (board->position[i] && (board->position[i] & 0x80) == color &&
                is_attacked(board, i))
            if (!is_protected(board, i))
            {
                hanging_squares[hang_count++] = i;
            }
    for (i = 0; i < 128; ++i)
    {
        int curr_square = (rand() % 64 + i) % 64;
        if (board->position[curr_square] &&
                (board->position[curr_square] & 0x80) == opp_color)
        {
            struct found* found_moves = find_attacker(board, curr_square,
                    pieces);
            int j;
            for (j = 0; j < found_moves->num_found; ++j)
                if (!is_attacked(board, curr_square))
                {
                    int target = found_moves->squares[j];
                    if (hang_count)
                    {
                        int k;
                        int found = 0;
                        for (k = 0; k < hang_count; ++k)
                            if (hanging_squares[k] == found_moves->squares[j])
                            {
                                target = hanging_squares[k];
                                found = 1;
                                break;
                            }
                        if (!found)
                            break;
                    }
                    move.dest = curr_square;
                    move.src_piece = board->position[target];
                    move.src_rank = target / 8;
                    move.src_file = target % 8;
                    break;
                }
            free(found_moves);
        }
        if (move.dest != -1)
            break;
    }
    if (move.dest == -1)
        return Esafe(board);
    else
        return move;
}

Move Enohang(Board* board)
{
    print_debug("Playing nohang Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    pieces |= (board->to_move) ? black : white;
    uint8_t color = pieces & 0x80;
    int hanging_squares[16] = {-1};
    int hang_count = 0;
    for (i = 0; i < 64; ++i)
        if (board->position[i] && (board->position[i] & 0x80) == color &&
                is_attacked(board, i))
            if (!is_protected(board, i))
            {
                hanging_squares[hang_count++] = i;
            }
    int start_square = rand() % 64;
    for (i = 0; i < 128; ++i)
    {
        int curr_square = (rand() % 64 + i) % 64;
        struct found* found_moves = find_attacker(board, curr_square, pieces);
        int j;
        for (j = 0; j < found_moves->num_found; ++j)
            if (gives_check(board, curr_square, found_moves->squares[j]) && 
                    !is_attacked(board, curr_square))
            {
                int target = found_moves->squares[j];
                if (hang_count)
                {
                    int k;
                    int found = 0;
                    for (k = 0; k < hang_count; ++k)
                        if (hanging_squares[k] == found_moves->squares[j])
                        {
                            target = hanging_squares[k];
                            found = 1;
                            break;
                        }
                    if (!found)
                        break;
                }
                move.dest = curr_square;
                move.src_piece = board->position[target];
                move.src_rank = target / 8;
                move.src_file = target % 8;
                break;
            }
        free(found_moves);
        if (move.dest != -1)
            break;
    }
    if (move.dest == -1)
        return Esafeaggro(board);
    else
        return move;
}

Move Eideal(Board* board)
{
    print_debug("Playing ideal Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    uint8_t opp_color = (board->to_move) ? white : black;
    pieces |= (board->to_move) ? black : white;
    uint8_t color = pieces & 0x80;
    int hanging_squares[16] = {-1};
    int hang_count = 0;
    for (i = 0; i < 64; ++i)
        if (board->position[i] && (board->position[i] & 0x80) == color &&
                is_attacked(board, i))
            if (!is_protected(board, i))
            {
                print_square(i);
                printf("\n");
                hanging_squares[hang_count++] = i;
            }
    for (i = 0; i < 128; ++i)
    {
        int curr_square = (rand() % 64 + i) % 64;
        if (board->position[curr_square] && 
                (board->position[curr_square] & 0x80) == opp_color)
        {
            struct found* found_moves = find_attacker(board, curr_square,
                    pieces);
            int j;
            for (j = 0; j < found_moves->num_found; ++j)
                if (gives_check(board, curr_square, found_moves->squares[j]) &&
                        !is_attacked(board, curr_square))
                {
                    int target = found_moves->squares[j];
                    if (hang_count)
                    {
                        int k;
                        int found = 0;
                        for (k = 0; k < hang_count; ++k)
                            if (hanging_squares[k] == found_moves->squares[j])
                            {
                                target = hanging_squares[k];
                                found = 1;
                                break;
                            }
                        if (!found)
                            break;
                    }
                    move.dest = curr_square;
                    move.src_piece = board->position[target];
                    move.src_rank = target / 8;
                    move.src_file = target % 8;
                    break;
                }
            free(found_moves);
        }
        if (move.dest != -1)
            break;
    }
    if (move.dest == -1)
        return Enohang(board);
    else
        return move;
}

Move Emateinone(Board* board)
{
    print_debug("Playing mateinone Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    uint8_t opp_color = (board->to_move) ? white : black;
    pieces |= (board->to_move) ? black : white;
    for (i = 0; i < 64; ++i)
    {
        struct found* found_moves = find_attacker(board, i, pieces);
        int j;
        for (j = 0; j < found_moves->num_found; ++j)
            if (gives_checkmate(board, i, found_moves->squares[j]))
            {
                move.dest = i;
                move.src_piece = board->position[found_moves->squares[j]];
                move.src_rank = found_moves->squares[j] / 8;
                move.src_file = found_moves->squares[j] % 8;
                break;
            }
        free(found_moves);
        if (move.dest != -1)
            break;
    }
    if (move.dest == -1)
        return Eideal(board);
    else
        return move;
}
