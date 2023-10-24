#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"

#ifdef DEBUG
#define print_debug(...) fprintf(stderr,__VA_ARGS__)
#else
#define print_debug(...) ((void)0)
#endif

Board history[MAX_HISTORY];
int n_history = 0;

int is_attacked(Board* board, int square);
int castle(Board* board, int side);
void check_king(Board* board, int square, uint8_t piece, Found* founds);
void store_position(Board* board, char* dest);

const Move default_move = 
{
    .dest = -1,
    .src_rank = -1,
    .src_file = -1,
    .castle = -1,
    .src_piece = PAWN,
    .piece_taken = 0,
    .gave_check = 0,
    .game_over = 0,
    .promotion = QUEEN
};

/* Gets value of piece on passed square */
int get_value(Board* board, int square)
{
    uint8_t piece;
    if (square < 64 && square >= 0)
        piece = board->position[square];
    else
        return 0;
    if (piece & PAWN)
        return 1; 
    else if (piece & BISHOP || piece & KNIGHT)
        return 3;
    else if (piece & ROOK)
        return 5;
    else if (piece & QUEEN)
        return 9;
    else if (piece & KING)
        return 10;
    else
        return 0;
}

/* Populates white_score and black_score with the material score of the board
 */
void get_material_scores(Board* board, int* white_score, int* black_score)
{
    int i;
    white_score[0] = 0; /* First element is score counter */ 
    white_score[1] = 8; /* All other elements are counters for that piece */ 
    white_score[2] = 2;
    white_score[3] = 2;
    white_score[4] = 2;
    white_score[5] = 1;
    black_score[0] = 0;
    black_score[1] = 8;
    black_score[2] = 2;
    black_score[3] = 2;
    black_score[4] = 2;
    black_score[5] = 1;
    for (i = 0; i < 64; ++i)
    {
        if (board->position[i] & 0x80)
        {
            black_score[0] += get_value(board, i);
            if (board->position[i] & PAWN)
                black_score[1]--;
            if (board->position[i] & BISHOP)
                black_score[2]--;
            if (board->position[i] & KNIGHT)
                black_score[3]--;
            if (board->position[i] & ROOK)
                black_score[4]--;
            if (board->position[i] & QUEEN)
                black_score[5]--;
        }
        else
        {
            white_score[0] += get_value(board, i);
            if (board->position[i] & PAWN)
                white_score[1]--;
            if (board->position[i] & BISHOP)
                white_score[2]--;
            if (board->position[i] & KNIGHT)
                white_score[3]--;
            if (board->position[i] & ROOK)
                white_score[4]--;
            if (board->position[i] & QUEEN)
                white_score[5]--;
        }
    }
}

/* Sets board state to all default values */
void empty_board(Board* board)
{
    board->to_move = 0;
    board->castling = 0x00;
    board->en_p = -1;
    board->halfmoves = 0;
    board->moves = 1;
    board->bking_pos = 0;
    board->wking_pos = 0;
    board->pos_count = 0;
    board->white_name[0] = '\0';
    board->black_name[0] = '\0';
    int i;
    for (i = 0; i < 64; ++i)
        board->position[i] = 0;
    board->history_count = 0;
    for (i = 0; i < MAX_HISTORY; ++i)
    {
        board->history[i].dest = -1;
        board->history[i].src_piece = -1;
        board->history[i].src_rank = -1;
        board->history[i].src_file = -1;
        board->history[i].piece_taken = 0;
        board->history[i].gave_check = 0;
        board->history[i].castle = -1;
        board->history[i].game_over = 0;
        board->history[i].promotion = 0;
    }
}

/* Sets board to default chess starting position */
void default_board(Board* board)
{
    int i;
    empty_board(board);
    board->bking_pos = 4;
    board->wking_pos = 60;
    board->castling = 0x0F;
    memcpy(board->white_name, "White\0", 6);
    memcpy(board->black_name, "Black\0", 6);
    board->position[0] = ROOK   | BLACK;
    board->position[1] = KNIGHT | BLACK;
    board->position[2] = BISHOP | BLACK;
    board->position[3] = QUEEN  | BLACK;
    board->position[4] = KING   | BLACK;
    board->position[5] = BISHOP | BLACK;
    board->position[6] = KNIGHT | BLACK;
    board->position[7] = ROOK   | BLACK;
    for (i = 0; i < 8; ++i)
    {
        board->position[i + 8]     = PAWN | BLACK;
        board->position[i + 8 * 6] = PAWN | WHITE;
    }
    board->position[0 + 8 * 7] = ROOK   | WHITE;
    board->position[1 + 8 * 7] = KNIGHT | WHITE;
    board->position[2 + 8 * 7] = BISHOP | WHITE;
    board->position[3 + 8 * 7] = QUEEN  | WHITE;
    board->position[4 + 8 * 7] = KING   | WHITE;
    board->position[5 + 8 * 7] = BISHOP | WHITE;
    board->position[6 + 8 * 7] = KNIGHT | WHITE;
    board->position[7 + 8 * 7] = ROOK   | WHITE;
    store_position(board, board->position_hist[board->pos_count++]);
    if (board->notes)
        board->notes[0] = '\0';
}


/* Move contents of one square to another */
void move_square(Board* board, int dest, int src)
{
    if (board->position[src] == (KING | BLACK))
        board->bking_pos = dest;
    if (board->position[src] == (KING | WHITE))
        board->wking_pos = dest;
    board->position[dest] = board->position[src];
    board->position[src] = 0;
}

