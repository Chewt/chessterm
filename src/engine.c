#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "engine.h"
#include "io.h"

#ifdef DEBUG
#define print_debug(...) fprintf(stderr,__VA_ARGS__)
#else
#define print_debug(...) ((void)0)
#endif

/* Prints the algebraic form of a square to stdout */
void print_square(int i)
{
    printf("%c", i/8+'a');
    printf("%d", 8-i/8);
}

/* Returns non-zero if moving src to dest will result in check */
int gives_check(Board* board, int dest, int src)
{
    board->to_move = !board->to_move;
    int result = !is_legal(board, dest, src);
    board->to_move = !board->to_move;
    return result;
}

/* Returns non-zero if moving src to dest will result in checkmate */
int gives_checkmate(Board* board, int dest, int src)
{
    uint8_t original_piece = board->position[dest];
    move_square(board, dest, src);
    board->to_move = !board->to_move;
    uint8_t color = (board->to_move) ? BLACK : WHITE;
    int result = is_checkmate(board, color);
    move_square(board, src, dest);
    board->to_move = !board->to_move;
    board->position[dest] = original_piece;
    return result;
}

/* Returns non-zero if src is covered by at least one other friendly piece.
 * use omit to specify a square that will not take part in the check
 */
int is_protected(Board* board, int src, int omit)
{
    uint8_t orig;
    if (omit != -1)
    {
        orig = board->position[omit];
        board->position[omit] = 0;
    }
    Found found;
    uint8_t orig_src = board->position[src];
    uint8_t opp_color = (orig_src & 0x80) ^ 0x80;
    board->position[src] = PAWN | opp_color;
    find_attacker(board, src, ALL_PIECES, &found);
    board->position[src] = orig_src;
    int result = found.num_found;
    if (omit != -1)
        board->position[omit] = orig;
    return result;
}

/* Returns non-zero if src is protected and none of the attackers are of lesser
 * value than it. This is because generally someone will take a piece of higher
 * value even if it means sacrifcing their piec. 
 */
int is_safe(Board* board, int src)
{
    if (!is_attacked(board, src))
        return 1;
    board->to_move = !board->to_move;
    Found found;
    find_attacker(board, src, ALL_PIECES, &found);
    board->to_move = !board->to_move;
    int lowest_value = 9;
    int i;
    for (i = 0; i < found.num_found; ++i)
    {
        int curr = found.squares[i];
        if (get_value(*board, curr) < lowest_value)
            lowest_value = get_value(*board, curr);
    }
    if (lowest_value >= get_value(*board, src) && 
          found.num_found <= is_protected(board, src, -1))
        return 1;
    return 0;
}

/* Returns non-zero if moving src to dest will make target satisfy the 
 * requirements of the is_safe() function.
 */
int will_protect(Board* board, int dest, int src, int target)
{
    if (target < 0)
        return 0;
    uint8_t original_piece = board->position[dest];
    move_square(board, dest, src);
    int result = is_safe(board, target);
    move_square(board, src, dest);
    board->position[dest] = original_piece;
    return result;
}

/* Returns non-zero if moving src to dest will result in src being attacked */
int will_be_attacked(Board* board, int dest, int src)
{
    uint8_t orig = board->position[dest];
    move_square(board, dest, src);
    int result = is_attacked(board, dest);
    move_square(board, src, dest);
    board->position[dest] = orig;
    return result;
}

/* Returns non-zero is moving src to dest will result in dest satisfying the
 * is_safe() function
 */
int is_safe_move(Board* board, int dest, int src)
{
    uint8_t orig = board->position[dest];
    move_square(board, dest, src);
    int result = is_safe(board, dest);
    move_square(board, src, dest);
    board->position[dest] = orig;
    return result;
}

int comp_cand(const void* one, const void* two)
{
    if (((Candidate*)one)->weight > ((Candidate*)two)->weight)
        return -1;
    else if (((Candidate*)one)->weight < ((Candidate*)two)->weight)
        return 1;
    return 0;
}

