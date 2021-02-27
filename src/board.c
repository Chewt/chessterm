#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"

#ifdef DEBUG
#define print_debug(...) printf(__VA_ARGS__)
#else
#define print_debug(...) ((void)0)
#endif

int is_attacked(Board* board, int square);

void empty_board(Board* board)
{
    board->to_move = 0;
    board->castling = 0x00;
    board->en_p = -1;
    board->halfmoves = 0;
    board->moves = 1;
    board->bking_pos = 0;
    board->wking_pos = 0;
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

void default_board(Board* board)
{
    int i;
    empty_board(board);
    board->bking_pos = 4;
    board->wking_pos = 60;
    board->castling = 0x0F;
    board->position[0] = rook   | black;
    board->position[1] = knight | black;
    board->position[2] = bishop | black;
    board->position[3] = queen  | black;
    board->position[4] = king   | black;
    board->position[5] = bishop | black;
    board->position[6] = knight | black;
    board->position[7] = rook   | black;
    for (i = 0; i < 8; ++i)
    {
        board->position[i + 8]     = pawn | black;
        board->position[i + 8 * 6] = pawn | white;
    }
    board->position[0 + 8 * 7] = rook   | white;
    board->position[1 + 8 * 7] = knight | white;
    board->position[2 + 8 * 7] = bishop | white;
    board->position[3 + 8 * 7] = queen  | white;
    board->position[4 + 8 * 7] = king   | white;
    board->position[5 + 8 * 7] = bishop | white;
    board->position[6 + 8 * 7] = knight | white;
    board->position[7 + 8 * 7] = rook   | white;
}


void move_square(Board* board, int dest, int src)
{
    print_debug("DEST: %d, SRC: %d\n", dest, src);
    if (board->position[src] == (king | black))
        board->bking_pos = dest;
    if (board->position[src] == (king | white))
        board->wking_pos = dest;
    board->position[dest] = board->position[src];
    board->position[src] = 0;
}

void move_verbose(Board* board, char* dest, char* src)
{
    uint8_t source_num = src[0] - 'a' + ('8' - src[1]) * 8;
    uint8_t dest_num = dest[0] - 'a' + ('8' - dest[1]) * 8;
    move_square(board, dest_num, source_num);
}

struct found 
{
    int num_found;
    int squares[16];
    int en_p_taken;
    int made_en_p;
    int promotion;
};

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

int is_legal(Board* board, int dest, int src)
{
    uint8_t color = board->position[src] & 0x80;
    if (dest < 0 || dest > 63)
        return 0;
    if (board->position[dest] && (board->position[dest] & 0x80) == color)
        return 0;
    Board t_board;
    int j;
    for (j = 0; j < 64; ++j)
        t_board.position[j] = board->position[j];
    t_board.to_move = board->to_move;
    t_board.en_p = board->en_p;
    t_board.bking_pos = board->bking_pos;
    t_board.wking_pos = board->wking_pos;
    move_square(&t_board, dest, src);
    if (dest == t_board.en_p)
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
    if (!is_attacked(&t_board, square_check))
        return 1;
    return 0;
}

void check_for_check(Board* board, int square,
        struct found* dest, struct found* src)
{
    dest->en_p_taken = src->en_p_taken;
    dest->promotion = src->promotion;
    dest->made_en_p = src->made_en_p;
    int i;
    for (i = 0; i < src->num_found; ++i)
    {
        if (is_legal(board, square, src->squares[i]))
        {
            dest->num_found++;
            dest->squares[dest->num_found - 1] = src->squares[i];
        }
    }
}

void check_knight(Board* board, int square, uint8_t piece, struct found* founds)
{
    founds->num_found = 0;
    founds->en_p_taken = -1;
    founds->promotion = 0;
    if (square + UP + UPL >= 0 && square + UP + UPL <= 63)
        if (board->position[square + UP + UPL] == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + UP + UPL;
        }
    if (square + LEFT + UPL >= 0 && square + LEFT + UPL <= 63)
        if (board->position[square + LEFT + UPL] == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + LEFT + UPL;
        }
    if (square + UP + UPR >= 0 && square + UP + UPR <= 63)
        if (board->position[square + UP + UPR] == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + UP + UPR;
        }
    if (square + RIGHT + UPR >= 0 && square + RIGHT + UPR <= 63)
        if (board->position[square + RIGHT + UPR] == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + RIGHT + UPR;
        }
    if (square + LEFT + DOWNL >= 0 && square + LEFT + DOWNL <= 63)
        if (board->position[square + LEFT + DOWNL] == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + LEFT + DOWNL;
        }
    if (square + DOWN + DOWNL >= 0 && square + DOWN + DOWNL <= 63)
        if (board->position[square + DOWN + DOWNL] == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + DOWN + DOWNL;
        }
    if (square + RIGHT + DOWNR >= 0 && square + RIGHT + DOWNR <= 63)
        if (board->position[square + RIGHT + DOWNR] == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + RIGHT + DOWNR;
        }
    if (square + DOWN + DOWNR >= 0 && square + DOWN + DOWNR <= 63)
        if (board->position[square + DOWN + DOWNR] == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + DOWN + DOWNR;
        }
}

void check_rook(Board* board, int square, uint8_t piece, struct found* founds)
{
    int i = square;
    while ((i + LEFT) % 8 < 7)
    {
        if (i + LEFT >= 0 && i + LEFT <= 63)
        {
            i += LEFT;
            if (board->position[i] != 0 && board->position[i] == piece)
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
    while ((i + RIGHT) % 8 > 0)
    {
        if (i + RIGHT >= 0 && i + RIGHT <= 63)
        {
            i += RIGHT;
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
    while ((i + UP) / 8 < 7)
    {
        if (i + UP >= 0 && i + UP <= 63)
        {
            i += UP;
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
    while ((i + DOWN) / 8 > 0)
    {
        if (i + DOWN >= 0 && i + DOWN <= 63)
        {
            i += DOWN;
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

void check_bishop(Board* board, int square, uint8_t piece, struct found* founds)
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

void check_pawn(Board* board, int square, uint8_t piece, struct found* founds)
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
        else if (square / 8 == 6)
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
        int i;
        uint8_t looking_at = board->position[square + 2 * DOWN];
        if (looking_at == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + 2 * DOWN;
            board->en_p = square + DOWN;
            founds->made_en_p = 1;
        }
    }
    else if (square / 8 == 3 && !board->position[square])
    {
        int i;
        uint8_t looking_at = board->position[square + 2 * UP];
        if (looking_at == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + 2 * UP;
            board->en_p = square + UP;
            founds->made_en_p = 1;
        }
    }
    else 
        founds->made_en_p = 0;

    if (square / 8 == 0 || square / 8 == 7)
        founds->promotion = 1;

    if (board->to_move)
    {
        if (board->position[square] && 
                ((board->position[square] & black) ^
                 (board->to_move << 7)))

        {
            if (board->position[square + UPR] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + UPR;
            }
            if (board->position[square + UPL] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + UPL;
            }
        }
        if(board->position[square + UP] == piece 
                && !board->position[square])
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + UP;
        }
    }
    else
    {
        if (board->position[square] && 
                ((board->position[square] & black) ^
                 (board->to_move << 7)))

        {
            if (board->position[square + DOWNL] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + DOWNL;
            }
            if (board->position[square + DOWNR] == piece)
            {
                founds->num_found++;
                founds->squares[founds->num_found - 1] = square + DOWNR;
            }
        }
        if(board->position[square + DOWN] == piece 
                && !board->position[square])
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + DOWN;
        }
    }
}

int can_move_to(Board* board, int square, uint8_t pieces)
{
    struct found* found_hyp = malloc(sizeof(struct found));
    found_hyp->num_found = 0;
    found_hyp->en_p_taken = -1;
    found_hyp->promotion = 0;
    uint8_t color = (board->to_move == 1) ? black : white;
    if (pieces & knight)
        check_knight(board, square, knight | color, found_hyp);
    if (found_hyp->num_found)
    {
        free(found_hyp);
        return 1;
    }
    if (pieces & bishop)
        check_bishop(board, square, bishop | color, found_hyp);
    if (found_hyp->num_found)
    {
        free(found_hyp);
        return 1;
    }
    if (pieces & rook)
        check_rook(board, square, rook | color, found_hyp);
    if (found_hyp->num_found)
    {
        free(found_hyp);
        return 1;
    }
    if (pieces & pawn)
        check_pawn(board, square, pawn | color, found_hyp);
    if (found_hyp->num_found)
    {
        free(found_hyp);
        return 1;
    }
    if (pieces & queen)
        check_rook(board, square, queen | color, found_hyp);
    if (found_hyp->num_found)
    {
        free(found_hyp);
        return 1;
    }
    if (pieces & queen)
        check_bishop(board, square, queen | color, found_hyp);
    if (found_hyp->num_found)
    {
        free(found_hyp);
        return 1;
    }
    free(found_hyp);
    return 0;
}

int is_attacked(Board* board, int square)
{
    board->to_move = !board->to_move;
    uint8_t color = (board->to_move == 1) ? black : white;
    uint8_t prev_piece = board->position[square];
    board->position[square] = pawn | (color ^ 0x80);
    int check = can_move_to(board, square, all_pieces);
    board->position[square] = prev_piece;
    board->to_move = !board->to_move;
    return check;
}

void check_king(Board* board, int square, uint8_t piece, struct found* founds)
{
    if (is_attacked(board, square))
        return;
    if (board->position[square + LEFT] == piece)
    {
        founds->num_found++;
        founds->squares[founds->num_found - 1] = square + LEFT;
    }
    if (board->position[square + RIGHT] == piece)
    {
        founds->num_found++;
        founds->squares[founds->num_found - 1] = square + RIGHT;
    }
    if (board->position[square + UPL] == piece)
    {
        founds->num_found++;
        founds->squares[founds->num_found - 1] = square + UPL;
    }
    if (board->position[square + DOWNR] == piece)
    {
        founds->num_found++;
        founds->squares[founds->num_found - 1] = square + DOWNR;
    }
    if (board->position[square + UPR] == piece)
    {
        founds->num_found++;
        founds->squares[founds->num_found - 1] = square + UPR;
    }
    if (board->position[square + DOWNL] == piece)
    {
        founds->num_found++;
        founds->squares[founds->num_found - 1] = square + DOWNL;
    }
    if (board->position[square + UP] == piece)
    {
        founds->num_found++;
        founds->squares[founds->num_found - 1] = square + UP;
    }
    if (board->position[square + DOWN] == piece)
    {
        founds->num_found++;
        founds->squares[founds->num_found - 1] = square + DOWN;
    }
}

struct found* find_attacker(Board* board, int square, uint8_t piece)
{
    struct found* founds = malloc(sizeof(struct found));
    struct found src;
    src.num_found = 0;
    src.en_p_taken = -1;
    src.promotion = 0;
    src.made_en_p = 0;
    founds->num_found = 0;
    founds->en_p_taken = -1;
    founds->promotion = 0;
    founds->made_en_p = 0;
    uint8_t color = 0x80 & piece;
    if (board->position[square] && !((board->position[square] & black) ^
                (board->to_move << 7)))
        return founds;

    if (piece & knight)
        check_knight(board, square, knight|color, &src);
    if (piece & rook)
        check_rook(board, square, rook|color, &src);
    if (piece & bishop)
        check_bishop(board, square, bishop|color, &src);
    if (piece & king)
        check_king(board, square, king|color, &src);
    if (piece & queen)
        check_rook(board, square, queen|color, &src);
    if (piece & queen)
        check_bishop(board, square, queen|color, &src);
    if (piece & pawn)
        check_pawn(board, square, pawn|color, &src);
    check_for_check(board, square, founds, &src);
    return founds;
}

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
                {
                    move_square(board, 62, 60);
                    board->position[61] = board->position[63];
                    board->position[63] = 0;
                    board->wking_pos = 62;
                    board->castling &= 0xF3;
                    return 1;
                }
        }
        else if(board->to_move && board->castling & 0x02)
        {
            if (!(board->position[5] | board->position[6]))
                if(   !is_attacked(board, 4) 
                   && !is_attacked(board, 5) 
                   && !is_attacked(board, 6))
                {
                    move_square(board, 6, 4);
                    board->position[5] = board->position[7];
                    board->position[7] = 0;
                    board->bking_pos = 6;
                    board->castling &= 0xFC;
                    return 1;
                }
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
                {
                    move_square(board, 58, 60);
                    board->position[59] = board->position[56];
                    board->position[56] = 0;
                    board->wking_pos = 58;
                    board->castling &= 0xF3;
                    return 1;
                }
        }
        else if(board->to_move && board->castling & 0x01)
        {
            if (!(board->position[1]|board->position[2]|board->position[3]))
                if(   !is_attacked(board, 4) 
                   && !is_attacked(board, 3) 
                   && !is_attacked(board, 2))
                {
                    move_square(board, 2, 4);
                    board->position[3] = board->position[0];
                    board->position[0] = 0;
                    board->bking_pos = 2;
                    board->castling &= 0xFC;
                    return 1;
                }
        }
    }
    return 0;
}

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
        if (piece & pawn)
            (piece & 0x80) ? bp++ : p++;
        else if (piece & bishop)
            (piece & 0x80) ? bb++ : b++;
        else if (piece & knight)
            (piece & 0x80) ? bn++ : n++;
        else if (piece & rook)
            (piece & 0x80) ? br++ : r++;
        else if (piece & queen)
            (piece & 0x80) ? bq++ : q++;
        else if (piece & king)
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
            return 2;
    }

    if (is_legal(board, king_attacked + UP, king_attacked))
        return 0;
    print_debug("can move up\n");
    if (is_legal(board, king_attacked + UPR, king_attacked))
        return 0;
    print_debug("upr\n");
    if (is_legal(board, king_attacked + UPL, king_attacked))
        return 0;
    print_debug("upl\n");
    if (is_legal(board, king_attacked + LEFT, king_attacked))
        return 0;
    print_debug("left\n");
    if (is_legal(board, king_attacked + RIGHT, king_attacked))
        return 0;
    print_debug("right\n");
    if (is_legal(board, king_attacked + DOWNL, king_attacked))
        return 0;
    print_debug("downl\n");
    if (is_legal(board, king_attacked + DOWNR, king_attacked))
        return 0;
    print_debug("downr\n");
    if (is_legal(board, king_attacked + DOWN, king_attacked))
        return 0;
    print_debug("down\n");
    for (i = 0; i < 64; i++ )
    {
        if ((board->position[i] & 0x80) != color || i != king_attacked)
        {
            uint8_t orig_piece = board->position[i];
            struct found* founds = find_attacker(board, i,
                    (all_pieces & (~king)) | color);
            if (founds->num_found)
            {
                free(founds);
                return 0;
            }
            free(founds);
            board->position[i] = orig_piece;
        }
    }
    return 2;
}

int check_checkmate(Board* board, int which_color)
{
    int king_attacked = (which_color) ? board->bking_pos : board->wking_pos;
    print_debug("KING: %d\n", king_attacked);
    if (!is_attacked(board, king_attacked))
        return 0;
    print_debug("is attacked\n");
    uint8_t color = board->position[king_attacked] & 0x80;
    int i;
    if (is_legal(board, king_attacked + UP, king_attacked))
        return 0;
    print_debug("can move up\n");
    if (is_legal(board, king_attacked + UPR, king_attacked))
        return 0;
    print_debug("upr\n");
    if (is_legal(board, king_attacked + UPL, king_attacked))
        return 0;
    print_debug("upl\n");
    if (is_legal(board, king_attacked + LEFT, king_attacked))
        return 0;
    print_debug("left\n");
    if (is_legal(board, king_attacked + RIGHT, king_attacked))
        return 0;
    print_debug("right\n");
    if (is_legal(board, king_attacked + DOWNL, king_attacked))
        return 0;
    print_debug("downl\n");
    if (is_legal(board, king_attacked + DOWNR, king_attacked))
        return 0;
    print_debug("downr\n");
    if (is_legal(board, king_attacked + DOWN, king_attacked))
        return 0;
    print_debug("down\n");
    print_debug("can't move\n");
    for (i = 0; i < 64; ++i)
    {
        if (i != king_attacked)
        {
            uint8_t orig_piece = board->position[i];
            board->position[i] = pawn | color;
            if (!is_attacked(board, king_attacked) || i == board->en_p)
            {
                board->position[i] = orig_piece;
                struct found* founds = find_attacker(board, i,
                        (all_pieces & (~king)) | color);
                if (founds->num_found)
                {
                    print_debug("%d can be blocked\n", i);
                    int j;
                    for (j = 0; j < founds->num_found; ++j)
                        print_debug("%d ", founds->squares[j]);
                    print_debug("\n");
                    free(founds);
                    return 0;
                }
                free(founds);
            }
            board->position[i] = orig_piece;
        }
    }
    return 1;
}

int is_gameover(Board* board)
{

    print_debug("WKING_POS BEG %d\n", board->wking_pos);
    int game_over = check_checkmate(board, board->to_move);
    print_debug("WKING_POS cm %d\n", board->wking_pos);
    print_debug("was checkmate? %d\n", game_over);
    if (!game_over)
        game_over = check_stalemate(board, board->to_move);
    print_debug("WKING_POS sm %d\n", board->wking_pos);
    print_debug("was stalemate? %d\n", game_over);
    if (!game_over && board->halfmoves >= 100)
        game_over = 2;
    print_debug("was 50-move? %d\n", game_over);
    
    if (board->history_count > 0)
        board->history[board->history_count - 1].game_over = game_over;
    print_debug("WKING_POS END %d\n", board->wking_pos);
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
            struct found* founds = find_attacker(board, j, all_pieces);
            free(founds);
        }
    }
}