/* Move contents of one square to another.     */
/* src and dest are algebraic forms of squares */
/* example: e4                                 */
void move_verbose(Board* board, char* dest, char* src)
{
    uint8_t source_num = src[0] - 'a' + ('8' - src[1]) * 8;
    uint8_t dest_num = dest[0] - 'a' + ('8' - dest[1]) * 8;
    move_square(board, dest_num, source_num);
}

enum Direction
{
    UP    = -8,
    DOWN  =  8,
    LEFT  = -1,
    RIGHT =  1,
    UPR   = -7,
    UPL   = -9,
    DOWNR =  9,
    DOWNL =  7
};

/* Returns zero if moving the contents of src to dest will result in the king
 * being put into check, and non-zero otherwise.
 */
int is_legal(Board* board, int dest, int src)
{
    uint8_t color = board->position[src] & 0x80;
    /* Check if in bounds */
    if (dest < 0 || dest > 63)
        return 0;
    if (board->position[dest] && (board->position[dest] & 0x80) == color)
        return 0;

    /* Create a deep copy of the board */
    Board t_board;
    memcpy(&t_board, board, sizeof(Board));

    /* Make the move on the board copy */
    move_square(&t_board, dest, src);
    if (dest == t_board.en_p && (t_board.position[dest] & PAWN))
    {
        if (color)
            t_board.position[dest + UP] = 0;
        else
            t_board.position[dest + DOWN] = 0;
    }

    int square_check;
    if (board->to_move)
        square_check = t_board.bking_pos;
    else
        square_check = t_board.wking_pos;

    /* Check if that move put the friendly KING in check */
    if (!is_attacked(&t_board, square_check))
        return 1;
    return 0;
}

/* Places all moves from src that are legal into dest */
void check_for_check(Board* board, int square,
        Found* dest, Found* src)
{
    dest->en_p_taken = src->en_p_taken;
    dest->promotion = src->promotion;
    dest->made_en_p = src->made_en_p;
    int i;
    for (i = 0; i < src->num_found; ++i)
    {
        int csquare = src->squares[i];
        if (is_legal(board, square, src->squares[i]))
        {
            dest->num_found++;
            dest->squares[dest->num_found - 1] = src->squares[i];
            if (src->castle != -1 && (csquare == 60 || csquare == 4))
            {
                if (castle(board, src->castle))
                    dest->castle = src->castle;
                else
                {
                    dest->squares[dest->num_found - 1] = -1;
                    dest->num_found--;
                }
            }
        }
    }
}

/* Fills found struct with locations of knights that can move to square 
 * Does not check if the move will result in own KING being put in check
 */
void check_knight(Board* board, int square, uint8_t piece, Found* founds)
{
    if (square + UP + UPL >= 0 && square + UP + UPL <= 63)
        if (square / 8 > 1 && square % 8 > 0)
            if (board->position[square + UP + UPL] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + UP + UPL;
            }
    if (square + LEFT + UPL >= 0 && square + LEFT + UPL <= 63)
        if (square / 8 > 0 && square % 8 > 1)
            if (board->position[square + LEFT + UPL] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + LEFT + UPL;
            }
    if (square + UP + UPR >= 0 && square + UP + UPR <= 63)
        if (square / 8 > 1 && square % 8 < 7)
            if (board->position[square + UP + UPR] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + UP + UPR;
            }
    if (square + RIGHT + UPR >= 0 && square + RIGHT + UPR <= 63)
        if (square / 8 > 0 && square % 8 < 6)
            if (board->position[square + RIGHT + UPR] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + RIGHT + UPR;
            }
    if (square + LEFT + DOWNL >= 0 && square + LEFT + DOWNL <= 63)
        if (square / 8 < 7 && square % 8 > 1)
            if (board->position[square + LEFT + DOWNL] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + LEFT + DOWNL;
            }
    if (square + DOWN + DOWNL >= 0 && square + DOWN + DOWNL <= 63)
        if (square / 8 < 6 && square % 8 > 0)
            if (board->position[square + DOWN + DOWNL] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + DOWN + DOWNL;
            }
    if (square + RIGHT + DOWNR >= 0 && square + RIGHT + DOWNR <= 63)
        if (square / 8 < 7 && square % 8 < 6)
            if (board->position[square + RIGHT + DOWNR] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + RIGHT + DOWNR;
            }
    if (square + DOWN + DOWNR >= 0 && square + DOWN + DOWNR <= 63)
        if (square / 8 < 6 && square % 8 < 7)
            if (board->position[square + DOWN + DOWNR] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + DOWN + DOWNR;
            }
}

/* Fills found struct with locations of rooks that can move to square 
 * Does not check if the move will result in own KING being put in check
 */
void check_rook(Board* board, int square, uint8_t piece, Found* founds)
{
    int i = square;
    while (i % 8 > 0)
    {
        if (i + LEFT >= 0 && i + LEFT <= 63)
        {
            i += LEFT;
            if (board->position[i] != 0 && board->position[i] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = i;
                break;
            }
            else if (board->position[i])
                break;
        }
        else
            break;
    }
    i = square;
    while (i % 8 < 7)
    {
        if (i + RIGHT >= 0 && i + RIGHT <= 63)
        {
            i += RIGHT;
            if (board->position[i] && board->position[i] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = i;
                break;
            }
            else if (board->position[i])
                break;
        }
        else
            break;
    }
    i = square;
    while (i / 8 > 0)
    {
        if (i + UP >= 0 && i + UP <= 63)
        {
            i += UP;
            if (board->position[i] && board->position[i] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = i;
                break;
            }
            else if (board->position[i])
                break;
        }
        else
            break;
    }
    i = square;
    while (i / 8 < 7)
    {
        if (i + DOWN >= 0 && i + DOWN <= 63)
        {
            i += DOWN;
            if (board->position[i] && board->position[i] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = i;
                break;
            }
            else if (board->position[i])
                break;
        }
        else
            break;
    }
}