/* Gets all possible moves from the position and sorts them */
void get_all_moves(Board* board, Candidate* cans)
{
    int i;
    memset(cans, 0, sizeof(Candidate) * MOVES_PER_POSITION);
    int cans_ind = 0;
    uint8_t color = (board->to_move) ? BLACK : WHITE;
    for (i = 0; i < 64; ++i)
    {
        Found found;
        find_attacker(board, i, ALL_PIECES | color, &found);
        int j;
        for (j = 0; j < found.num_found; ++j)
        {
            cans[cans_ind].move = default_move;
            cans[cans_ind].move.dest = i;
            cans[cans_ind].move.src_piece = board->position[found.squares[j]];
            cans[cans_ind].move.src_rank = found.squares[j] / 8;
            cans[cans_ind].move.src_file = found.squares[j] % 8;
            cans[cans_ind].weight = 1;
            /* Gives checkmate */
            /*
            if (gives_checkmate(board, i, found.squares[j]))
                cans[cans_ind].weight += 100;
            */
            /* Gives check */
            /*
            if (gives_check(board, i, found.squares[j]))
                cans[cans_ind].weight++;
            */
            /* Is moving to a safe square */
            if (is_safe_move(board, i, found.squares[j]))
                cans[cans_ind].weight += 4;
            /* Takes a piece of higher value than itself */
            if (get_value(*board, i) > get_value(*board, found.squares[j]))
                cans[cans_ind].weight++;
            /* Takes an enemy piece */
            if ((board->position[i] & 0x80) == (color ^ 0x80))
                cans[cans_ind].weight++;
            /* Protects a hanging piece */
            /*
            if (will_protect(board, i, found.squares[j], hanging))
                cans[cans_ind].weight++;
            */
            cans_ind++;
        }
    }
    qsort(cans, MOVES_PER_POSITION, sizeof(Candidate), comp_cand);
}