void move_piece(Board* board, Move* move)
{
    /* Castle */
    if (move->castle != -1)
    {
        print_debug("CASTLING\n");
        int success = castle(board, move->castle);
        if (!success)
            printf("Move not valid.\n");
        else
        {
            board->history[board->history_count].castle = move->castle;
            board->history[board->history_count].src_piece = 0;
            uint8_t curr_king;
            if (board->to_move)
                curr_king = board->bking_pos;
            else
                curr_king = board->wking_pos;
            if (is_attacked(board, curr_king))
                board->history[board->history_count].gave_check = 1;
            board->history_count++;
        }
        return;
    }

    /* Get list of valid moves */
    struct found* found;
    found = find_attacker(board, move->dest, move->src_piece);
    print_debug("NUM FOUND: %d\n", found->num_found);

    /* Determine which move from list to choose */
    int i;
    int move_to = -1;
    int file_match = 0;
    int rank_match = 0;
    if (found->num_found)
    {
        if (found->num_found > 1 && 
                (move->src_rank != -1 || move->src_file != -1))
        {
            for (i = 0; i < found->num_found; ++i)
            {
                if (move->src_rank != -1 &&
                        found->squares[i] / 8 == move->src_rank)
                {
                    rank_match = 1;
                    if (move_to == -1)
                        move_to = found->squares[i];
                    else
                        move_to = -2;
                }
                else
                    rank_match = 0;
                if (move->src_file != -1 &&
                        found->squares[i] % 8 == move->src_file)
                {
                    file_match = 1;
                    if (move_to == -1)
                        move_to = found->squares[i];
                    else
                        move_to = -2;
                }
                else
                    file_match = 0;
            }
        }
        else if (found->num_found == 1)
        {
            if (!(move->src_piece & pawn))
                move->src_file = -1;
            move->src_rank = -1;
            if (move_to == -1)
                move_to = found->squares[0];
            else
                move_to = -2;
        }
        else
            move_to = -2;
    }

    /* Make move */
    if (move_to == -2)
        printf("Ambigous move, more than one piece can move there.\n");
    else if (move_to == -1)
        printf("Move not valid.\n");
    else
    {
        print_debug("HERE\n");
        Move* record = &(board->history[board->history_count]);
        record->dest = move->dest;
        record->src_piece = move->src_piece;
        if (rank_match)
            record->src_rank = move->src_rank;
        if (file_match)
            record->src_file = move->src_file;
        record->piece_taken = board->position[move->dest];
        move_square(board, move->dest, move_to);
        board->to_move = !board->to_move;
        uint8_t curr_king;
        if (board->to_move)
            curr_king = board->bking_pos;
        else
            curr_king = board->wking_pos;
        if (is_attacked(board, curr_king))
            record->gave_check = 1;
        if (move->src_piece == (king | black))
            board->bking_pos = move->dest;
        else if (move->src_piece == king)
            board->wking_pos = move->dest;
        if (found->en_p_taken != -1)
        {
            record->piece_taken = board->position[found->en_p_taken];
            board->position[found->en_p_taken] = 0;
        }
        board->halfmoves++;
        if (!found->made_en_p)
            board->en_p = -1;
        if (move->src_piece & pawn)
        {
            board->halfmoves = 0;
            if (record->piece_taken)
                record->src_file = move_to % 8;
        }
        if (record->piece_taken)
            board->halfmoves = 0;
        if (found->promotion)
        {
            board->position[move->dest] = move->promotion;
            record->promotion = move->promotion;
        }
        board->history_count++;
        if (board->to_move)
            board->moves++;
    }
    free(found);

    /* Update castling permissions */
    if (board->castling & 0x08)
        if (board->position[60] != (king|white) ||
            board->position[63] != (rook|white))
            board->castling &= 0xF7;
    if (board->castling & 0x04)
        if (board->position[60] != (king|white) ||
            board->position[56] != (rook|white))
            board->castling &= 0xFB;
    if (board->castling & 0x02)
        if (board->position[4] != (king|black) ||
            board->position[7] != (rook|black))
            board->castling &= 0xFD;
    if (board->castling & 0x01)
        if (board->position[4] != (king|black) ||
            board->position[0] != (rook|black))
            board->castling &= 0xFE;
}