/* Fills found struct with locations of bishops that can move to square 
 * Does not check if the move will result in own KING being put in check
 */
void check_bishop(Board* board, int square, uint8_t piece, Found* founds)
{
    int i = square;
    while ((i + UPL) % 8 < 7 && (i + UPL) / 8 < 7)
    {
        if (i + UPL >= 0 && i + UPL <= 63)
        {
            i += UPL;
            if (board->position[i] && board->position[i] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = i;
            }
            else if (board->position[i])
                break;
        }
        else
            break;
    }
    i = square;
    while ((i + UPR) % 8 > 0 && (i + UPR) / 8 < 7)
    {
        if (i + UPR >= 0 && i + UPR <= 63)
        {
            i += UPR;
            if (board->position[i] && board->position[i] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = i;
            }
            else if (board->position[i])
                break;
        }
        else
            break;
    }
    i = square;
    while ((i + DOWNL) % 8 < 7 && (i + DOWNL) / 8 > 0)
    {
        if (i + DOWNL >= 0 && i + DOWNL <= 63)
        {
            i += DOWNL;
            if (board->position[i] && board->position[i] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = i;
            }
            else if (board->position[i])
                break;
        }
        else
            break;
    }
    i = square;
    while ((i + DOWNR) % 8 > 0 && (i + DOWNR) / 8 > 0)
    {
        if (i + DOWNR >= 0 && i + DOWNR <= 63)
        {
            i += DOWNR;
            if (board->position[i] && board->position[i] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = i;
            }
            else if (board->position[i])
                break;
        }
        else
            break;
    }
}

/* Fills found struct with locations of pawns that can move to square 
 * Does not check if the move will result in own KING being put in check
 */
void check_pawn(Board* board, int square, uint8_t piece, Found* founds)
{
    if (square == board->en_p)
    {
        if (square / 8 == 2)
        {
            if (board->position[square + DOWNL]  == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + DOWNL;
                founds->en_p_taken = square + DOWN;
            }
            if (board->position[square + DOWNR]  == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + DOWNR;
                founds->en_p_taken = square + DOWN;
            }
        }
        else if (square / 8 == 5)
        {
            if (board->position[square + UPR]  == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + UPR;
                founds->en_p_taken = square + UP;
            }
            if (board->position[square + UPL]  == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + UPL;
                founds->en_p_taken = square + UP;
            }
        }
    }
    if (square / 8 == 4 && !board->position[square])
    {
        uint8_t looking_at = board->position[square + 2 * DOWN];
        if (!(looking_at & 0x80))
            if (looking_at == piece && !(board->position[square + DOWN]))
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + 2 * DOWN;
                founds->made_en_p = square + DOWN;
            }
    }
    else if (square / 8 == 3 && !board->position[square])
    {
        uint8_t looking_at = board->position[square + 2 * UP];
        if (looking_at & 0x80)
            if (looking_at == piece && !(board->position[square + UP]))
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + 2 * UP;
                founds->made_en_p = square + UP;
            }
    }
    else 
        founds->made_en_p = -1;

    if (!(piece & 0x80) && square / 8 == 0)
        founds->promotion = 1;
    if ((piece & 0x80) && square / 8 == 7)
        founds->promotion = 1;

    if (board->to_move)
    {
        if (board->position[square] && !(board->position[square] & BLACK))
        {
            if (square % 8 < 7 && square / 8 > 0)
                if (board->position[square + UPR] == piece)
                {
                    founds->num_found++;
                    founds->squares[founds->num_found - 1] = square + UPR;
                }
            if (square % 8 > 0 && square / 8 > 0)
                if (board->position[square + UPL] == piece)
                {
                    founds->num_found++;
                    founds->squares[founds->num_found - 1] = square + UPL;
                }
        }
        if (square + UP >= 0)
           if(board->position[square + UP] == piece && 
                                             !board->position[square])
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + UP;
            }
    }
    else
    {
        if (board->position[square] && (board->position[square] & BLACK)) 
        {
            if (square % 8 > 0 && square / 8 < 7)
                if (board->position[square + DOWNL] == piece)
                {
                    founds->num_found++;
                    founds->squares[founds->num_found - 1] = square + DOWNL;
                }
            if (square % 8 < 7 && square / 8 < 7)
                if (board->position[square + DOWNR] == piece)
                {
                    founds->num_found++;
                    founds->squares[founds->num_found - 1] = square + DOWNR;
                }
        }
        if (square + DOWN < 64)
            if(board->position[square + DOWN] == piece && 
                                                !board->position[square])
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + DOWN;
        }
    }
}

/* Returns non-zero if any pieces specified can move to square,
 * otherwise it returns zero.
 * Does not check if move will result in check on the king
 */