/* Returns a random legal move */
Move Erandom_move(Board* board)
{
    print_debug("Playing Random Move\n");
    Move move = default_move;
    move.promotion = PAWN << (rand() % 4 + 1);
    move.promotion |= (board->to_move) ? BLACK : WHITE;
    while (move.dest == -1)
    {
        int start_square = rand() % 64;
        uint8_t pieces = ALL_PIECES;
        pieces |= (board->to_move) ? BLACK : WHITE;
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

/* Returns a random move that will take an enemy piece */
Move Eaggressive_move(Board* board)
{
    print_debug("Playing Aggressive Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = ALL_PIECES;
    uint8_t opp_color = (board->to_move) ? WHITE : BLACK;
    move.promotion |= (board->to_move) ? BLACK : WHITE;
    pieces |= (board->to_move) ? BLACK : WHITE;
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

/* Returns a move that will put the enemy king in check */
Move Eape_move(Board* board)
{
    print_debug("Playing Ape Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = ALL_PIECES;
    pieces |= (board->to_move) ? BLACK : WHITE;
    move.promotion |= (board->to_move) ? BLACK : WHITE;
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

/* Returns a random safe move */
Move Esafe(Board* board, int protecc)
{
    if (protecc)
        print_debug("Protecc Playing safe Move\n");
    else
        print_debug("Playing safe Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = ALL_PIECES;
    pieces |= (board->to_move) ? BLACK : WHITE;
    move.promotion |= (board->to_move) ? BLACK : WHITE;
    uint8_t color = pieces & 0x80;
    int start_square = rand() % 64;
    int hanging = -1;
    if (protecc)
    {
        for (i = 0; i < 64; ++i)
        {
            if (board->position[i] && (board->position[i] & 0x80) == color &&
                                      !is_safe(board, i))
                if (get_value(*board, i) > get_value(*board, hanging))
                    hanging = i;
        }
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
            if (is_safe_move(board, curr_square, target))
            {
                if (hanging != -1)
                {
                    print_debug("Will %c%d  to %c%d save %c%d? ",
                            target%8+'a',8-target/8,
                            curr_square%8+'a',8-curr_square/8,
                            hanging%8+'a',8-hanging/8);
                    int found = 0;
                    if (hanging == target)
                        found = 1;
                    else if (will_protect(board, curr_square, target, hanging))
                        found = 1;
                    if (found)
                        print_debug("Yes.\n");
                    else
                        print_debug("No.\n");
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

/* Returns a random move that simultaneously takes a piece, and is safe */
Move Esafeaggro(Board* board, int protecc)
{
    if (protecc)
        print_debug("Protecc Playing safeaggro Move\n");
    else
        print_debug("Playing safeaggro Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = ALL_PIECES;
    uint8_t opp_color = (board->to_move) ? WHITE : BLACK;
    pieces |= (board->to_move) ? BLACK : WHITE;
    move.promotion |= (board->to_move) ? BLACK : WHITE;
    uint8_t color = pieces & 0x80;
    int start_square = rand() % 64;
    int hanging = -1;
    if (protecc)
    {
        for (i = 0; i < 64; ++i)
        {
            if (board->position[i] && (board->position[i] & 0x80) == color &&
                    !is_safe(board, i))
                if (get_value(*board, i) > get_value(*board, hanging))
                    hanging = i;
        }
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
                if (is_safe_move(board, curr_square, target) ||
                        get_value(*board, curr_square) > get_value(*board, target))
                {
                if (hanging != -1)
                {
                    int found = 0;
                    if (hanging == target)
                        found = 1;
                    else if (will_protect(board, curr_square, target, hanging))
                        found = 1;
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

/* Returns a random move that simultanously puts the enemy king in check, while
 * also is a safe move
 */
Move Enohang(Board* board, int protecc)
{
    if (protecc)
        print_debug("Protecc Playing nohang Move\n");
    else
        print_debug("Playing nohang Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = ALL_PIECES;
    pieces |= (board->to_move) ? BLACK : WHITE;
    move.promotion |= (board->to_move) ? BLACK : WHITE;
    uint8_t color = pieces & 0x80;
    int hanging = -1;
    if (protecc)
    {
        for (i = 0; i < 64; ++i)
        {
            if (board->position[i] && (board->position[i] & 0x80) == color &&
                    !is_safe(board, i))
                if (get_value(*board, i) > get_value(*board, hanging))
                    hanging = i;
        }
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
                     (is_safe_move(board, curr_square, target) ||
                      get_value(*board, curr_square) > get_value(*board, target)))
            {
                if (hanging != -1)
                {
                    int found = 0;
                    if (hanging == target)
                        found = 1;
                    else if (will_protect(board, curr_square, target, hanging))
                        found = 1;
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

/* Returns a move that is safe, takes a piece, and puts the enemy king in check
 */
Move Eideal(Board* board, int protecc)
{
    if (protecc)
        print_debug("Protecc Playing ideal Move\n");
    else
        print_debug("Playing ideal Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = ALL_PIECES;
    uint8_t opp_color = (board->to_move) ? WHITE : BLACK;
    pieces |= (board->to_move) ? BLACK : WHITE;
    move.promotion |= (board->to_move) ? BLACK : WHITE;
    uint8_t color = pieces & 0x80;
    int hanging = -1;
    if (protecc)
    {
        for (i = 0; i < 64; ++i)
        {
            if (board->position[i] && (board->position[i] & 0x80) == color &&
                    !is_safe(board, i))
                if (get_value(*board, i) > get_value(*board, hanging))
                    hanging = i;
        }
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
                        (is_safe_move(board, curr_square, target) ||
                         get_value(*board, curr_square) > get_value(*board, target)))
                {
                if (hanging != -1)
                {
                    int found = 0;
                    if (hanging == target)
                        found = 1;
                    else if (will_protect(board, curr_square, target, hanging))
                        found = 1;
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

/* Returns a move that will put the king in checkmate */
Move Emateinone(Board* board)
{
    print_debug("Playing mateinone Move\n");
    Move move = default_move;
    int i;
    uint8_t pieces = ALL_PIECES;
    uint8_t opp_color = (board->to_move) ? WHITE : BLACK;
    pieces |= (board->to_move) ? BLACK : WHITE;
    move.promotion |= (board->to_move) ? BLACK : WHITE;
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

int evaluate_move(Board* board, Candidate can, int depth)
{
    Board temp_board = *board;
    int black_score[6];
    int white_score[6];
    int old_score = 0;
    get_material_scores(temp_board, white_score, black_score);
    if (!temp_board.to_move)
        old_score = white_score[0] - black_score[0];
    else
        old_score = black_score[0] - white_score[0];
    move_piece(&temp_board, &can.move);
    get_material_scores(temp_board, white_score, black_score);
    int board_value = 200;
    if (depth > 0)
    {
        Candidate cans[MOVES_PER_POSITION];
        get_all_moves(&temp_board, cans);
        int i;
        int temp;
        for (i = 0; i < MOVES_PER_POSITION; ++i)
        {
            if (cans[i].weight <= 0)
                continue;
            temp = -1 * evaluate_move(&temp_board, cans[i], depth - 1);
            if (temp < board_value)
                board_value = temp;
        }

        //return board_value;
        if (temp_board.to_move)
            return white_score[0] - black_score[0] - old_score + board_value;
        else
            return black_score[0] - white_score[0] - old_score + board_value;
    }
    else
    {
        if (temp_board.to_move)
            return white_score[0] - black_score[0] - old_score;
        else
            return black_score[0] - white_score[0] - old_score;
    }
}

int eval_prune(Board* board, Candidate can, int alpha, int beta, int depth)
{
    Board temp_board = *board;
    move_piece(&temp_board, &can.move);
    if (check_stalemate(board, temp_board.to_move))
        return 0;
    if (depth == 0)
    {
        int black_score[6];
        int white_score[6];
        get_material_scores(temp_board, white_score, black_score);
        return white_score[0] - black_score[0];
    }
    else
    {
        Candidate cans[MOVES_PER_POSITION];
        get_all_moves(&temp_board, cans);
        int i;
        int board_value;
        int temp = 0;
        if (temp_board.to_move)
            board_value = 300;
        else
            board_value = -300;
        for (i = 0; i < MOVES_PER_POSITION; ++i)
        {
            if (cans[i].weight <= 0)
                break;
            if (temp_board.to_move)
            {
                temp = eval_prune(&temp_board, cans[i], alpha, beta, depth - 1);
                if (temp < board_value)
                    board_value = temp;
                beta = (temp < beta) ? temp : beta;
                if (beta <= alpha)
                    break;
            }
            else
            {
                temp = eval_prune(&temp_board, cans[i], alpha, beta, depth - 1);
                if (temp > board_value)
                    board_value = temp;
                alpha = (temp > alpha) ? temp : alpha;
                if (beta <= alpha)
                   break;
            }
        }
        return board_value;
    }
}

Move Econdensed(Board* board, int depth)
{
    Board temp_board;
    memcpy(&temp_board, board, sizeof(Board));
    Candidate cans[MOVES_PER_POSITION];
    get_all_moves(board, cans);
    int i;
    Candidate best = cans[0];
    //print_fancy(board);
    int j;
    for (j = 1; j <= depth; ++j)
    {
        for (i = 0; i < 10; ++i)
        {
            if (cans[i].weight <= 0)
                break;
            /*printf("BEFORE: Cand %c%d to %c%d",cans[i].move.src_file + 'a', 
              8 - cans[i].move.src_rank,
              cans[i].move.dest%8+'a',8-cans[i].move.dest/8);
              printf(" with weight: %d\n", cans[i].weight);
              */
            int new_weight = eval_prune(board, cans[i], -300, 300, j);
            if (board->to_move)
                new_weight *= -1;
            cans[i].weight += new_weight;
            if (j == depth)
            {
                printf("Cand %c%d to %c%d",cans[i].move.src_file + 'a', 
                        8 - cans[i].move.src_rank,
                        cans[i].move.dest%8+'a',8-cans[i].move.dest/8);
                printf(" with weight: %d\n", cans[i].weight);
            }
            if (cans[i].weight > best.weight)
                best = cans[i];
        }
        qsort(cans, MOVES_PER_POSITION, sizeof(Candidate), comp_cand);
    }
    for (i = 0; i < MOVES_PER_POSITION; ++i)
        if (cans[i].weight != best.weight)
            break;
    if (i - 1 > 0)
        best = cans[rand() % (i - 1)];
    return best.move;
}
