#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "dynarray.h"
#include "settings.h"

#ifndef SETTINGS
#define LIGHT 249
#define DARK  239
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
    if (board->history)
    {
        for (i = 0; i < dynarray_size(board->history); ++i)
            free(dynarray_get(board->history, i));
        dynarray_free(board->history);
    }
}

void init_board(Board* board)
{
    board->history = NULL;
    empty_board(board);
}

void default_board(Board* board)
{
    int i;
    empty_board(board);
    board->bking_pos = 4;
    board->wking_pos = 60;
    board->history = dynarray_create();
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

void board_stats(Board* board)
{
    if (board->to_move == 1)
        printf("Black");
    else if (board->to_move == 0)
        printf("White");
    printf(" to move.\nCastling: ");
    if (board->castling & 0x08)
        printf("K");
    if (board->castling & 0x04)
        printf("Q");
    if (board->castling & 0x02)
        printf("k");
    if (board->castling & 0x01)
        printf("q");
    printf("\nEn pessant: ");
    if (board->en_p == -1)
        printf("None\n");
    else
        printf("%c%d\n", (board->en_p % 8) + 'a', 8 - (board->en_p / 8));
    printf("Halfmoves: %d\nFullmoves: %d\n", board->halfmoves, board->moves);
    printf("Whiteking_pos: %c%d\n", (board->wking_pos % 8) + 'a',
            8 - (board->wking_pos / 8));
    printf("Blackking_pos: %c%d\n", (board->bking_pos % 8) + 'a',
            8 - (board->bking_pos / 8));
}

void print_board(Board* board)
{
    int i;
    printf("  \u2554");
    for (i = 1; i < 32; ++i)
        printf("\u2550");
    printf("\u2557");
    for (i = 0; i < 64; ++i)
    {
        uint8_t square = board->position[i];
        if (i % 8 == 0)
            printf("\n%d \u2551", 8 - (i / 8));
        /* Bullshit that colors them checkered-like 
         * - Courtesy of Zach Gorman
         */
        if (!(!(i & 1) ^ !(i & 8))) 
            printf("\e[48;5;%dm", LIGHT);
        else
            printf("\e[48;5;%dm", DARK);
        printf(" ");
        if (square & pawn)
            (square & black) ? printf("\e[30mp") : printf("\e[37mP");
        else if (square & bishop)
            (square & black) ? printf("\e[30mb") : printf("\e[37mB");
        else if (square & knight)
            (square & black) ? printf("\e[30mn") : printf("\e[37mN");
        else if (square & rook)
            (square & black) ? printf("\e[30mr") : printf("\e[37mR");
        else if (square & queen)
            (square & black) ? printf("\e[30mq") : printf("\e[37mQ");
        else if (square & king)
            (square & black) ? printf("\e[30mk") : printf("\e[37mK");
        else
            printf(" ");
        printf(" \e[0m");

        if (i % 8 == 7)
            printf("\u2551");
        else
            printf("\u2502");
        if (i % 8 == 7 && i / 8 != 7)
        {
            printf("\n  \u2551\u2500\u2500\u2500\u253c\u2500\u2500\u2500\u253c"
                    "\u2500\u2500\u2500\u253c\u2500\u2500\u2500\u253c\u2500"
                    "\u2500\u2500\u253c\u2500\u2500\u2500\u253c\u2500\u2500"
                    "\u2500\u253c\u2500\u2500\u2500\u2551");
        }
    }
    printf("\n  ");
    printf("\u255a");
    for (i = 1; i < 32; ++i)
        printf("\u2550");
    printf("\u255d\n ");
    for (i = 0; i < 8; i++)
        printf("   %c", 'a' + i);
    printf("\n");
}

int string_to_int(char* str)
{
    int digits = strlen(str);
    int i;
    int num = 0;
    for (i = 0; i < digits; ++i)
    {
        int temp = str[i] - '0';
        int j;
        for (j = 0; j < digits - i - 1; ++i)
            temp *= 10;
        num += temp;
    }
    return num;
}

void load_fen(Board* board, char* fen)
{
    empty_board(board);
    char* fen_copy = malloc(strlen(fen) + 1);
    strcpy(fen_copy, fen);
    char* saveptr = NULL;
    char* token = NULL;
    token = strtok_r(fen_copy, " ", &saveptr);
    int square_ind = 0;
    int i = 0;
    char curr_char = token[i];
    while (curr_char)
    {
        if (curr_char == 'p')
            board->position[square_ind++] = pawn   | black;
        else if (curr_char == 'b')
            board->position[square_ind++] = bishop | black;
        else if (curr_char == 'n')
            board->position[square_ind++] = knight | black;
        else if (curr_char == 'r')
            board->position[square_ind++] = rook   | black;
        else if (curr_char == 'q')
            board->position[square_ind++] = queen  | black;
        else if (curr_char == 'k')
        {
            board->bking_pos = square_ind;
            board->position[square_ind++] = king   | black;
        }
        else if (curr_char == 'P')
            board->position[square_ind++] = pawn   | white;
        else if (curr_char == 'B')   
            board->position[square_ind++] = bishop | white;
        else if (curr_char == 'N')   
            board->position[square_ind++] = knight | white;
        else if (curr_char == 'R')   
            board->position[square_ind++] = rook   | white;
        else if (curr_char == 'Q')   
            board->position[square_ind++] = queen  | white;
        else if (curr_char == 'K')   
        {
            board->wking_pos = square_ind;
            board->position[square_ind++] = king   | white;
        }
        else if (curr_char <= '9' && curr_char >= '0')
            square_ind += curr_char - '0';
        i++;
        curr_char = token[i];
    }

    token = strtok_r(NULL, " ", &saveptr);
    if (token[0] == 'w')
        board->to_move = 0;
    else if (token[0] == 'b')
        board->to_move = 1;

    token = strtok_r(NULL, " ", &saveptr);
    i = 0;
    curr_char = token[i];
    while (curr_char)
    {
        if (curr_char == 'K')
            board->castling |= 0x08;
        else if (curr_char == 'Q')
            board->castling |= 0x04;
        else if (curr_char == 'k')
            board->castling |= 0x02;
        else if (curr_char == 'q')
            board->castling |= 0x01;
        else if (curr_char == '-')
            board->castling = 0x00;
        curr_char = token[++i];
    }

    token = strtok_r(NULL, " ", &saveptr);
    if (!strcmp(token, "-"))
        board->en_p = -1;
    else
        board->en_p = ((token[0] - 'a') + ('8' - token[1]) * 8);

    token = strtok_r(NULL, " ", &saveptr);
    board->halfmoves = string_to_int(token);

    token = strtok_r(NULL, " ", &saveptr);
    board->moves = string_to_int(token);

    free(fen_copy);
}

char* export_fen(Board* board)
{
    char* fen = malloc(100);
    int str_ind = 0;
    int i;
    for (i = 0; i < 100; ++i)
        fen[i] = 0;
    int empty = 0;
    for (i = 0; i < 64; ++i)
    {
        uint8_t square = board->position[i];
        if (i % 8 == 0 && i)
        {
            if (empty)
            {
                fen[str_ind++] = '0' + empty;
                empty = 0;
            }
            fen[str_ind++] = '/';
        }
        if (square)
        {
            if (empty)
            {
                fen[str_ind++] = '0' + empty;
                empty = 0;
            }
            if (square & knight)
                fen[str_ind] = 'N';
            else if (square & pawn)
                fen[str_ind] = 'P';
            else if (square & bishop)
                fen[str_ind] = 'B';
            else if (square & rook)
                fen[str_ind] = 'R';
            else if (square & queen)
                fen[str_ind] = 'Q';
            else if (square & king)
                fen[str_ind] = 'K';
            if (square & black)
                fen[str_ind] += 32;
            str_ind++;
        }
        else
            empty++;

    }
    if (empty)
    {
        fen[str_ind++] = '0' + empty;
        empty = 0;
    }
    fen[str_ind++] = ' ';
    if (board->to_move)
        fen[str_ind++] = 'b';
    else
        fen[str_ind++] = 'w';
    fen[str_ind++] = ' ';
    if (board->castling & 0x08)
        fen[str_ind++] = 'K';
    if (board->castling & 0x04)
        fen[str_ind++] = 'Q';
    if (board->castling & 0x02)
        fen[str_ind++] = 'k';
    if (board->castling & 0x01)
        fen[str_ind++] = 'q';
    if (!board->castling)
        fen[str_ind++] = '-';
    fen[str_ind++] = ' ';
    if (board->en_p != -1)
    {
        fen[str_ind++] = board->en_p % 8 + 'a';
        fen[str_ind++] = 8 - board->en_p / 8 + '0';
    }
    else
        fen[str_ind++] = '-';
    fen[str_ind++] = ' ';
    str_ind += sprintf(fen + str_ind, "%d", board->halfmoves);
    fen[str_ind++] = ' ';
    sprintf(fen + str_ind, "%d", board->moves);
    return fen;
}

void move_square(Board* board, int dest, int src)
{
    if (board->position[src] == (king | black))
        board->bking_pos = dest;
    if (board->position[src] == (king | white))
        board->wking_pos = dest;
    board->position[dest] = board->position[src];
    board->position[src] = 0;
    if (board->to_move)
        board->moves++;
    board->to_move = !board->to_move;
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
    t_board.to_move = !board->to_move;
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
    int made_en_p = 0;
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
    if (square / 8 == 4)
    {
        int i;
        uint8_t looking_at = board->position[square + 2 * DOWN];
        if (looking_at == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + 2 * DOWN;
            board->en_p = square + DOWN;
            made_en_p = 1;
        }
    }
    else if (square / 8 == 3)
    {
        int i;
        uint8_t looking_at = board->position[square + 2 * UP];
        if (looking_at == piece)
        {
            founds->num_found++;
            founds->squares[founds->num_found - 1] = square + 2 * UP;
            board->en_p = square + UP;
            made_en_p = 1;
        }
    }

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
    founds->num_found = 0;
    founds->en_p_taken = -1;
    founds->promotion = 0;
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
    else
        board->en_p = -1;
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

int is_checkmate(Board* board, int which_color)
{
    int king_attacked = (which_color) ? board->bking_pos : board->wking_pos;
    if (!is_attacked(board, king_attacked))
        return 0;
    uint8_t color = board->position[king_attacked] & 0x80;
    int i;
    if (is_legal(board, king_attacked + UP, king_attacked))
        return 0;
    if (is_legal(board, king_attacked + UPR, king_attacked))
        return 0;
    if (is_legal(board, king_attacked + UPL, king_attacked))
        return 0;
    if (is_legal(board, king_attacked + LEFT, king_attacked))
        return 0;
    if (is_legal(board, king_attacked + RIGHT, king_attacked))
        return 0;
    if (is_legal(board, king_attacked + DOWNL, king_attacked))
        return 0;
    if (is_legal(board, king_attacked + DOWNR, king_attacked))
        return 0;
    if (is_legal(board, king_attacked + DOWN, king_attacked))
        return 0;
    for (i = 0; i < 64; ++i)
    {
        if (i != king_attacked)
        {
            uint8_t orig_piece = board->position[i];
            board->position[i] = pawn | color;
            if (!is_attacked(board, king_attacked))
            {
                board->position[i] = orig_piece;
                struct found* founds = find_attacker(board, i,
                        (all_pieces & (~king)) | color);
                if (founds->num_found)
                {
                    free(founds);
                    board->to_move = !board->to_move;
                    return 0;
                }
                free(founds);
            }
            board->position[i] = orig_piece;
        }
    }
    return 1;
}

void move_san(Board* board, char* move)
{
    uint8_t sourcepiece = pawn;
    uint8_t promotionpiece = queen;
    int sourcerank = -1;
    int sourcefile = -1;
    int destrank = -1;
    int destfile = -1;
    int castled = -1;
    int ind = 0;
    char curr_char = move[ind];
    if (curr_char == 'B')
        sourcepiece = bishop;
    else if (curr_char == 'N')
        sourcepiece = knight;
    else if (curr_char == 'R')
        sourcepiece = rook;
    else if (curr_char == 'Q')
        sourcepiece = queen;
    else if (curr_char == 'K')
        sourcepiece = king;
    else
        destfile = curr_char - 'a';
    if (board->to_move == 1)
        sourcepiece |= black;
    curr_char = move[++ind];
    while (curr_char)
    {
        if (!strcmp(move, "O-O"))
            castled = 0;
        if (!strcmp(move, "O-O-O"))
            castled = 1;
        if (curr_char == 'B')
            promotionpiece = bishop;
        else if (curr_char == 'N')
            promotionpiece = knight;
        else if (curr_char == 'R')
            promotionpiece = rook;
        else if (curr_char == 'Q')
            promotionpiece = queen;
        if (curr_char <= 'h' && curr_char >= 'a')
        {
            if (destfile != -1)
                sourcefile = destfile;
            destfile = curr_char - 'a';
        }
        else if (curr_char <= '9' && curr_char >= '0')
        {
            if (destrank != -1)
                sourcerank = destrank;
            destrank = '8' - curr_char;
        }
        curr_char = move[++ind];
    }
    if (board->to_move == 1)
        promotionpiece |= black;

    /* Check if castling is allowed */
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

    /* Castle */
    if (castled != -1)
    {
        int success = castle(board, castled);
        if (!success)
            printf("Move not valid.\n");
        return;
    }

    /* Get list of valid moves */
    struct found* found;
    found = find_attacker(board, destrank * 8 + destfile, sourcepiece);

    /* Remove moves which put the king in check 
    struct found* found = malloc(sizeof(struct found));
    found->en_p_taken = found_first->en_p_taken;
    found->promotion = found_first->promotion;
    found->num_found = 0;
    int i;
    for (i = 0; i < found_first->num_found; ++i)
    {
        Board t_board;
        int j;
        for (j = 0; j < 64; ++j)
            t_board.position[j] = board->position[j];
        t_board.to_move = !board->to_move;
        t_board.en_p = board->en_p;
        t_board.bking_pos = board->bking_pos;
        t_board.wking_pos = board->wking_pos;
        move_square(&t_board, destrank * 8 + destfile, found_first->squares[i]);
        int square_check;
        if (board->to_move)
            square_check = t_board.bking_pos;
        else
            square_check = t_board.wking_pos;
        if (!is_attacked(&t_board, square_check))
        {
            found->num_found++;
            found->squares[found->num_found - 1] = found_first->squares[i];
        }
    }
    free(found_first);
    */
    /* Determine which move from list to choose */
    int i;
    int move_to = -1;
    if (found->num_found)
    {
        if (found->num_found > 1 && (sourcerank != -1 || sourcefile != -1))
        {
            for (i = 0; i < found->num_found; ++i)
            {
                if (sourcerank != -1 && found->squares[i] / 8 == sourcerank)
                {
                    if (move_to == -1)
                        move_to = found->squares[i];
                    else
                        move_to = -2;
                }
                if (sourcefile != -1 && found->squares[i] % 8 == sourcefile)
                {
                    if (move_to == -1)
                        move_to = found->squares[i];
                    else
                        move_to = -2;
                }
            }
        }
        else if (found->num_found == 1)
        {
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
        move_square(board, destrank * 8 + destfile, move_to);
        if (sourcepiece == (king | black))
            board->bking_pos = destrank * 8 + destfile;
        else if (sourcepiece == king)
            board->wking_pos = destrank * 8 + destfile;
        if (found->en_p_taken != -1)
        {
            board->position[found->en_p_taken] = 0;
            board->en_p = -1;
        }
        if (found->promotion)
            board->position[destrank * 8 + destfile] = promotionpiece;
    }
    free(found);
}