int can_move_to(Board* board, int square, uint8_t pieces)
{
    Found found_hyp;
    found_hyp.num_found = 0;
    found_hyp.en_p_taken = -1;
    found_hyp.promotion = 0;
    uint8_t color = (board->to_move) ? BLACK : WHITE;
    if (pieces & KNIGHT)
        check_knight(board, square, KNIGHT | color, &found_hyp);
    if (found_hyp.num_found)
        return 1;
    if (pieces & BISHOP)
        check_bishop(board, square, BISHOP | color, &found_hyp);
    if (found_hyp.num_found)
        return 1;
    if (pieces & ROOK)
        check_rook(board, square, ROOK | color, &found_hyp);
    if (found_hyp.num_found)
        return 1;
    if (pieces & PAWN)
        check_pawn(board, square, PAWN | color, &found_hyp);
    if (found_hyp.num_found)
        return 1;
    if (pieces & QUEEN)
        check_rook(board, square, QUEEN | color, &found_hyp);
    if (found_hyp.num_found)
        return 1;
    if (pieces & QUEEN)
        check_bishop(board, square, QUEEN | color, &found_hyp);
    if (found_hyp.num_found)
        return 1;
    if (pieces & KING)
        check_king(board, square, KING | color, &found_hyp);
    if (found_hyp.num_found)
        return 1;
    return 0;
}

/* returns non-zero if the square can be moved to by an opposite color piece */
int is_attacked(Board* board, int square)
{
    board->to_move = !board->to_move;
    uint8_t color = (board->to_move) ? BLACK : WHITE;
    uint8_t prev_piece = board->position[square];
    board->position[square] = PAWN | (color ^ 0x80);
    int check = can_move_to(board, square, ALL_PIECES);
    board->position[square] = prev_piece;
    board->to_move = !board->to_move;
    return check;
}

/* Returns non-zero if castling is possible
 * Does check if move will result in check on the king
 */
int castle(Board* board, int side)
{
    enum {KINGSIDE, QUEENSIDE};
    if (side == KINGSIDE)
    {
        if (!board->to_move && board->castling & 0x08)
        {
            if (!(board->position[61] | board->position[62]))
                if(   !is_attacked(board, 60) 
                   && !is_attacked(board, 61) 
                   && !is_attacked(board, 62))
                    return 1;
        }
        else if(board->to_move && board->castling & 0x02)
        {
            if (!(board->position[5] | board->position[6]))
                if(   !is_attacked(board, 4) 
                   && !is_attacked(board, 5) 
                   && !is_attacked(board, 6))
                    return 1;
        }
    }
    else if (side == QUEENSIDE)
    {
        if (!board->to_move && board->castling & 0x04)
        {
            if (!(board->position[57]|board->position[58]|board->position[59]))
                if(   !is_attacked(board, 60) 
                   && !is_attacked(board, 59) 
                   && !is_attacked(board, 58))
                    return 1;
        }
        else if(board->to_move && board->castling & 0x01)
        {
            if (!(board->position[1]|board->position[2]|board->position[3]))
                if(   !is_attacked(board, 4) 
                   && !is_attacked(board, 3) 
                   && !is_attacked(board, 2))
                    return 1;
        }
    }
    return 0;
}

/* Fills found struct with locations of kings that can move to square 
 * Does not check if the move will result in own KING being put in check
 */
void check_king(Board* board, int square, uint8_t piece, Found* founds)
{
    if (square + LEFT >= 0 && square % 8 > 0)
        if (board->position[square + LEFT] == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + LEFT;
        }
    if (square + RIGHT < 64 && square % 8 < 7)
        if (board->position[square + RIGHT] == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + RIGHT;
        }
    if (square + UPL >= 0 && square + UPL < 64)
        if (square % 8 > 0 && square / 8 > 0)
            if (board->position[square + UPL] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + UPL;
            }
    if (square + DOWNR >= 0 && square + DOWNR < 64)
        if (square % 8 < 7 && square / 8 < 7)
            if (board->position[square + DOWNR] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + DOWNR;
            }
    if (square + UPR >= 0 && square + UPR < 64)
        if (square % 8 < 7 && square / 8 > 0)
            if (board->position[square + UPR] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + UPR;
            }
    if (square + DOWNL >= 0 && square + DOWNL < 64)
        if (square % 8 > 0 && square / 8 < 7)
            if (board->position[square + DOWNL] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + DOWNL;
            }
    if (square + UP >= 0 && square / 8 > 0)
        if (board->position[square + UP] == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + UP;
        }
    if (square + DOWN < 64 && square / 8 < 7)
        if (board->position[square + DOWN] == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + DOWN;
        }
    if ((!board->to_move && square == 62 && board->wking_pos == 60 && (board->castling & 0x8)) ||
         (board->to_move && square == 6 && board->bking_pos == 4 && (board->castling & 0x2)))
    {
        founds->castle = 0;
        founds->num_found++;
        founds->squares[founds->num_found - 1] = square - 2;
    }
    else if ((!board->to_move && square == 58 && board->wking_pos == 60 && (board->castling & 0x4)) || 
            (board->to_move && square == 2 && board->bking_pos == 4 && (board->castling & 0x1)))
    {
        founds->castle = 1;
        founds->num_found++;
        founds->squares[founds->num_found - 1] = square + 2;
    }
}

/* Returns found struct filled with locations of piece
 * Resultant found struct will not contain moves that will put the KING in check
 */