void move_san(Board* board, char* move)
{
    Move this_move;
    this_move.src_piece = pawn;
    this_move.promotion = queen;
    this_move.src_rank = -1;
    this_move.src_file = -1;
    this_move.dest = -1;
    this_move.castle = -1;

    int destrank = -1;
    int destfile = -1;
    int ind = 0;
    char curr_char = move[ind];
    if (curr_char == 'B')
         this_move.src_piece = bishop;
    else if (curr_char == 'N')
        this_move.src_piece = knight;
    else if (curr_char == 'R')
        this_move.src_piece = rook;
    else if (curr_char == 'Q')
        this_move.src_piece = queen;
    else if (curr_char == 'K')
        this_move.src_piece = king;
    else
        destfile = curr_char - 'a';
    if (board->to_move == 1)
        this_move.src_piece |= black;
    curr_char = move[++ind];
    while (curr_char)
    {
        if (!strcmp(move, "O-O"))
            this_move.castle = 0;
        if (!strcmp(move, "O-O-O"))
            this_move.castle = 1;
        if (curr_char == 'B')
            this_move.promotion = bishop;
        else if (curr_char == 'N')
            this_move.promotion = knight;
        else if (curr_char == 'R')
            this_move.promotion = rook;
        else if (curr_char == 'Q')
            this_move.promotion = queen;
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
        this_move.promotion |= black;
    if (destrank != -1 && destfile != -1)
        this_move.dest = destrank * 8 + destfile;

    move_piece(board, &this_move);
}
