#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "engine.h"

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
        Found found_moves;
        find_attacker(board, start_square, pieces, &found_moves);
        if (found_moves.num_found)
        {
            int choice = rand() % found_moves.num_found;
            move.dest = start_square;
            move.src_piece = board->position[found_moves.squares[choice]];
            move.src_rank = found_moves.squares[choice] / 8;
            move.src_file = found_moves.squares[choice] % 8;
        }
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
    move.promotion |= (board->to_move) ? black : white;
    pieces |= (board->to_move) ? black : white;
    int start_square = rand() % 64;
    for (i = 0; i < 64; ++i)
    {
        int curr_square = (start_square + i) % 64;
        if (board->position[curr_square] && 
                (board->position[curr_square] & 0x80) == opp_color)
        {
            Found found_moves;
            find_attacker(board, curr_square, pieces, &found_moves);
            if (found_moves.num_found)
            {
                int choice = rand() % found_moves.num_found;
                move.dest = curr_square;
                move.src_piece = board->position[found_moves.squares [choice]];
                move.src_rank = found_moves.squares[choice] / 8;
                move.src_file = found_moves.squares[choice] % 8;
                break;
            }
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
    board->to_move = !board->to_move;
    uint8_t color = (board->to_move) ? black : white;
    int result = is_checkmate(board, color);
    move_square(board, src, dest);
    board->to_move = !board->to_move;
    board->position[dest] = original_piece;
    return result;
}

int is_protected(Board* board, int src, int omit)
{
    board->to_move = !board->to_move;
    uint8_t orig;
    if (omit != -1)
    {
        orig = board->position[omit];
        board->position[omit] = 0;
    }
    int result = is_attacked(board, src);
    if (omit != -1)
        board->position[omit] = orig;
    board->to_move = !board->to_move;
    return result;
}

int will_protect(Board* board, int dest, int src, int target)
{
    uint8_t original_piece = board->position[dest];
    move_square(board, dest, src);
    int result = is_protected(board, target, -1);
    if (!is_attacked(board, target))
        result = 1;
    move_square(board, src, dest);
    board->position[dest] = original_piece;
    return result;
}

int will_be_attacked(Board* board, int dest, int src)
{
    uint8_t orig = board->position[dest];
    move_square(board, dest, src);
    int result = is_attacked(board, dest);
    move_square(board, src, dest);
    board->position[dest] = orig;
    return result;
}

int get_value(Board* board, int square)
{
    uint8_t piece = board->position[square];
    if (piece & pawn)
        return 1;
    else if (piece & bishop || piece & knight)
        return 3;
    else if (piece & rook)
        return 5;
    else if (piece & queen)
        return 9;
    else if (piece & king)
        return 10;
    else
        return 0;
}

Move Eape_move(Board* board)
{
    print_debug("Playing Ape Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    pieces |= (board->to_move) ? black : white;
    move.promotion |= (board->to_move) ? black : white;
    int start_square = rand() % 64;
    for (i = 0; i < 64; ++i)
    {
        int curr_square = (start_square + i) % 64;
        Found found_moves;
        find_attacker(board, curr_square, pieces, &found_moves);
        int j;
        for (j = 0; j < found_moves.num_found; ++j)
            if (gives_check(board, curr_square, found_moves.squares[j]))
            {
                move.dest = curr_square;
                move.src_piece = board->position[found_moves.squares[j]];
                move.src_rank = found_moves.squares[j] / 8;
                move.src_file = found_moves.squares[j] % 8;
                break;
            }
    }
    if (move.dest == -1)
        return Eaggressive_move(board);
    else
        return move;
}

Move Esafe(Board* board, int protecc)
{
    if (protecc)
        print_debug("Protecc Playing safe Move\n");
    else
        print_debug("Playing safe Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    pieces |= (board->to_move) ? black : white;
    move.promotion |= (board->to_move) ? black : white;
    uint8_t color = pieces & 0x80;
    int start_square = rand() % 64;
    int hanging_squares[16] = {-1};
    int hang_count = 0;
    if (protecc)
    {
        for (i = 0; i < 64; ++i)
        {
            if (board->position[i] && (board->position[i] & 0x80) == color &&
                    is_attacked(board, i))
            {
                if (!is_protected(board, i, -1))
                {
                    hanging_squares[hang_count++] = i;
                }
            }
        }
        print_debug("Found: %d hanging pieces\n", hang_count);
        for (i = 0; i < hang_count; ++i)
            print_debug("==%c%d\n",hanging_squares[i]%8+'a',
                                   8-hanging_squares[i]/8);
    }
    for (i = 0; i < 128; ++i)
    {
        int curr_square = (rand() % 64 + i) % 64;
        Found found_moves;
        find_attacker(board, curr_square, pieces, &found_moves);
        int j;
        for (j = 0; j < found_moves.num_found; ++j)
        {
            int target = found_moves.squares[j];
            if (!will_be_attacked(board, curr_square, target) || 
                    is_protected(board, curr_square, target))
            {
                if (hang_count)
                {
                    int k;
                    int found = 0;
                    for (k = 0; k < hang_count; ++k)
                    {
                        if (hanging_squares[k] == target)
                        {
                            found = 1;
                            break;
                        }
                        else if (will_protect(board, curr_square, target,
                                    hanging_squares[k]))
                        {
                            found = 1;
                            break;
                        }
                    }
                    if (!found)
                        continue;
                }
                move.dest = curr_square;
                move.src_piece = board->position[target];
                move.src_rank = target / 8;
                move.src_file = target % 8;
                break;
            }
        }
    }
    if (move.dest == -1)
    {
        if (protecc)
            return Eideal(board, 0);
        else
            return Eape_move(board);
    }
    else
        return move;
}

Move Esafeaggro(Board* board, int protecc)
{
    if (protecc)
        print_debug("Protecc Playing safeaggro Move\n");
    else
        print_debug("Playing safeaggro Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    uint8_t opp_color = (board->to_move) ? white : black;
    pieces |= (board->to_move) ? black : white;
    move.promotion |= (board->to_move) ? black : white;
    uint8_t color = pieces & 0x80;
    int start_square = rand() % 64;
    int hanging_squares[16] = {-1};
    int hang_count = 0;
    if (protecc)
    {
        for (i = 0; i < 64; ++i)
        {
            if (board->position[i] && (board->position[i] & 0x80) == color &&
                    is_attacked(board, i))
            {
                if (!is_protected(board, i, -1))
                {
                    hanging_squares[hang_count++] = i;
                }
            }
        }
        print_debug("Found: %d hanging pieces\n", hang_count);
        for (i = 0; i < hang_count; ++i)
            print_debug("==%c%d\n",hanging_squares[i]%8+'a',
                                   8-hanging_squares[i]/8);
    }
    for (i = 0; i < 128; ++i)
    {
        int curr_square = (rand() % 64 + i) % 64;
        if (board->position[curr_square] &&
                (board->position[curr_square] & 0x80) == opp_color)
        {
            Found found_moves;
            find_attacker(board, curr_square, pieces, &found_moves);
            int j;
            for (j = 0; j < found_moves.num_found; ++j)
            {
                int target = found_moves.squares[j];
                if (!will_be_attacked(board, curr_square, target) || 
                       is_protected(board, curr_square, target) ||
                       get_value(board, curr_square) > get_value(board, target))
                {
                    if (hang_count)
                    {
                        int k;
                        int found = 0;
                        for (k = 0; k < hang_count; ++k)
                        {
                            if (hanging_squares[k] == found_moves.squares[j])
                            {
                                target = hanging_squares[k];
                                found = 1;
                                break;
                            }
                            else if (will_protect(board, curr_square, target,
                                                  hanging_squares[k]))
                            {
                                found = 1;
                                break;
                            }
                        }
                        if (!found)
                            continue;
                    }
                    move.dest = curr_square;
                    move.src_piece = board->position[target];
                    move.src_rank = target / 8;
                    move.src_file = target % 8;
                    break;
                }
            }
        }
        if (move.dest != -1)
            break;
    }
    if (move.dest == -1)
    {
        if (protecc)
            return Esafe(board, 1);
        else
            return Esafe(board, 0);
    }
    else
        return move;
}

Move Enohang(Board* board, int protecc)
{
    if (protecc)
        print_debug("Protecc Playing nohang Move\n");
    else
        print_debug("Playing nohang Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    pieces |= (board->to_move) ? black : white;
    move.promotion |= (board->to_move) ? black : white;
    uint8_t color = pieces & 0x80;
    int hanging_squares[16] = {-1};
    int hang_count = 0;
    if (protecc)
    {
        for (i = 0; i < 64; ++i)
        {
            if (board->position[i] && (board->position[i] & 0x80) == color &&
                    is_attacked(board, i))
            {
                if (!is_protected(board, i, -1))
                {
                    hanging_squares[hang_count++] = i;
                }
            }
        }
        print_debug("Found: %d hanging pieces\n", hang_count);
        for (i = 0; i < hang_count; ++i)
            print_debug("==%c%d\n",hanging_squares[i]%8+'a',
                                   8-hanging_squares[i]/8);
    }
    int start_square = rand() % 64;
    for (i = 0; i < 128; ++i)
    {
        int curr_square = (rand() % 64 + i) % 64;
        Found found_moves;
        find_attacker(board, curr_square, pieces, &found_moves);
        int j;
        for (j = 0; j < found_moves.num_found; ++j)
        {
            int target = found_moves.squares[j];
            if (gives_check(board, curr_square, found_moves.squares[j]) && 
                    (!will_be_attacked(board, curr_square, target) || 
                     is_protected(board, curr_square, target)) ||
                    get_value(board, curr_square) > get_value(board, target))
            {
                if (hang_count)
                {
                    int k;
                    int found = 0;
                    for (k = 0; k < hang_count; ++k)
                    {
                        if (hanging_squares[k] == found_moves.squares[j])
                        {
                            target = hanging_squares[k];
                            found = 1;
                            break;
                        }
                        else if (will_protect(board, curr_square, target,
                                              hanging_squares[k]))

                        {
                            found = 1;
                            break;
                        }
                    }
                    if (!found)
                        continue;
                }
                move.dest = curr_square;
                move.src_piece = board->position[target];
                move.src_rank = target / 8;
                move.src_file = target % 8;
                break;
            }
        }
        if (move.dest != -1)
            break;
    }
    if (move.dest == -1)
    {
        if (protecc)
            return Esafeaggro(board, 1);
        else
            return Esafeaggro(board, 0);
    }
    else
    {
        return move;
    }
}

Move Eideal(Board* board, int protecc)
{
    if (protecc)
        print_debug("Protecc Playing ideal Move\n");
    else
        print_debug("Playing ideal Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = all_pieces;
    uint8_t opp_color = (board->to_move) ? white : black;
    pieces |= (board->to_move) ? black : white;
    move.promotion |= (board->to_move) ? black : white;
    uint8_t color = pieces & 0x80;
    int hanging_squares[16] = {-1};
    int hang_count = 0;
    if (protecc)
    {
        for (i = 0; i < 64; ++i)
        {
            if (board->position[i] && (board->position[i] & 0x80) == color &&
                    is_attacked(board, i))
            {
                if (!is_protected(board, i, -1))
                {
                    hanging_squares[hang_count++] = i;
                }
            }
        }
        print_debug("Found: %d hanging pieces\n", hang_count);
        for (i = 0; i < hang_count; ++i)
            print_debug("==%c%d\n",hanging_squares[i]%8+'a',
                                   8-hanging_squares[i]/8);
    }
    for (i = 0; i < 128; ++i)
    {
        int curr_square = (rand() % 64 + i) % 64;
        if (board->position[curr_square] && 
                (board->position[curr_square] & 0x80) == opp_color)
        {
            Found found_moves;
            find_attacker(board, curr_square, pieces, &found_moves);
            int j;
            for (j = 0; j < found_moves.num_found; ++j)
            {
                int target = found_moves.squares[j];
                if (gives_check(board, curr_square, found_moves.squares[j]) &&
                        (!will_be_attacked(board, curr_square, target) || 
                         is_protected(board, curr_square, target)))
                {
                    if (hang_count)
                    {
                        int k;
                        int found = 0;
                        for (k = 0; k < hang_count; ++k)
                        {
                            if (hanging_squares[k] == found_moves.squares[j])
                            {
                                target = hanging_squares[k];
                                found = 1;
                                break;
                            }
                            else if (will_protect(board, curr_square, target,
                                                  hanging_squares[k]))
                            {
                                found = 1;
                                break;
                            }
                        }
                        if (!found)
                            continue;
                    }
                    move.dest = curr_square;
                    move.src_piece = board->position[target];
                    move.src_rank = target / 8;
                    move.src_file = target % 8;
                    break;
                }
            }
        }
        if (move.dest != -1)
            break;
    }
    if (move.dest == -1)
    {
        if (protecc)
            return Enohang(board, 1);
        else
            return Enohang(board, 0);
    }
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
    move.promotion |= (board->to_move) ? black : white;
    for (i = 0; i < 64; ++i)
    {
        Found found_moves;
        find_attacker(board, i, pieces, &found_moves);
        int j;
        for (j = 0; j < found_moves.num_found; ++j)
        {
            int m = found_moves.squares[j];
            if (gives_checkmate(board, i, found_moves.squares[j]))
            {
                move.dest = i;
                move.src_piece = board->position[found_moves.squares[j]];
                move.src_rank = found_moves.squares[j] / 8;
                move.src_file = found_moves.squares[j] % 8;
                break;
            }
        }
        if (move.dest != -1)
            break;
    }
    if (move.dest == -1)
        return Eideal(board, 1);
    else
        return move;
}