void find_attacker(Board* board, int square, uint8_t piece, Found* founds)
{
    Found src;
    src.num_found = 0;
    src.en_p_taken = -1;
    src.promotion = 0;
    src.made_en_p = -1;
    src.castle = -1;
    founds->num_found = 0;
    founds->en_p_taken = -1;
    founds->promotion = 0;
    founds->made_en_p = -1;
    founds->castle = -1;
    uint8_t color = (board->to_move) ? BLACK : WHITE;
    if (board->position[square] && !((board->position[square] & 0x80) ^ color))
        return;

    if (piece & KNIGHT)
        check_knight(board, square, KNIGHT|color, &src);
    if (piece & ROOK)
        check_rook(board, square, ROOK|color, &src);
    if (piece & BISHOP)
        check_bishop(board, square, BISHOP|color, &src);
    if (piece & KING)
        check_king(board, square,  KING|color, &src);
    if (piece & QUEEN)
        check_rook(board, square, QUEEN|color, &src);
    if (piece & QUEEN)
        check_bishop(board, square, QUEEN|color, &src);
    if (piece & PAWN)
        check_pawn(board, square, PAWN|color, &src);
    print_debug("Num found before check_for_check: %d\n", src.num_found);
    int i;
    for (i = 0; i < src.num_found; ++i)
    {
        print_debug("%c%d\n",
                src.squares[i]%8+'a',8-src.squares[i]/8);
    }
    if (src.num_found)
        check_for_check(board, square, founds, &src);
}

/* Returns 2 if the current position is stalemate
 * by insufficient material to checkmate
 */
int check_stalemate(Board* board, int which_color)
{
    int king_attacked = (which_color) ? board->bking_pos : board->wking_pos;
    uint8_t color = board->position[king_attacked] & 0x80;
    print_debug("KING: %d\n", king_attacked);
    if (is_attacked(board, king_attacked))
        return 0;
    int i;
    int p = 0, b = 0, n = 0, r = 0, q = 0, k = 0;
    int bp = 0, bb = 0, bn = 0, br = 0, bq = 0, bk = 0;
    for(i = 0; i < 64; ++i)
    {
        uint8_t piece = board->position[i];
        if (piece & PAWN)
            (piece & 0x80) ? bp++ : p++;
        else if (piece & BISHOP)
            (piece & 0x80) ? bb++ : b++;
        else if (piece & KNIGHT)
            (piece & 0x80) ? bn++ : n++;
        else if (piece & ROOK)
            (piece & 0x80) ? br++ : r++;
        else if (piece & QUEEN)
            (piece & 0x80) ? bq++ : q++;
        else if (piece & KING)
            (piece & 0x80) ? bk++ : k++;
    }
    if (!(p || r || q || bp || br || bq))
    {
        int stale_white = 0;
        int stale_black = 0;
        if ((!n && b <= 1) || (!b && n <= 1))
            stale_white = 1;
        if ((!bn && bb <= 1) || (!bb && bn <= 1))
            stale_black = 1;
        if (stale_black && stale_white)
            return 0x4;
    }
    if (king_attacked + UP >= 0 && king_attacked / 8 > 0)
        if (is_legal(board, king_attacked + UP, king_attacked))
            return 0;
    print_debug("can move up\n");
    if (king_attacked + UPR >= 0 && king_attacked + UPR < 64)
        if (king_attacked % 8 < 7 && king_attacked / 8 > 0)
            if (is_legal(board, king_attacked + UPR, king_attacked))
                return 0;
    print_debug("upr\n");
    if (king_attacked + UPL >= 0 && king_attacked + UPL < 64)
        if (king_attacked % 8 > 0 && king_attacked / 8 > 0)
            if (is_legal(board, king_attacked + UPL, king_attacked))
                return 0;
    print_debug("upl\n");
    if (king_attacked + LEFT >= 0 && king_attacked % 8 > 0)
        if (is_legal(board, king_attacked + LEFT, king_attacked))
            return 0;
    print_debug("left\n");
    if (king_attacked + RIGHT < 64 && king_attacked % 8 < 7)
        if (is_legal(board, king_attacked + RIGHT, king_attacked))
            return 0;
    print_debug("right\n");
    if (king_attacked + DOWNL >= 0 && king_attacked + DOWNL < 64)
        if (king_attacked % 8 > 0 && king_attacked / 8 < 7)
            if (is_legal(board, king_attacked + DOWNL, king_attacked))
                return 0;
    print_debug("downl\n");
    if (king_attacked + DOWNR >= 0 && king_attacked + DOWNR < 64)
        if (king_attacked % 8 < 7 && king_attacked / 8 < 7)
            if (is_legal(board, king_attacked + DOWNR, king_attacked))
                return 0;
    print_debug("downr\n");
    if (king_attacked + DOWN < 64 && king_attacked / 8 < 7)
        if (is_legal(board, king_attacked + DOWN, king_attacked))
            return 0;
    print_debug("down\n");
    for (i = 0; i < 64; i++ )
    {
        if ((board->position[i] & 0x80) != color || i != king_attacked)
        {
            uint8_t orig_piece = board->position[i];
            Found founds;
            find_attacker(board, i, (ALL_PIECES & (~KING)) | color, &founds);
            if (founds.num_found)
                return 0;
            board->position[i] = orig_piece;
        }
    }
    return 0x4;
}

/* Returns 1 if the current position is checkmate
 * Returns 0 otherwise
 */
int is_checkmate(Board* board, int which_color)
{
    int king_attacked = (which_color) ? board->bking_pos : board->wking_pos;
    print_debug("KING: %d\n", king_attacked);
    if (!is_attacked(board, king_attacked))
        return 0;
    print_debug("is attacked\n");
    uint8_t color = board->position[king_attacked] & 0x80;
    int i;
    if (king_attacked + UP >= 0 && king_attacked / 8 > 0)
        if (is_legal(board, king_attacked + UP, king_attacked))
            return 0;
    print_debug("can move up\n");
    if (king_attacked + UPR >= 0 && king_attacked + UPR < 64)
        if (king_attacked % 8 < 7 && king_attacked / 8 > 0)
            if (is_legal(board, king_attacked + UPR, king_attacked))
                return 0;
    print_debug("upr\n");
    if (king_attacked + UPL >= 0 && king_attacked + UPL < 64)
        if (king_attacked % 8 > 0 && king_attacked / 8 > 0)
            if (is_legal(board, king_attacked + UPL, king_attacked))
                return 0;
    print_debug("upl\n");
    if (king_attacked + LEFT >= 0 && king_attacked % 8 > 0)
        if (is_legal(board, king_attacked + LEFT, king_attacked))
            return 0;
    print_debug("left\n");
    if (king_attacked + RIGHT < 64 && king_attacked % 8 < 7)
        if (is_legal(board, king_attacked + RIGHT, king_attacked))
            return 0;
    print_debug("right\n");
    if (king_attacked + DOWNL >= 0 && king_attacked + DOWNL < 64)
        if (king_attacked % 8 > 0 && king_attacked / 8 < 7)
            if (is_legal(board, king_attacked + DOWNL, king_attacked))
                return 0;
    print_debug("downl\n");
    if (king_attacked + DOWNR >= 0 && king_attacked + DOWNR < 64)
        if (king_attacked % 8 < 7 && king_attacked / 8 < 7)
            if (is_legal(board, king_attacked + DOWNR, king_attacked))
                return 0;
    print_debug("downr\n");
    if (king_attacked + DOWN < 64 && king_attacked / 8 < 7)
        if (is_legal(board, king_attacked + DOWN, king_attacked))
            return 0;
    print_debug("down\n");
    print_debug("can't move\n");
    for (i = 0; i < 64; ++i)
    {
        if (i != king_attacked)
        {
            uint8_t orig_piece = board->position[i];
            board->position[i] = PAWN | color;
            if (!is_attacked(board, king_attacked) || i == board->en_p)
            {
                board->position[i] = orig_piece;
                if (i == board->en_p)
                {
                    Found en_found;
                    find_attacker(board, i,PAWN|color, &en_found);
                    if (!en_found.num_found)
                        continue;
                }
                Found founds;
                find_attacker(board, i, (ALL_PIECES & (~KING))|color, &founds);
                if (founds.num_found)
                {
                    print_debug("%c%d can be blocked by ", i%8+'a', 8-i/8);
                    int j;
                    for (j = 0; j < founds.num_found; ++j)
                        print_debug("%c%d ", founds.squares[j]%8+'a',
                                8-founds.squares[j]/8);
                    print_debug("\n");
                    return 0;
                }
            }
            board->position[i] = orig_piece;
        }
    }
    return 0x2;
}

/* Returns 2 if the current position is stalemate by three-fold repetition
 * Returns 0 otherwise
 */
int check_threefold(Board* board)
{
    int i;
    int count = 0;
    for (i = 0; i < board->pos_count - 1; ++i)
    {
        if (!strcmp(board->position_hist[board->pos_count - 1],
                    board->position_hist[i]))
            count++;
        if (count >= 2)
            return 0x10;
    }
    return 0;
}

/* Returns 1 if the current position is checkmate
 * Returns 2 if the current position is stalemate
 * Returns 0 otherwise
 */
int is_gameover(Board* board)
{
    if (board->history_count >= MAX_HISTORY)
    {
        snprintf(board->notes, 245, "MAXIMUM HISTORY (%u) REACHED\n", MAX_HISTORY);
        return 0x20;
    }
    if (board->pos_count >= MAX_STORED_POSITIONS)
    {
        snprintf(board->notes, 256, "MAXIMUM POSITIONS (%u) REACHED\n", MAX_STORED_POSITIONS);
        return 0x40;
    }
    int game_over = is_checkmate(board, board->to_move);
    print_debug("was checkmate? %d\n", game_over);
    if (!game_over)
        game_over = check_stalemate(board, board->to_move);
    print_debug("was stalemate? %d\n", game_over);
    if (!game_over && board->halfmoves >= 100)
        game_over = 0x8;
    print_debug("was 50-move? %d\n", game_over);
    if (!game_over && board->pos_count)
        game_over = check_threefold(board);

    if (board->history_count > 0)
        board->history[board->history_count - 1].game_over = game_over;
    return game_over;
}

void stress_test(Board* board, int times)
{
    int i;
    for (i = 0; i < times; ++i)
    {
        int j;
        for (j = 0; j < 64; ++j)
        {
            Found founds;
            find_attacker(board, j, ALL_PIECES, &founds);
        }
    }
}

/* Stores partial FEN for current position 
 * used in three-fold repetition check
 */
void store_position(Board* board, char* dest)
{
    int str_ind = 0;
    int i;
    for (i = 0; i < MAX_POSITION_STRING; ++i)
        dest[i] = 0;
    int empty = 0;
    for (i = 0; i < 64; ++i)
    {
        uint8_t square = board->position[i];
        if (i % 8 == 0 && i)
        {
            if (empty)
            {
                dest[str_ind++] = '0' + empty;
                empty = 0;
            }
            dest[str_ind++] = '/';
        }
        if (square)
        {
            if (empty)
            {
                dest[str_ind++] = '0' + empty;
                empty = 0;
            }
            if (square & KNIGHT)
                dest[str_ind] = 'N';
            else if (square & PAWN)
                dest[str_ind] = 'P';
            else if (square & BISHOP)
                dest[str_ind] = 'B';
            else if (square & ROOK)
                dest[str_ind] = 'R';
            else if (square & QUEEN)
                dest[str_ind] = 'Q';
            else if (square & KING)
                dest[str_ind] = 'K';
            if (square & BLACK)
                dest[str_ind] += 32;
            str_ind++;
        }
        else
            empty++;
    }

    if (empty)
        dest[str_ind++] = '0' + empty;

    dest[str_ind++] = ' ';
    /*
    if (board->to_move)
        dest[str_ind++] = 'b';
    else
        dest[str_ind++] = 'w';
        */
    //dest[str_ind++] = ' ';
    if (board->castling & 0x08)
        dest[str_ind++] = 'K';
    if (board->castling & 0x04)
        dest[str_ind++] = 'Q';
    if (board->castling & 0x02)
        dest[str_ind++] = 'k';
    if (board->castling & 0x01)
        dest[str_ind++] = 'q';
    if (!board->castling)
        dest[str_ind++] = '-';
    dest[str_ind++] = ' ';
    if (board->en_p != -1)
    {
        dest[str_ind++] = board->en_p % 8 + 'a';
        dest[str_ind++] = 8 - board->en_p / 8 + '0';
    }
    else
        dest[str_ind++] = '-';
    dest[str_ind] = '\0';
}

/* Makes a move on the board based on given Move struct and updates board state
 * Use this function when submitting an actual move on the board
 * Returns 0 for successful move, error otherwise
 */
int move_piece(Board* board, Move* move)
{
    /* Add current board state to history before making the move */
    memcpy(history + n_history, board, sizeof(Board));
    n_history++;

    /* Get list of valid moves */
    Found found;
    find_attacker(board, move->dest, move->src_piece, &found);
    move->castle = found.castle;
    print_debug("MOVE SRC_PIECE: %d\n", move->src_piece);
    print_debug("MOVE DEST: %c%d\n", move->dest%8+'a',8-move->dest/8);
    print_debug("NUM FOUND: %d\n", found.num_found);
    print_debug("Is in check?\n");
    if (board->to_move && is_attacked(board, board->bking_pos))
    {
        print_debug("    yes.\n");
        Found temp_fnd;
        board->to_move = !board->to_move;
        find_attacker(board, board->bking_pos, ALL_PIECES, &temp_fnd);
        board->to_move = !board->to_move;
        print_debug("attackers: %d\n", temp_fnd.num_found);
        int i;
        for (i = 0; i < temp_fnd.num_found; ++i)
        {
            print_debug("%c%d\n",
                    temp_fnd.squares[i]%8+'a',8-temp_fnd.squares[i]/8);
        }
        board->to_move = BLACK;
    }
    if (!board->to_move && is_attacked(board, board->wking_pos))
    {
        print_debug("    yes.\n");
        Found temp_fnd;
        board->to_move = !board->to_move;
        find_attacker(board, board->bking_pos, ALL_PIECES, &temp_fnd);
        board->to_move = !board->to_move;
        print_debug("attackers: %d\n", temp_fnd.num_found);
        int i;
        for (i = 0; i < temp_fnd.num_found; ++i)
        {
            print_debug("%c%d\n",
                    temp_fnd.squares[i]%8+'a',8-temp_fnd.squares[i]/8);
        }
        board->to_move = WHITE;
    }
        

    /* Determine which move from list to choose */
    int i;
    int move_to = -1;
    int file_match = 0;
    int rank_match = 0;
    if (found.num_found)
    {
        if (found.num_found > 1 && 
                (move->src_rank != -1 || move->src_file != -1))
        {
            move_to = -2;
            for (i = 0; i < found.num_found; ++i)
            {
                if (move->src_rank != -1)
                {
                    if (found.squares[i] / 8 == move->src_rank)
                        rank_match = 1;
                    else
                        rank_match = 0;
                }
                if (move->src_file != -1)
                {
                    if (found.squares[i] % 8 == move->src_file)
                        file_match = 1;
                    else
                        file_match = 0;
                }
                if (move->src_file != -1 && move->src_rank != -1)
                {
                    if (file_match && rank_match)
                    {
                        move_to = found.squares[i];
                        break;
                    }
                }
                else if (move->src_file != -1 && file_match)
                {
                    move_to = found.squares[i];
                    break;
                }
                else if (move->src_rank != -1 && rank_match)
                {
                    move_to = found.squares[i];
                    break;
                }
            }
        }
        else if (found.num_found == 1)
        {
            if (!(move->src_piece & PAWN))
                move->src_file = -1;
            move->src_rank = -1;
            if (move_to == -1)
                move_to = found.squares[0];
            else
                move_to = -2;
        }
        else
            move_to = -2;
    }

    /* Make move */
    if (move_to == -2)
    {
        int chars_printed = snprintf(board->notes, 256, "Ambigous move, more than one piece can move there.\n");
        chars_printed += snprintf(board->notes + chars_printed, 256, "%d. ", board->moves);
        if (board->to_move)
            chars_printed += snprintf(board->notes + chars_printed, 256, "... ");
        snprintf(board->notes + chars_printed, 256, "0x%X from %c%d to %c%d\n", move->src_piece, 
                move->src_file + 'a', 8 - move->src_rank,
                move->dest % 8 + 'a', 8 - move->dest / 8);
        return -1;
    }
    else if (move_to == -1)
    {
        snprintf(board->notes, 256, "Move not valid.\n");
        return -1;
    }
    else
    {
        Move* record = &(board->history[board->history_count]);
        if (move->castle != -1)
        {
            print_debug("CASTLING\n");
            int success = castle(board, move->castle);
            if (!success)
                snprintf(board->notes, 256, "Move not valid.\n");
            else
            {
                if (move->castle == 0)
                {
                    if (board->to_move)
                    {
                        move_square(board, 6, 4);
                        move_square(board, 5, 7);
                        board->castling &= 0xFC;
                    }
                    else
                    {
                        move_square(board, 62, 60);
                        move_square(board, 61, 63);
                        board->castling &= 0xF3;
                    }
                }
                else if (move->castle == 1)
                {
                    if (board->to_move)
                    {
                        move_square(board, 2, 4);
                        move_square(board, 3, 0);
                        board->castling &= 0xFC;
                    }
                    else
                    {
                        move_square(board, 58, 60);
                        move_square(board, 59, 56);
                        board->castling &= 0xF3;
                    }
                }
                record->castle = move->castle;
                record->src_piece = 0;
            }
        }
        else
        {
            record->dest = move->dest;
            record->src_piece = move->src_piece;
            //if (rank_match)
                record->src_rank = move_to / 8;
            //if (file_match)
                record->src_file = move_to % 8;
            record->piece_taken = board->position[move->dest];
            move_square(board, move->dest, move_to);
            if (move->src_piece == (KING | BLACK))
                board->bking_pos = move->dest;
            else if (move->src_piece == KING)
                board->wking_pos = move->dest;
            if (found.en_p_taken != -1)
            {
                record->piece_taken = board->position[found.en_p_taken];
                board->position[found.en_p_taken] = 0;
            }
            board->halfmoves++;
            if (found.made_en_p == -1)
                board->en_p = -1;
            else
                board->en_p = found.made_en_p;
            if (move->src_piece & PAWN)
            {
                board->pos_count = 0;
                board->halfmoves = 0;
                if (record->piece_taken)
                    record->src_file = move_to % 8;
            }
            if (record->piece_taken)
            {
                board->pos_count = 0;
                board->halfmoves = 0;
            }
            if (found.promotion)
            {
                board->position[move->dest] = move->promotion;
                record->promotion = move->promotion;
            }
        }
        store_position(board, board->position_hist[board->pos_count++]);
        board->history_count++;
        if (board->to_move)
            board->moves++;
        board->to_move = !board->to_move;
        uint8_t curr_king;
        if (board->to_move)
            curr_king = board->bking_pos;
        else
            curr_king = board->wking_pos;
        if (is_attacked(board, curr_king))
            record->gave_check = 1;
    }

    /* Update castling permissions */
    if (board->castling & 0x08)
        if (board->position[60] != ( KING|WHITE) ||
                board->position[63] != (ROOK|WHITE))
            board->castling &= 0xF7;
    if (board->castling & 0x04)
        if (board->position[60] != ( KING|WHITE) ||
                board->position[56] != (ROOK|WHITE))
            board->castling &= 0xFB;
    if (board->castling & 0x02)
        if (board->position[4] != ( KING|BLACK) ||
                board->position[7] != (ROOK|BLACK))
            board->castling &= 0xFD;
    if (board->castling & 0x01)
        if (board->position[4] != ( KING|BLACK) ||
                board->position[0] != (ROOK|BLACK))
            board->castling &= 0xFE;
    return 0;
}

void UndoMove(Board* board)
{
    if (n_history > 0)
    {
        n_history--;
        memcpy(board, history + n_history, sizeof(Board));
    }
}

/* Interprets Standard Algebraic Notation and makes a move */
int move_san(Board* board, char* move)
{
    Move this_move;
    this_move.src_piece = PAWN;
    this_move.promotion = QUEEN;
    this_move.src_rank = -1;
    this_move.src_file = -1;
    this_move.dest = -1;
    this_move.castle = -1;

    int destrank = -1;
    int destfile = -1;
    int ind = 0;
    char curr_char = move[ind];
    if (curr_char == 'B')
        this_move.src_piece = BISHOP;
    else if (curr_char == 'N')
        this_move.src_piece = KNIGHT;
    else if (curr_char == 'R')
        this_move.src_piece = ROOK;
    else if (curr_char == 'Q')
        this_move.src_piece = QUEEN;
    else if (curr_char == 'K')
        this_move.src_piece = KING;
    else
        destfile = curr_char - 'a';
    if (board->to_move == 1)
        this_move.src_piece |= BLACK;
    curr_char = move[++ind];
    while (curr_char)
    {
        if (!strcmp(move, "O-O"))
        {
            this_move.castle = 0;
            this_move.src_piece = KING;
            this_move.dest = (board->to_move) ? 6 : 62;
            break;
        }
        if (!strcmp(move, "O-O-O"))
        {
            this_move.castle = 1;
            this_move.src_piece = KING;
            this_move.dest = (board->to_move) ? 2 : 58;
            break;
        }
        if (curr_char == 'B')
            this_move.promotion = BISHOP;
        else if (curr_char == 'N')
            this_move.promotion = KNIGHT;
        else if (curr_char == 'R')
            this_move.promotion = ROOK;
        else if (curr_char == 'Q')
            this_move.promotion = QUEEN;
        if (curr_char <= 'h' && curr_char >= 'a')
        {
            if (destfile != -1)
                this_move.src_file = destfile;
            destfile = curr_char - 'a';
        }
        else if (curr_char <= '9' && curr_char >= '0')
        {
            if (destrank != -1)
                this_move.src_rank = destrank;
            destrank = '8' - curr_char;
        }
        curr_char = move[++ind];
    }
    if (board->to_move == 1)
        this_move.promotion |= BLACK;
    if (destrank != -1 && destfile != -1)
        this_move.dest = destrank * 8 + destfile;

    return move_piece(board, &this_move);
}
